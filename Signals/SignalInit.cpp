// Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
// SPDX-License-Identifier: BSD-3-Clause-Clear

#include "SysSignalsInternal.h"
#include "SignalServerPrivate.h"
#include "SignalRegistry.h"
#include "SignalQueue.h"
#include "Extensions.h"
#include "ComponentRegistry.h"

static std::thread signalServerProcessorThread;

static void preAllocateMemory() {
    int32_t concurrentRequestsUB = ResourceTunerSettings::metaConfigs.mMaxConcurrentRequests;
    int32_t resourcesPerRequestUB = ResourceTunerSettings::metaConfigs.mMaxResourcesPerRequest;

    MakeAlloc<Signal> (concurrentRequestsUB);
    MakeAlloc<std::vector<Resource*>> (concurrentRequestsUB * resourcesPerRequestUB);
    MakeAlloc<std::vector<uint32_t>> (concurrentRequestsUB * resourcesPerRequestUB);
}

static ErrCode fetchSignals() {
    ErrCode opStatus = RC_SUCCESS;

    TYPELOGV(NOTIFY_PARSING_START, "Signal");

    ConfigProcessor configProcessor;
    std::string filePath = ResourceTunerSettings::mCommonSignalFilePath;
    opStatus = configProcessor.parseSignalConfigs(filePath);
    if(RC_IS_NOTOK(opStatus)) {
        TYPELOGV(NOTIFY_PARSING_FAILURE, "Common-Signal");
        return opStatus;
    }

    filePath = ResourceTunerSettings::mTargetSpecificSignalFilePath;
    opStatus = configProcessor.parseSignalConfigs(filePath);
    if(RC_IS_NOTOK(opStatus)) {
        if(opStatus != RC_FILE_NOT_FOUND) {
            // Additional check for ErrCode, as it is possible that there is
            // no target-specific Configs at all for a given target. This
            // case should not result in a failure.
            TYPELOGV(NOTIFY_PARSING_FAILURE, "Target Specific Signal");
            return opStatus;
        } else {
            // Reset opStatus
            opStatus = RC_SUCCESS;
        }
    }

    filePath = Extensions::getSignalsConfigFilePath();
    if(filePath.length() > 0) {
        // Custom Resource Config file has been provided by BU
        TYPELOGV(NOTIFY_CUSTOM_CONFIG_FILE, "Signal", filePath.c_str());
        opStatus = configProcessor.parseSignalConfigs(filePath);
        if(RC_IS_NOTOK(opStatus)) {
            TYPELOGV(NOTIFY_PARSING_FAILURE, "Custom-Signal");
            return opStatus;
        }
    }

    TYPELOGV(NOTIFY_PARSING_SUCCESS, "Signal");
    return opStatus;
}

ErrCode initSignals() {
    ErrCode opStatus = RC_SUCCESS;

    // Pre-Allocate Memory for Commonly used Types via Memory Pool
    preAllocateMemory();

    // Fetch and Parse Signal Configs
    // Signal Configs which will be considered:
    // - Common Signal Configs
    // - Target Specific Signal Configs
    // - Custom Signal Configs (if present)
    opStatus = fetchSignals();
    if(RC_IS_NOTOK(opStatus)) {
        return opStatus;
    }

    // Fetch and Parse Resource Configs

    // Create Signal Processor thread
    try {
        signalServerProcessorThread = std::thread(SignalsdServerThread);
    } catch(const std::system_error& e) {
        TYPELOGV(SYSTEM_THREAD_CREATION_FAILURE, "Signal", e.what());
        opStatus = RC_MODULE_INIT_FAILURE;
    }

    // Wait for the thread to initialize
    std::this_thread::sleep_for(std::chrono::milliseconds(300));

    return opStatus;
}

ErrCode terminateSignals() {
    // Terminate SysSignals
    // Check if the thread is joinable, to prevent undefined behaviour
    if(signalServerProcessorThread.joinable()) {
        SignalQueue::getInstance()->forcefulAwake();
        signalServerProcessorThread.join();
    } else {
        TYPELOGV(SYSTEM_THREAD_NOT_JOINABLE, "Signal");
    }
    return RC_SUCCESS;
}

RESTUNE_REGISTER_MODULE(MOD_SYSSIGNAL,
                    initSignals,
                    terminateSignals,
                    submitSignalRequest);
