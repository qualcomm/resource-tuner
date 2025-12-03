// Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
// SPDX-License-Identifier: BSD-3-Clause-Clear

#include <iostream>
#include <csignal>
#include <fcntl.h>
#include <sys/wait.h>
#include <dlfcn.h>

#include "ComponentRegistry.h"
#include "ServerInternal.h"
#include "ResourceTunerSettings.h"
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

static ErrCode parseServerStartupCLIOpts(int32_t argCount, char *argStrings[]) {
    if(argCount != 2) {
        std::cout<<"Usage: "<<argStrings[0]<<" --[start|help]"<<std::endl;
        return RC_MODULE_INIT_FAILURE;
    }

    const char* shortPrompts = "sth";
    const struct option longPrompts[] = {
        {"start", no_argument, nullptr, 's'},
        {"help", no_argument, nullptr, 'h'},
        {nullptr, no_argument, nullptr, 0}
    };

    int32_t c;
    while((c = getopt_long(argCount, argStrings, shortPrompts, longPrompts, nullptr)) != -1) {
        switch(c) {
            case 's':
                break;
            case 'h':
                std::cout<<"Help Options"<<std::endl;
                std::cout<<"--start : Start the Resource Tuner Server"<<std::endl;
                return RC_MODULE_INIT_FAILURE;
            default:
                std::cout<<"Invalid CLI Option specified"<<std::endl;
                return RC_MODULE_INIT_FAILURE;
        }
    }
    return RC_SUCCESS;
}

static void restoreToSafeState() {
    if(AuxRoutines::fileExists(ResourceTunerSettings::mPersistenceFile)) {
        AuxRoutines::writeSysFsDefaults();
    }

    // Delete the Node Persistence File
    AuxRoutines::deleteFile(ResourceTunerSettings::mPersistenceFile);
}

// Load the Extensions Plugin lib if it is available
// If the lib is not present, we simply return Success. Since this lib is optional
static ErrCode loadExtensionsLib() {
    std::string libPath = ResourceTunerSettings::mExtensionsPluginLibPath;

    // Check if the library file exists
    extensionsLibHandle = dlopen(libPath.c_str(), RTLD_NOW);
    if(extensionsLibHandle == nullptr) {
        TYPELOGV(NOTIFY_EXTENSIONS_LOAD_FAILED, dlerror());
        return RC_SUCCESS;  // Return success regardless, since this is an extension.
    }

    TYPELOGD(NOTIFY_EXTENSIONS_LIB_LOADED_SUCCESS);
    return RC_SUCCESS;
}

// Initialize Request and Timer ThreadPools
static ErrCode preAllocateWorkers() {
    int32_t desiredThreadCapacity = ResourceTunerSettings::desiredThreadCount;
    int32_t maxScalingCapacity = ResourceTunerSettings::maxScalingCapacity;

    try {
        RequestReceiver::mRequestsThreadPool = new ThreadPool(desiredThreadCapacity,
                                                              maxScalingCapacity);

        // Allocate 2 extra threads for Pulse Monitor and Garbage Collector
        Timer::mTimerThreadPool = new ThreadPool(desiredThreadCapacity + 2,
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
    // Initialize syslog
    openlog(RESTUNE_IDENTIFIER, LOG_PID | LOG_CONS, LOG_USER);

    ErrCode opStatus = RC_SUCCESS;

    std::signal(SIGINT, handleSIGINT);
    std::signal(SIGTERM, handleSIGTERM);

    if(RC_IS_OK(opStatus)) {
        opStatus = parseServerStartupCLIOpts(argc, argv);
        if(RC_IS_NOTOK(opStatus)) {
            return 0;
        }
    }

    TYPELOGV(NOTIFY_RESOURCE_TUNER_INIT_START, getpid());

    // Server might have been restarted by systemd
    // Ensure that Resource Nodes are reset to sane state
    restoreToSafeState();

    // Start Resource Tuner Server Initialization
    // As part of Server Initialization the Configs (Resource / Signals etc.) will be parsed
    // If any of mandatory Configs cannot be parsed then initialization will fail.
    // Mandatory Configs include: Properties Configs, Resource Configs and Signal Configs (if Signal
    // module is plugged in)
    if(RC_IS_OK(opStatus)) {
        // Check if Extensions Plugin lib is available
        opStatus = loadExtensionsLib();
    }

    if(RC_IS_OK(opStatus)) {
        ResourceTunerSettings::setServerOnlineStatus(true);
        ResourceTunerSettings::targetConfigs.currMode = MODE_RESUME;
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
        opStatus = ComponentRegistry::getEventCallback(EventIdentifier::MOD_CORE_INIT)(nullptr);
        if(RC_IS_NOTOK(opStatus)) {
            TYPELOGV(MODULE_INIT_FAILED, "Core");
        }
    }

    if(RC_IS_OK(opStatus)) {
        if(ComponentRegistry::isModuleEnabled(ModuleIdentifier::MOD_SIGNAL)) {
            TYPELOGV(NOTIFY_MODULE_ENABLED, "Signal");
            opStatus = ComponentRegistry::getEventCallback(EventIdentifier::MOD_SIGNAL_INIT)(nullptr);
            if(RC_IS_NOTOK(opStatus)) {
                TYPELOGV(MODULE_INIT_FAILED, "Signal");
            }
        }
    }

    if(RC_IS_OK(opStatus)) {
        if(ComponentRegistry::isModuleEnabled(ModuleIdentifier::MOD_STATE_OPTIMIZER)) {
            TYPELOGV(NOTIFY_MODULE_ENABLED, "Display_Detector");
            opStatus = ComponentRegistry::getEventCallback(EventIdentifier::MOD_STATE_OPTIMIZER_INIT)(nullptr);
            if(RC_IS_NOTOK(opStatus)) {
                TYPELOGV(MODULE_INIT_FAILED, "Display_Detector");
            }
        }
    }

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
        ComponentRegistry::getEventCallback(EventIdentifier::MOD_CORE_TEAR)(nullptr);
    }

    if(ComponentRegistry::isModuleEnabled(ModuleIdentifier::MOD_SIGNAL)) {
        ComponentRegistry::getEventCallback(EventIdentifier::MOD_SIGNAL_TEAR)(nullptr);
    }

    if(ComponentRegistry::isModuleEnabled(ModuleIdentifier::MOD_STATE_OPTIMIZER)) {
        ComponentRegistry::getEventCallback(EventIdentifier::MOD_STATE_OPTIMIZER_TEAR)(nullptr);
    }

    if(resourceTunerListener.joinable()) {
        resourceTunerListener.join();
    } else {
        TYPELOGV(SYSTEM_THREAD_NOT_JOINABLE, "Listener");
    }

    // Restore all the Resources to Original Values
    ResourceRegistry::getInstance()->restoreResourcesToDefaultValues();

    stopPulseMonitorDaemon();
    stopClientGarbageCollectorDaemon();

    if(RequestReceiver::mRequestsThreadPool != nullptr) {
        delete RequestReceiver::mRequestsThreadPool;
    }

    if(Timer::mTimerThreadPool != nullptr) {
        delete Timer::mTimerThreadPool;
    }

    // Delete the Sysfs Persistent File
    AuxRoutines::deleteFile(ResourceTunerSettings::mPersistenceFile);

    if(extensionsLibHandle != nullptr) {
        dlclose(extensionsLibHandle);
    }

    closelog();
    return 0;
}
