// Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
// SPDX-License-Identifier: BSD-3-Clause-Clear

#include "SignalInternal.h"
#include "SignalRegistry.h"
#include "SignalQueue.h"
#include "Extensions.h"
#include "ComponentRegistry.h"
#include "ConfigProcessor.h"
#include "ResourceTunerSettings.h"

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

    ConfigProcessor configProcessor;

    TYPELOGV(NOTIFY_PARSING_START, "Common-Signal");
    std::string filePath = ResourceTunerSettings::mCommonSignalFilePath;
    opStatus = configProcessor.parseSignalConfigs(filePath);
    if(RC_IS_NOTOK(opStatus)) {
        TYPELOGV(NOTIFY_PARSING_FAILURE, "Common-Signal");
        return opStatus;
    }
    TYPELOGV(NOTIFY_PARSING_SUCCESS, "Common-Signal");

    filePath = Extensions::getSignalsConfigFilePath();
    if(filePath.length() > 0) {
        TYPELOGV(NOTIFY_CUSTOM_CONFIG_FILE, "Signal", filePath.c_str());
        TYPELOGV(NOTIFY_PARSING_START, "Custom-Signal");
        opStatus = configProcessor.parseSignalConfigs(filePath, true);
        if(RC_IS_NOTOK(opStatus)) {
            TYPELOGV(NOTIFY_PARSING_FAILURE, "Custom-Signal");
            return opStatus;
        }
        TYPELOGV(NOTIFY_PARSING_SUCCESS, "Custom-Signal");

    } else {
        TYPELOGV(NOTIFY_PARSING_START, "Custom-Signal");
        filePath = ResourceTunerSettings::mCustomSignalFilePath;
        opStatus = configProcessor.parseSignalConfigs(filePath, true);
        if(RC_IS_NOTOK(opStatus)) {
            if(opStatus == RC_FILE_NOT_FOUND) {
                TYPELOGV(NOTIFY_PARSER_FILE_NOT_FOUND, "Custom-Signal", filePath.c_str());
                return RC_SUCCESS;
            }
            TYPELOGV(NOTIFY_PARSING_FAILURE, "Custom-Signal");
            return opStatus;
        }
        TYPELOGV(NOTIFY_PARSING_SUCCESS, "Custom-Signal");
    }

    return opStatus;
}

// Since this is a Custom (Optional) Config, hence if the expected Config file is
// not found, we simply return Success.
static ErrCode fetchExtFeatureConfigs() {
    ErrCode opStatus = RC_SUCCESS;

    ConfigProcessor configProcessor;
    std::string filePath = Extensions::getExtFeaturesConfigFilePath();
    if(filePath.length() > 0) {
        // Custom Resource Config file has been provided by BU
        TYPELOGV(NOTIFY_CUSTOM_CONFIG_FILE, "Ext-Features", filePath.c_str());
        TYPELOGV(NOTIFY_PARSING_START, "Ext-Features");

        opStatus = configProcessor.parseExtFeaturesConfigs(filePath);
        if(RC_IS_NOTOK(opStatus)) {
            TYPELOGV(NOTIFY_PARSING_FAILURE, "Ext-Features");
            return opStatus;
        }

    } else {
        TYPELOGV(NOTIFY_PARSING_START, "Ext-Features");
        filePath = ResourceTunerSettings::mCustomExtFeaturesFilePath;
        opStatus = configProcessor.parseExtFeaturesConfigs(filePath);

        if(RC_IS_NOTOK(opStatus)) {
            if(opStatus == RC_FILE_NOT_FOUND) {
                TYPELOGV(NOTIFY_PARSER_FILE_NOT_FOUND, "Ext-Features", filePath.c_str());
                return RC_SUCCESS;
            }
            TYPELOGV(NOTIFY_PARSING_FAILURE, "Ext-Features");
            return opStatus;
        }
        TYPELOGV(NOTIFY_PARSING_SUCCESS, "Ext-Features");
    }

    return opStatus;
}

static void* signalThreadStart() {
    std::shared_ptr<SignalQueue> signalQueue = SignalQueue::getInstance();
    while(ResourceTunerSettings::isServerOnline()) {
        signalQueue->wait();
    }

    return nullptr;
}

static ErrCode init(void* arg=nullptr) {
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

    // Fetch and Parse Extension Features Configs
    opStatus = fetchExtFeatureConfigs();
    if(RC_IS_NOTOK(opStatus)) {
        return opStatus;
    }

    ExtFeaturesRegistry::getInstance()->initializeFeatures();

    // Create Signal Processor thread
    try {
        signalServerProcessorThread = std::thread(signalThreadStart);
    } catch(const std::system_error& e) {
        TYPELOGV(SYSTEM_THREAD_CREATION_FAILURE, "Signal", e.what());
        opStatus = RC_MODULE_INIT_FAILURE;
    }

    // Wait for the thread to initialize
    std::this_thread::sleep_for(std::chrono::milliseconds(300));

    return opStatus;
}

static ErrCode terminate(void* arg=nullptr) {
    // Terminate Signal module
    // Check if the thread is joinable, to prevent undefined behaviour
    if(signalServerProcessorThread.joinable()) {
        SignalQueue::getInstance()->forcefulAwake();
        signalServerProcessorThread.join();
    } else {
        TYPELOGV(SYSTEM_THREAD_NOT_JOINABLE, "Signal");
    }

    ExtFeaturesRegistry::getInstance()->teardownFeatures();
    return RC_SUCCESS;
}

RESTUNE_REGISTER_MODULE(MOD_SIGNAL, init, terminate, submitSignalRequest);
