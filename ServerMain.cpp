// Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
// SPDX-License-Identifier: BSD-3-Clause-Clear

#include <csignal>
#include <fcntl.h>
#include <sys/wait.h>
#include <dlfcn.h>

#include "ComponentRegistry.h"
#include "ServerInternal.h"
#include "ClientGarbageCollector.h"
#include "PulseMonitor.h"
#include "RequestReceiver.h"

static int8_t terminateServer = false;
static void* extensionsLibHandle = nullptr;

static void handleSIGINT(int32_t sig) {
    terminateServer = true;
}

static void handleSIGTERM(int32_t sig) {
    terminateServer = true;
}

static void handleSIGTSTP(int32_t sig) {
    toggleDisplayModes();
}

static ErrCode parseServerStartupCLIOpts(int32_t argCount, char *argStrings[]) {
    if(argCount != 2) {
        std::cout<<"Usage: "<<argStrings[0]<<" <start|stop|test>"<<std::endl;
        return RC_MODULE_INIT_FAILURE;
    }

    const char* shortPrompts = "sth";
    const struct option longPrompts[] = {
        {"start", no_argument, nullptr, 's'},
        {"test", no_argument, nullptr, 't'},
        {"help", no_argument, nullptr, 'h'},
        {nullptr, no_argument, nullptr, 0}
    };

    int32_t c;
    while ((c = getopt_long(argCount, argStrings, shortPrompts, longPrompts, nullptr)) != -1) {
        switch (c) {
            case 's':
                ResourceTunerSettings::serverInTestMode = false;
                break;
            case 't':
                ResourceTunerSettings::serverInTestMode = true;
                RESTUNE_REGISTER_CONFIG(PROPERTIES_CONFIG, ResourceTunerSettings::mTestPropertiesFilePath)
                RESTUNE_REGISTER_CONFIG(RESOURCE_CONFIG, ResourceTunerSettings::mTestResourceFilePath)
                RESTUNE_REGISTER_CONFIG(SIGNALS_CONFIG, ResourceTunerSettings::mTestSignalFilePath)
                RESTUNE_REGISTER_CONFIG(TARGET_CONFIG, ResourceTunerSettings::mTestTargetConfigFilePath)
                RESTUNE_REGISTER_CONFIG(INIT_CONFIG, ResourceTunerSettings::mTestInitConfigFilePath)
                break;
            case 'h':
                std::cout<<"Help Options"<<std::endl;
                return RC_MODULE_INIT_FAILURE;
            default:
                std::cout<<"Invalid CLI Option specified"<<std::endl;
                return RC_MODULE_INIT_FAILURE;
        }
    }
    return RC_SUCCESS;
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

// Load the Extensions Plugin lib if it is available
// If the lib is not present, we simply return Success. Since this lib is optional
static ErrCode loadExtensionsLib() {
    std::string libPath = ResourceTunerSettings::mExtensionsPluginLibPath;

    // Check if the library file exists
    if(!AuxRoutines::fileExists(libPath)) {
        TYPELOGD(NOTIFY_EXTENSIONS_LIB_NOT_PRESENT);
        return RC_SUCCESS;  // Not an error if the library is not specified
    }

    extensionsLibHandle = dlopen(libPath.c_str(), RTLD_NOW);
    if (extensionsLibHandle == nullptr) {
        TYPELOGV(NOTIFY_EXTENSIONS_LOAD_FAILED, dlerror());
        return RC_MODULE_INIT_FAILURE;  // Error if the library exists but can't be loaded
    }

    TYPELOGD(NOTIFY_EXTENSIONS_LIB_LOADED_SUCCESS);
    return RC_SUCCESS;
}

// Initialize Request and Timer ThreadPools
static ErrCode preAllocateWorkers() {
    int32_t desiredThreadCapacity = ResourceTunerSettings::desiredThreadCount;
    int32_t maxPendingQueueSize = ResourceTunerSettings::maxPendingQueueSize;
    int32_t maxScalingCapacity = ResourceTunerSettings::maxScalingCapacity;

    try {
        RequestReceiver::mRequestsThreadPool = new ThreadPool(desiredThreadCapacity,
                                                              maxPendingQueueSize,
                                                              maxScalingCapacity);

        Timer::mTimerThreadPool = new ThreadPool(desiredThreadCapacity,
                                                 maxPendingQueueSize,
                                                 maxScalingCapacity);

    } catch(const std::bad_alloc& e) {
        TYPELOGV(THREAD_POOL_CREATION_FAILURE, e.what());
        return RC_MODULE_INIT_FAILURE;
    }

    return RC_SUCCESS;
}

static void serverCleanup() {
    LOGE("RESTUNE_SERVER_INIT", "Server Stopped, Cleanup Initiated");
    ResourceTunerSettings::setServerOnlineStatus(false);
}

int32_t main(int32_t argc, char *argv[]) {
    // PID of the Child Daemon
    ErrCode opStatus = RC_SUCCESS;
    int32_t childProcessID = -1;

    std::signal(SIGINT, handleSIGINT);
    std::signal(SIGTSTP, handleSIGTSTP);
    std::signal(SIGTERM, handleSIGTERM);

    if(RC_IS_OK(opStatus)) {
        opStatus = parseServerStartupCLIOpts(argc, argv);
        if(RC_IS_NOTOK(opStatus)) {
            return 0;
        }
    }

    TYPELOGV(NOTIFY_RESOURCE_TUNER_INIT_START, getpid());

    // Start Resource Tuner Server Initialization
    // As part of Server Initialization the Configs (Resource / Signals etc.) will be parsed
    // If any of mandatory Configs cannot be parsed then initialization will fail.
    // Mandatory Configs include: Properties Configs, Resource Configs and Signal Configs (if Signal
    // module is plugged in)
    if(RC_IS_OK(opStatus)) {
        opStatus = createResourceTunerDaemon(childProcessID);
        if(RC_IS_NOTOK(opStatus)) {
            TYPELOGD(RESOURCE_TUNER_DAEMON_CREATION_FAILURE);
        }
    }

    if(RC_IS_OK(opStatus)) {
        // Check if Extensions Plugin lib is available
        opStatus = loadExtensionsLib();
    }

    if(RC_IS_OK(opStatus)) {
        ResourceTunerSettings::setServerOnlineStatus(true);
        ResourceTunerSettings::targetConfigs.currMode = MODE_DISPLAY_ON;
        opStatus = preAllocateWorkers();
    }

    if(RC_IS_OK(opStatus)) {
        opStatus = fetchProperties();
        if(RC_IS_NOTOK(opStatus)) {
            TYPELOGD(PROPERTY_RETRIEVAL_FAILED);
        }
    }

    // Check which modules are plugged In and Initialize them
    if(RC_IS_OK(opStatus)) {
        opStatus = ComponentRegistry::getModuleRegistrationCallback(ModuleIdentifier::MOD_CORE)();
        if(RC_IS_NOTOK(opStatus)) {
            TYPELOGV(MODULE_INIT_FAILED, "Core");
        }
    }

    if(RC_IS_OK(opStatus)) {
        if(ComponentRegistry::isModuleEnabled(ModuleIdentifier::MOD_SYSSIGNAL)) {
            TYPELOGV(NOTIFY_MODULE_ENABLED, "Signal");
            if(RC_IS_OK(opStatus)) {
                opStatus = ComponentRegistry::getModuleRegistrationCallback(ModuleIdentifier::MOD_SYSSIGNAL)();
                if(RC_IS_NOTOK(opStatus)) {
                    TYPELOGV(MODULE_INIT_FAILED, "Signal");
                }
            }
        }
    }

    if(!ResourceTunerSettings::serverInTestMode) {
        // Start the Pulse Monitor and Garbage Collector Daemon Threads
        if(RC_IS_OK(opStatus)) {
            opStatus = startPulseMonitorDaemon();

            if(RC_IS_NOTOK(opStatus)) {
                TYPELOGD(PULSE_MONITOR_INIT_FAILED);
            }
        }

        if(RC_IS_OK(opStatus)) {
            opStatus = startClientGarbageCollectorDaemon();

            if(RC_IS_NOTOK(opStatus)) {
                TYPELOGD(GARBAGE_COLLECTOR_INIT_FAILED);
            }
        }
    }

    // Create a Listener Thread
    std::thread resourceTunerListener;
    if(RC_IS_OK(opStatus)) {
        try {
            resourceTunerListener = std::thread(listenerThreadStartRoutine);
            TYPELOGD(LISTENER_THREAD_CREATION_SUCCESS);

        } catch(const std::system_error& e) {
            TYPELOGV(SYSTEM_THREAD_CREATION_FAILURE, "Listener", e.what());
            opStatus = RC_MODULE_INIT_FAILURE;
        }
    }

    if(RC_IS_OK(opStatus)) {
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

    if(ComponentRegistry::isModuleEnabled(ModuleIdentifier::MOD_CORE)) {
        ComponentRegistry::getModuleTeardownCallback(ModuleIdentifier::MOD_CORE)();
    }

    if(ComponentRegistry::isModuleEnabled(ModuleIdentifier::MOD_SYSSIGNAL)) {
        ComponentRegistry::getModuleTeardownCallback(ModuleIdentifier::MOD_SYSSIGNAL)();
    }

    if(resourceTunerListener.joinable()) {
        resourceTunerListener.join();
    } else {
        TYPELOGV(SYSTEM_THREAD_NOT_JOINABLE, "Listener");
    }

    // Restore all the Resources to Original Values
    ResourceRegistry::getInstance()->restoreResourcesToDefaultValues();

    if(RequestReceiver::mRequestsThreadPool != nullptr) {
        delete RequestReceiver::mRequestsThreadPool;
    }

    if(childProcessID != -1) {
        kill(childProcessID, SIGKILL);
        // Wait for the Child Process to terminate
        waitpid(childProcessID, nullptr, 0);
    }

    // Delete the Sysfs Persistent File
    AuxRoutines::deleteFile("sysfsOriginalValues.txt");

    if(extensionsLibHandle != nullptr) {
        dlclose(extensionsLibHandle);
    }

    return 0;
}
