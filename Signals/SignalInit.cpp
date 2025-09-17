// Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
// SPDX-License-Identifier: BSD-3-Clause-Clear

#include "SignalInternal.h"
#include "SignalRegistry.h"
#include "SignalQueue.h"
#include "Extensions.h"
#include "ComponentRegistry.h"
#include "SignalConfigProcessor.h"
#include "ResourceTunerSettings.h"

static std::thread signalServerProcessorThread;

static void preAllocateMemory() {
    int32_t concurrentRequestsUB = ResourceTunerSettings::metaConfigs.mMaxConcurrentRequests;
    int32_t resourcesPerRequestUB = ResourceTunerSettings::metaConfigs.mMaxResourcesPerRequest;

    MakeAlloc<Signal> (concurrentRequestsUB);
    MakeAlloc<std::vector<Resource*>> (concurrentRequestsUB * resourcesPerRequestUB);
    MakeAlloc<std::vector<uint32_t>> (concurrentRequestsUB * resourcesPerRequestUB);
}

static ErrCode parseUtil(const std::string& filePath,
                         const std::string& desc,
                         ConfigType configType,
                         int8_t isCustom=false) {

    if(filePath.length() == 0) return RC_FILE_NOT_FOUND;
    ErrCode opStatus = RC_SUCCESS;
    SignalConfigProcessor configProcessor;

    TYPELOGV(NOTIFY_PARSING_START, desc.c_str());
    opStatus = configProcessor.parse(configType, filePath, isCustom);

    if(RC_IS_NOTOK(opStatus)) {
        TYPELOGV(NOTIFY_PARSING_FAILURE, desc.c_str());
        return opStatus;
    }

    TYPELOGV(NOTIFY_PARSING_SUCCESS, desc.c_str());
    return opStatus;
}

static ErrCode fetchSignals() {
    ErrCode opStatus = RC_SUCCESS;

    // Parse Common Signal Configs
    std::string filePath = ResourceTunerSettings::mCommonSignalFilePath;
    opStatus = parseUtil(filePath, COMMON_SIGNAL, ConfigType::SIGNALS_CONFIG);
    if(RC_IS_NOTOK(opStatus)) {
        return opStatus;
    }

    // Parse Custom Signal Configs provided via Extension Interface (if any)
    filePath = Extensions::getSignalsConfigFilePath();
    if(filePath.length() > 0) {
        TYPELOGV(NOTIFY_CUSTOM_CONFIG_FILE, "Signal", filePath.c_str());
        return parseUtil(filePath, CUSTOM_SIGNAL, ConfigType::SIGNALS_CONFIG, true);
    }

    // Parse Custom Signal Configs provided in /etc/resource-tuner/custom (if any)
    filePath = ResourceTunerSettings::mCustomSignalFilePath;
    if(AuxRoutines::fileExists(filePath)) {
        return parseUtil(filePath, CUSTOM_SIGNAL, ConfigType::SIGNALS_CONFIG, true);
    }

    return opStatus;
}

// Since this is a Custom (Optional) Config, hence if the expected Config file is
// not found, we simply return Success.
static ErrCode fetchExtFeatureConfigs() {
    ErrCode opStatus = RC_SUCCESS;

    // Check if a Custom Target Config is provided, if so process it.
    std::string filePath = Extensions::getExtFeaturesConfigFilePath();

    if(filePath.length() > 0) {
        // Custom Target Config file has been provided by BU
        TYPELOGV(NOTIFY_CUSTOM_CONFIG_FILE, CUSTOM_EXT_FEATURE, filePath.c_str());
        return parseUtil(filePath, CUSTOM_EXT_FEATURE, ConfigType::EXT_FEATURES_CONFIG, true);
    }

    filePath = ResourceTunerSettings::mCustomExtFeaturesFilePath;
    if(AuxRoutines::fileExists(filePath)) {
        return parseUtil(filePath, CUSTOM_EXT_FEATURE, ConfigType::EXT_FEATURES_CONFIG, true);
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
