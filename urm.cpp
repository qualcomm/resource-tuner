// Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
// SPDX-License-Identifier: BSD-3-Clause-Clear

#include <csignal>
#include <thread>

#include "Utils.h"
#include "Common.h"
#include "Logger.h"
#include "Timer.h"
#include "RequestReceiver.h"
#include "ComponentRegistry.h"

static int8_t terminateServer = false;

static void handleSIGINT(int32_t sig) {
    terminateServer = true;
}

static void handleSIGTERM(int32_t sig) {
    terminateServer = true;
}

static ErrCode initModuleIfPresent(ModuleID moduleId) {
    // Check if the module is plugged in and Initialize it
    ModuleInfo modInfo = ComponentRegistry::getModuleInfo(moduleId);
    if(modInfo.mInit == nullptr) {
        // Module not enabled
        return RC_SUCCESS;
    }

    return modInfo.mInit(nullptr);
}

static ErrCode cleanupModule(ModuleID moduleId) {
    // Check if the module is plugged in and Initialize it
    ModuleInfo modInfo = ComponentRegistry::getModuleInfo(moduleId);
    if(modInfo.mTear == nullptr) {
        // Module not enabled
        return RC_SUCCESS;
    }

    return modInfo.mTear(nullptr);
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

    TYPELOGV(NOTIFY_RESOURCE_TUNER_INIT_START, getpid());

    if(RC_IS_OK(opStatus)) {
        opStatus = initModuleIfPresent(ModuleID::MOD_RESTUNE);
        if(RC_IS_NOTOK(opStatus)) {
            TYPELOGV(MODULE_INIT_FAILED, "resource-tuner");
        }
    }

    if(RC_IS_OK(opStatus)) {
        opStatus = initModuleIfPresent(ModuleID::MOD_CLASSIFIER);
        if(RC_IS_NOTOK(opStatus)) {
            TYPELOGV(MODULE_INIT_FAILED, "classifier");
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

    // Ready for requests
    if(RC_IS_OK(opStatus)) {
        ResourceTunerSettings::setServerOnlineStatus(true);
        ResourceTunerSettings::targetConfigs.currMode = MODE_RESUME;
    }

    if(RC_IS_OK(opStatus)) {
        // Listen for Terminal prompts
        while(!terminateServer) {
            std::this_thread::sleep_for(std::chrono::seconds(2));
        }
    }

    // Cleanup the Server, so that it boots up correctly the next time
    // - This includes Closing the Server Communication Endpoints
    // - Resetting all the Sysfs nodes to their original values
    // - Terminating the different listener and Processor threads created to handle Requests.
    // - Killing the child process created to monitor the parent (Server)
    serverCleanup();

    cleanupModule(ModuleID::MOD_RESTUNE);
    cleanupModule(ModuleID::MOD_CLASSIFIER);

    if(resourceTunerListener.joinable()) {
        resourceTunerListener.join();
    } else {
        TYPELOGV(SYSTEM_THREAD_NOT_JOINABLE, "Listener");
    }

    closelog();
    return 0;
}
