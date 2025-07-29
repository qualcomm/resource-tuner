// Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
// SPDX-License-Identifier: BSD-3-Clause-Clear

#include <csignal>
#include <fcntl.h>
#include <sys/wait.h>
#include <getopt.h>

#include "ComponentRegistry.h"
#include "SyslockInternal.h"
#include "ClientGarbageCollector.h"
#include "PulseMonitor.h"
#include "Extensions.h"
#include "RequestReceiver.h"

static int8_t terminateServer = false;

static void handleSIGINT(int32_t sig) {
    terminateServer = true;
}

static void handleSIGTSTP(int32_t sig) {
    toggleDisplayModes();
}

static void writeSysFsDefaults() {
    // Write Defaults
    std::ifstream file;

    file.open("../sysfsOriginalValues.txt");
    if(!file.is_open()) {
        LOGE("URM_SYSTUNE_MAIN", "Failed to open sysfs original values file: sysfsOriginalValues.txt");
        return;
    }

    std::string line;
    while(std::getline(file, line)) {
        std::stringstream lineStream(line);
        std::string token;

        int8_t index = 0;
        std::string sysfsNodePath = "";
        int32_t sysfsNodeDefaultValue = -1;

        while(std::getline(lineStream, token, ',')) {
            if(index == 0) {
                sysfsNodePath = token;
            } else if(index == 1) {
                sysfsNodeDefaultValue = std::stoi(token);
            }
            index++;
        }

        if(sysfsNodePath.length() > 0 && sysfsNodeDefaultValue != -1) {
            // Write To Node
            std::ofstream sysfsFile(sysfsNodePath, std::ios::out | std::ios::trunc);

            if(!sysfsFile.is_open()) {
                LOGE("URM_SYSTUNE_MAIN",
                     "Failed to write default value to sysfs node: " + sysfsNodePath);
                continue;
            }

            sysfsFile << std::to_string(sysfsNodeDefaultValue);
            if(sysfsFile.fail()) {
                LOGE("URM_SYSTUNE_MAIN",
                     "Failed to write default value to sysfs node: " + sysfsNodePath);
                sysfsFile.flush();
                sysfsFile.close();
                continue;
            }

            sysfsFile.flush();
            sysfsFile.close();
        }
    }
}

static void serverCleanup() {
    LOGE("URM_SYSTUNE_MAIN", "Server Stopped, Cleanup Initiated");
    SystuneSettings::setServerOnlineStatus(false);
}

/**
 * @brief The main function representing the initialisation of the server.
 * @details The parsing occurs in this stage.
 */
int32_t main(int32_t argc, char *argv[]) {
    std::signal(SIGINT, handleSIGINT);
    std::signal(SIGTSTP, handleSIGTSTP);

    if(argc != 2) {
        std::cout << "Usage: " << argv[0] << " <start|stop>" << std::endl;
        return -1;
    }

    const char* shortPrompts = "sth";
    const struct option longPrompts[] = {
        {"start", no_argument, nullptr, 's'},
        {"test", no_argument, nullptr, 't'},
        {"help", no_argument, nullptr, 'h'},
        {nullptr, no_argument, nullptr, 0}
    };

    int32_t c;
    while ((c = getopt_long(argc, argv, shortPrompts, longPrompts, nullptr)) != -1) {
        switch (c) {
            case 's':
                SystuneSettings::serverInTestMode = false;
                break;
            case 't':
                SystuneSettings::serverInTestMode = true;
                URM_REGISTER_CONFIG(RESOURCE_CONFIG, "/etc/systune/testResourceConfigs.json")
                URM_REGISTER_CONFIG(SIGNALS_CONFIG, "/etc/systune/testSignalConfigs.json")
                break;
            case 'h':
                std::cout<<"Help Options"<<std::endl;
                return 0;
            default:
                std::cout<<"Invalid CLI Option specified"<<std::endl;
                return 0;
        }
    }

    // Create a Child Process to Monitor the Parent (Server) Process
    // This is done to ensure that all the Resource sysfs Nodes are in a consistent state
    // If the Server Crashes or Terminates Abnormally.
    int32_t childProcessID = fork();
    if(childProcessID < 0) {
        TYPELOGV(ERRNO_LOG, "fork", strerror(errno));
        LOGE("URM_SYSTUNE_MAIN", "Failed to create Daemon, Terminating");
        return -1;

    } else if(childProcessID == 0) {
        while(true) {
            // Every Second, Check the Parent-PID for the Child Process
            // If at any point this value becomes 1, which means that the Parent
            // has terminated and the Child Process has been adopted by the init
            // Process, which has a PID of 1.
            // In such a scenario, the Child Process should proceed with restoring
            // all the Sysfs Nodes to a Sane State.
            std::this_thread::sleep_for(std::chrono::seconds(1));
            if(getppid() == 1) {
                writeSysFsDefaults();
                break;
            }
        }

        // Delete the Sysfs Persistent File
        remove("../sysfsOriginalValues.txt");
        exit(EXIT_SUCCESS);
    }

    ErrCode mOpStatus = RC_SUCCESS;
    SystuneSettings::setServerOnlineStatus(true);
    SystuneSettings::targetConfigs.currMode = MODE_DISPLAY_ON;
    RequestReceiver::mRequestsThreadPool = new ThreadPool(4, 4);
    Timer::mTimerThreadPool = new ThreadPool(4, 4);

    mOpStatus = fetchProperties();
    if(RC_IS_NOTOK(mOpStatus)) {
        TYPELOGD(PROPERTY_RETRIEVAL_FAILED);
    }

    // Check which modules are plugged In and Initialize them
    if(RC_IS_OK(mOpStatus)) {
        mOpStatus = ComponentRegistry::getModuleRegistrationCallback(MOD_PROVISIONER)();
        if(RC_IS_NOTOK(mOpStatus)) {
            TYPELOGV(MODULE_INIT_FAILED, PROVISIONER);
        }
    }

    if(ComponentRegistry::isModuleEnabled(MOD_SYSSIGNAL)) {
        TYPELOGV(NOTIFY_MODULE_ENABLED, SYSSIGNAL);
        if(RC_IS_OK(mOpStatus)) {
            mOpStatus = ComponentRegistry::getModuleRegistrationCallback(MOD_SYSSIGNAL)();
            if(RC_IS_NOTOK(mOpStatus)) {
                TYPELOGV(MODULE_INIT_FAILED, SYSSIGNAL);
            }
        }
    }

    if(!SystuneSettings::serverInTestMode) {
        // Start the Pulse Monitor and Garbage Collector Daemon Threads
        if(RC_IS_OK(mOpStatus)) {
            mOpStatus = startPulseMonitorDaemon();

            if(RC_IS_NOTOK(mOpStatus)) {
                TYPELOGD(PULSE_MONITOR_INIT_FAILED);
            }
        }

        if(RC_IS_OK(mOpStatus)) {
            mOpStatus = startClientGarbageCollectorDaemon();

            if(RC_IS_NOTOK(mOpStatus)) {
                TYPELOGD(GARBAGE_COLLECTOR_INIT_FAILED);
            }
        }
    }

    // Create a Listener Thread
    std::thread systuneListener;
    if(RC_IS_OK(mOpStatus)) {
        LOGI("URM_SYSTUNE_MAIN",
             "Starting Systune Listener Thread");
        systuneListener = std::thread(listenerThreadStartRoutine);
    }

    if(RC_IS_OK(mOpStatus)) {
        // Make Stdin Non-Blocking
        int32_t flags = fcntl(STDIN_FILENO, F_GETFL, 0);
        if(flags == -1) {
            TYPELOGV(ERRNO_LOG, "fcntl", strerror(errno));
        } else {
            fcntl(STDIN_FILENO, F_SETFL, flags | O_NONBLOCK);
        }

        // Listen for Terminal prompts
        while(!terminateServer) {
            char exitStatus[16];
            exitStatus[sizeof(exitStatus) - 1] = '\0';
            ssize_t bytesRead = read(STDIN_FILENO, exitStatus, sizeof(exitStatus) - 1);
            if(bytesRead > 0) {
                if(strncmp(exitStatus, "stop", 4) == 0 || strncmp(exitStatus, "exit", 4) == 0) {
                    terminateServer = true;
                }
            }
        }
    }

    // Cleanup the Server, so that it boots up correctly the next time
    // - This includes Closing the Server Communication Endpoints
    // - Resetting all the Sysfs nodes to their original values
    // - Terminating the different listener and Processor threads created to handle Requests.
    // - Killing the child process created to monitor the parent (Server)
    serverCleanup();

    ComponentRegistry::getModuleTeardownCallback(MOD_PROVISIONER)();

    if(ComponentRegistry::isModuleEnabled(MOD_SYSSIGNAL)) {
        ComponentRegistry::getModuleTeardownCallback(MOD_SYSSIGNAL)();
    }

    if(systuneListener.joinable()) {
        systuneListener.join();
    }

    // Restore all the Resources to Original Values
    ResourceRegistry::getInstance()->restoreResourcesToDefaultValues();

    kill(childProcessID, SIGKILL);
    // Wait for the Child Process to terminate
    waitpid(childProcessID, nullptr, 0);

    // Delete the Sysfs Persistent File
    remove("../sysfsOriginalValues.txt");

    return 0;
}
