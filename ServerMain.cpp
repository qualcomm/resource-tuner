// Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
// SPDX-License-Identifier: BSD-3-Clause-Clear

#include <csignal>
#include <fcntl.h>
#include <sys/wait.h>
#include <getopt.h>

#include "ComponentRegistry.h"
#include "ServerInternal.h"
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

static void serverCleanup() {
    LOGE("RTN_SERVER_INIT", "Server Stopped, Cleanup Initiated");
    ResourceTunerSettings::setServerOnlineStatus(false);
}

static ErrCode createResourceTunerDaemon(int32_t& childProcessID) {
    // Create a Child Process to Monitor the Parent (Server) Process
    // This is done to ensure that all the Resource sysfs Nodes are in a consistent state
    // If the Server Crashes or Terminates Abnormally.
    childProcessID = fork();
    if(childProcessID < 0) {
        TYPELOGV(ERRNO_LOG, "fork", strerror(errno));
        return RC_MODULE_INIT_FAILURE;

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
                AuxRoutines::writeSysFsDefaults();
                break;
            }
        }

        // Delete the Sysfs Persistent File
        AuxRoutines::deleteFile("sysfsOriginalValues.txt");
        exit(EXIT_SUCCESS);
    }

    return RC_SUCCESS;
}

int32_t main(int32_t argc, char *argv[]) {
    // PID of the Child Daemon
    int32_t childProcessID = -1;

    if(argc != 2) {
        std::cout<<"Usage: "<<argv[0]<<" <start|stop|test>"<<std::endl;
        return -1;
    }

    std::signal(SIGINT, handleSIGINT);
    std::signal(SIGTSTP, handleSIGTSTP);

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
                ResourceTunerSettings::serverInTestMode = false;
                break;
            case 't':
                ResourceTunerSettings::serverInTestMode = true;
                RESTUNE_REGISTER_CONFIG(PROPERTIES_CONFIG, "../Tests/Configs/testPropertiesConfig.yaml")
                RESTUNE_REGISTER_CONFIG(RESOURCE_CONFIG, "../Tests/Configs/testResourcesConfig.yaml")
                RESTUNE_REGISTER_CONFIG(SIGNALS_CONFIG, "../Tests/Configs/testSignalsConfig.yaml")
                RESTUNE_REGISTER_CONFIG(TARGET_CONFIG, "../Tests/Configs/testTargetConfig.yaml")
                break;
            case 'h':
                std::cout<<"Help Options"<<std::endl;
                return 0;
            default:
                std::cout<<"Invalid CLI Option specified"<<std::endl;
                return 0;
        }
    }

    TYPELOGV(NOTIFY_RESOURCE_TUNER_INIT_START, getpid());

    // Start Resource Tuner Server Initialization
    // As part of Server Initialization the Configs (Resource / Signals etc.) will be parsed
    // If any of mandatory Configs cannot be parsed then initialization will fail.
    // Mandatory Configs include: Properties Configs, Resource Configs and Signal Configs (if Signal
    // module is plugged in)
    ErrCode mOpStatus = RC_SUCCESS;
    if(RC_IS_OK(mOpStatus)) {
        mOpStatus = createResourceTunerDaemon(childProcessID);
        if(RC_IS_NOTOK(mOpStatus)) {
            TYPELOGD(RESOURCE_TUNER_DAEMON_CREATION_FAILURE);
        }
    }

    if(RC_IS_OK(mOpStatus)) {
        ResourceTunerSettings::setServerOnlineStatus(true);
        ResourceTunerSettings::targetConfigs.currMode = MODE_DISPLAY_ON;

        try {
            int32_t desiredThreadCapacity = ResourceTunerSettings::desiredThreadCount;
            int32_t maxPendingQueueSize = ResourceTunerSettings::maxPendingQueueSize;
            int32_t maxScalingCapacity = ResourceTunerSettings::maxScalingCapacity;

            RequestReceiver::mRequestsThreadPool = new ThreadPool(desiredThreadCapacity,
                                                                  maxPendingQueueSize,
                                                                  maxScalingCapacity);

            Timer::mTimerThreadPool = new ThreadPool(desiredThreadCapacity,
                                                     maxPendingQueueSize,
                                                     maxScalingCapacity);

        } catch(const std::bad_alloc& e) {
            TYPELOGV(THREAD_POOL_CREATION_FAILURE, e.what());
            mOpStatus = RC_MODULE_INIT_FAILURE;
        }
    }

    if(RC_IS_OK(mOpStatus)) {
        mOpStatus = fetchProperties();
        if(RC_IS_NOTOK(mOpStatus)) {
            TYPELOGD(PROPERTY_RETRIEVAL_FAILED);
        }
    }

    // Check which modules are plugged In and Initialize them
    if(RC_IS_OK(mOpStatus)) {
        mOpStatus = ComponentRegistry::getModuleRegistrationCallback(MOD_CORE)();
        if(RC_IS_NOTOK(mOpStatus)) {
            TYPELOGV(MODULE_INIT_FAILED, "Core");
        }
    }

    if(RC_IS_OK(mOpStatus)) {
        if(ComponentRegistry::isModuleEnabled(MOD_SYSSIGNAL)) {
            TYPELOGV(NOTIFY_MODULE_ENABLED, "Signal");
            if(RC_IS_OK(mOpStatus)) {
                mOpStatus = ComponentRegistry::getModuleRegistrationCallback(MOD_SYSSIGNAL)();
                if(RC_IS_NOTOK(mOpStatus)) {
                    TYPELOGV(MODULE_INIT_FAILED, "Signal");
                }
            }
        }
    }

    if(!ResourceTunerSettings::serverInTestMode) {
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
    std::thread resourceTunerListener;
    if(RC_IS_OK(mOpStatus)) {
        try {
            resourceTunerListener = std::thread(listenerThreadStartRoutine);
            TYPELOGD(LISTENER_THREAD_CREATION_SUCCESS);

        } catch(const std::system_error& e) {
            TYPELOGV(SYSTEM_THREAD_CREATION_FAILURE, "Listener", e.what());
            mOpStatus = RC_MODULE_INIT_FAILURE;
        }
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
            std::this_thread::sleep_for(std::chrono::seconds(2));

            if(flags != -1) {
                char exitStatus[16];
                exitStatus[sizeof(exitStatus) - 1] = '\0';
                ssize_t bytesRead = read(STDIN_FILENO, exitStatus, sizeof(exitStatus) - 1);
                if(bytesRead > 0) {
                    if(strncmp(exitStatus, "stop", 4) == 0) {
                        terminateServer = true;
                    }
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

    if(ComponentRegistry::isModuleEnabled(MOD_CORE)) {
        ComponentRegistry::getModuleTeardownCallback(MOD_CORE)();
    }

    if(ComponentRegistry::isModuleEnabled(MOD_SYSSIGNAL)) {
        ComponentRegistry::getModuleTeardownCallback(MOD_SYSSIGNAL)();
    }

    if(resourceTunerListener.joinable()) {
        resourceTunerListener.join();
    }

    // Restore all the Resources to Original Values
    ResourceRegistry::getInstance()->restoreResourcesToDefaultValues();

    if(RequestReceiver::mRequestsThreadPool != nullptr) {
        delete RequestReceiver::mRequestsThreadPool;
    }

    // TBF
    // if(Timer::mTimerThreadPool != nullptr) {
    //     delete Timer::mTimerThreadPool;
    // }

    if(childProcessID != -1) {
        kill(childProcessID, SIGKILL);
        // Wait for the Child Process to terminate
        waitpid(childProcessID, nullptr, 0);
    }

    // Delete the Sysfs Persistent File
    AuxRoutines::deleteFile("sysfsOriginalValues.txt");

    return 0;
}
