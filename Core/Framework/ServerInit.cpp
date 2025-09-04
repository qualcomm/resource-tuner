// Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
// SPDX-License-Identifier: BSD-3-Clause-Clear

#include <csignal>
#include <fcntl.h>
#include <sys/utsname.h>

#include "ServerInternal.h"
#include "ConfigProcessor.h"
#include "ComponentRegistry.h"

static std::thread serverThread;

static void preAllocateMemory() {
    // Preallocate Memory for certain frequently used types.
    int32_t concurrentRequestsUB = ResourceTunerSettings::metaConfigs.mMaxConcurrentRequests;
    int32_t resourcesPerRequestUB = ResourceTunerSettings::metaConfigs.mMaxResourcesPerRequest;

    int32_t maxBlockCount = concurrentRequestsUB * resourcesPerRequestUB;

    MakeAlloc<Message> (concurrentRequestsUB);
    MakeAlloc<Request> (concurrentRequestsUB);
    MakeAlloc<Timer> (concurrentRequestsUB);
    MakeAlloc<Resource> (maxBlockCount);
    MakeAlloc<CocoNode> (maxBlockCount);
    MakeAlloc<ClientInfo> (maxBlockCount);
    MakeAlloc<ClientTidData> (maxBlockCount);
    MakeAlloc<std::unordered_set<int64_t>> (maxBlockCount);
    MakeAlloc<std::vector<Resource*>> (maxBlockCount);
    MakeAlloc<std::vector<int32_t>> (maxBlockCount);
    MakeAlloc<std::vector<CocoNode*>> (maxBlockCount);
    MakeAlloc<MsgForwardInfo>(maxBlockCount);
    MakeAlloc<char[REQ_BUFFER_SIZE]> (maxBlockCount);
}

static ErrCode fetchMetaConfigs() {
    std::string resultBuffer;

    try {
        // Fetch target Name
        struct utsname sysInfo;
        if(uname(&sysInfo) == -1) {
            TYPELOGV(ERRNO_LOG, "uname", strerror(errno));
            return RC_PROP_PARSING_ERROR;
        }

        ResourceTunerSettings::targetConfigs.targetName = sysInfo.nodename;

        submitPropGetRequest(MAX_CONCURRENT_REQUESTS, resultBuffer, "120");
        ResourceTunerSettings::metaConfigs.mMaxConcurrentRequests = (uint32_t)std::stol(resultBuffer);

        submitPropGetRequest(MAX_RESOURCES_PER_REQUEST, resultBuffer, "5");
        ResourceTunerSettings::metaConfigs.mMaxResourcesPerRequest = (uint32_t)std::stol(resultBuffer);

        // Hard Code this value, as it should not be end-client customisable
        ResourceTunerSettings::metaConfigs.mListeningPort = 12000;

        submitPropGetRequest(PULSE_MONITOR_DURATION, resultBuffer, "60000");
        ResourceTunerSettings::metaConfigs.mPulseDuration = (uint32_t)std::stol(resultBuffer);

        submitPropGetRequest(GARBAGE_COLLECTOR_DURATION, resultBuffer, "83000");
        ResourceTunerSettings::metaConfigs.mClientGarbageCollectorDuration = (uint32_t)std::stol(resultBuffer);

        ResourceTunerSettings::metaConfigs.mCleanupBatchSize = 5;

        submitPropGetRequest(RATE_LIMITER_DELTA, resultBuffer, "5");
        ResourceTunerSettings::metaConfigs.mDelta = (uint32_t)std::stol(resultBuffer);

        submitPropGetRequest(RATE_LIMITER_PENALTY_FACTOR, resultBuffer, "2.0");
        ResourceTunerSettings::metaConfigs.mPenaltyFactor = std::stod(resultBuffer);

        submitPropGetRequest(RATE_LIMITER_REWARD_FACTOR, resultBuffer, "0.4");
        ResourceTunerSettings::metaConfigs.mRewardFactor = std::stod(resultBuffer);

        submitPropGetRequest(LOGGER_LOGGING_LEVEL, resultBuffer, "2");
        int8_t logLevel = (int8_t)std::stoi(resultBuffer);

        int8_t levelSpecificLogging = false;
        submitPropGetRequest(LOGGER_LOGGING_LEVEL_TYPE, resultBuffer, "false");
        if(resultBuffer == "true") {
            levelSpecificLogging = true;
        }

        RedirectOptions redirectOutputTo = LOG_FILE;
        submitPropGetRequest(LOGGER_LOGGING_OUTPUT_REDIRECT, resultBuffer, "1");
        if((int8_t)std::stoi(resultBuffer) == 0) {
            redirectOutputTo = FTRACE;
        }

        Logger::configure(logLevel, levelSpecificLogging, redirectOutputTo);

    } catch(const std::invalid_argument& e) {
        TYPELOGV(META_CONFIG_PARSE_FAILURE, e.what());
        return RC_PROP_PARSING_ERROR;

    } catch(const std::out_of_range& e) {
        TYPELOGV(META_CONFIG_PARSE_FAILURE, e.what());
        return RC_PROP_PARSING_ERROR;
    }

    return RC_SUCCESS;
}

static ErrCode parseUtil(const std::string& filePath,
                         const std::string& desc,
                         ConfigType configType,
                         int8_t isCustom=false) {

    if(filePath.length() == 0) return RC_FILE_NOT_FOUND;
    ErrCode opStatus = RC_SUCCESS;
    ConfigProcessor configProcessor;

    TYPELOGV(NOTIFY_PARSING_START, desc.c_str());
    opStatus = configProcessor.parse(configType, filePath, isCustom);

    if(RC_IS_NOTOK(opStatus)) {
        TYPELOGV(NOTIFY_PARSING_FAILURE, desc.c_str());
        return opStatus;
    }

    TYPELOGV(NOTIFY_PARSING_SUCCESS, desc.c_str());
    return opStatus;
}

ErrCode fetchProperties() {
    ErrCode opStatus = RC_SUCCESS;

    // Parse Common Properties Configs
    std::string filePath = ResourceTunerSettings::mCommonPropertiesFilePath;
    opStatus = parseUtil(filePath, COMMON_PROPERTIES, ConfigType::PROPERTIES_CONFIG);
    if(RC_IS_NOTOK(opStatus)) {
        // Common Properties Parsing Failed
        return opStatus;
    }

    filePath = Extensions::getPropertiesConfigFilePath();
    // Parse Custom Properties Configs provided via Extension Interface (if any)
    if(filePath.length() > 0) {
        TYPELOGV(NOTIFY_CUSTOM_CONFIG_FILE, "Property", filePath.c_str());

        opStatus = parseUtil(filePath, CUSTOM_PROPERTIES, ConfigType::PROPERTIES_CONFIG);
        if(RC_IS_OK(opStatus)) {
            // Properties Parsing is completed
            return fetchMetaConfigs();
        }

        if(opStatus != RC_FILE_NOT_FOUND) {
            // Custom Properties Parsing Failed
            return opStatus;
        }
    }

    // Parse Custom Properties Configs provided in /etc/resource-tuner/custom (if any)
    filePath = ResourceTunerSettings::mCustomPropertiesFilePath;
    opStatus = parseUtil(filePath, CUSTOM_PROPERTIES, ConfigType::PROPERTIES_CONFIG);

    // If file was not found, we simply return SUCCESS, since custom configs are optional
    if(opStatus != RC_FILE_NOT_FOUND) {
        // Custom Properties Parsing Failed
        return opStatus;
    }

    return fetchMetaConfigs();
}

static ErrCode fetchResources() {
    ErrCode opStatus = RC_SUCCESS;

    // Parse Common Resource Configs
    std::string filePath = ResourceTunerSettings::mCommonResourceFilePath;
    opStatus = parseUtil(filePath, COMMON_RESOURCE, ConfigType::RESOURCE_CONFIG);
    if(RC_IS_NOTOK(opStatus)) {
        return opStatus;
    }

    // Parse Custom Resource Configs provided via Extension Interface (if any)
    filePath = Extensions::getResourceConfigFilePath();
    if(filePath.length() > 0) {
        TYPELOGV(NOTIFY_CUSTOM_CONFIG_FILE, CUSTOM_RESOURCE, filePath.c_str());
        opStatus = parseUtil(filePath, CUSTOM_RESOURCE, ConfigType::RESOURCE_CONFIG, true);

        if(RC_IS_OK(opStatus)) {
            // Resource Config Parsing is completed
            return opStatus;
        }

        if(opStatus != RC_FILE_NOT_FOUND) {
            // Custom Resource Parsing Failed
            return opStatus;
        }
    }

    // Parse Custom Resource Configs provided in /etc/resource-tuner/custom (if any)
    filePath = ResourceTunerSettings::mCustomResourceFilePath;
    opStatus = parseUtil(filePath, CUSTOM_RESOURCE, ConfigType::RESOURCE_CONFIG, true);

    // If file was not found, we simply return SUCCESS, since custom configs are optional
    if(opStatus == RC_FILE_NOT_FOUND) {
        return RC_SUCCESS;
    }

    return opStatus;
}

static ErrCode fetchTargetInfo() {
    ErrCode opStatus = RC_SUCCESS;

    // Check if a Custom Target Config is provided, if so process it. Else, resort
    // to the default Target Config File, and see if a config is listed for this Target
    // in the Common Configs.
    std::string filePath = Extensions::getTargetConfigFilePath();

    if(filePath.length() > 0) {
        // Custom Target Config file has been provided by BU
        TYPELOGV(NOTIFY_CUSTOM_CONFIG_FILE, CUSTOM_TARGET, filePath.c_str());
        return parseUtil(filePath, CUSTOM_TARGET, ConfigType::TARGET_CONFIG, true);
    }

    filePath = ResourceTunerSettings::mCustomTargetFilePath;
    opStatus = parseUtil(filePath, CUSTOM_TARGET, ConfigType::TARGET_CONFIG, true);

    if(RC_IS_OK(opStatus)) {
        // Target Configs successfully parsed
        return opStatus;
    }

    if(opStatus != RC_FILE_NOT_FOUND) {
        return opStatus;
    }
    TYPELOGV(NOTIFY_PARSER_FILE_NOT_FOUND, CUSTOM_TARGET, filePath.c_str());

    // Note Common Target Configs should only be parsed if Custom Target
    // Configs have not been provided
    filePath = ResourceTunerSettings::mCommonTargetFilePath;
    opStatus = parseUtil(filePath, COMMON_TARGET, ConfigType::TARGET_CONFIG);

    return opStatus;
}

static ErrCode fetchInitInfo() {
    ErrCode opStatus = RC_SUCCESS;
    std::string filePath = ResourceTunerSettings::mCommonInitConfigFilePath;

    opStatus = parseUtil(filePath, COMMON_INIT, ConfigType::INIT_CONFIG);
    if(RC_IS_NOTOK(opStatus)) {
        return opStatus;
    }

    filePath = Extensions::getInitConfigFilePath();
    if(filePath.length() > 0) {
        // Custom Init Config file has been provided by BU
        TYPELOGV(NOTIFY_PARSER_FILE_NOT_FOUND, CUSTOM_INIT, filePath.c_str());
        opStatus = parseUtil(filePath, CUSTOM_INIT, ConfigType::INIT_CONFIG);
        if(RC_IS_OK(opStatus)) {
            // Init Config Parsing is completed
            return opStatus;
        }

        if(opStatus != RC_FILE_NOT_FOUND) {
            // Custom Resource Parsing Failed
            return opStatus;
        }
    }

    // Parse Custom Init Configs provided in /etc/resource-tuner/custom (if any)
    filePath = ResourceTunerSettings::mCustomInitConfigFilePath;
    opStatus = parseUtil(filePath, CUSTOM_INIT, ConfigType::INIT_CONFIG);

    // If file was not found, we simply return SUCCESS, since custom configs are optional
    if(opStatus == RC_FILE_NOT_FOUND) {
        return RC_SUCCESS;
    }

    return opStatus;
}

static void* serverThreadStart() {
    std::shared_ptr<RequestQueue> requestQueue = RequestQueue::getInstance();

    // Initialize CocoTable
    CocoTable::getInstance();
    while(ResourceTunerSettings::isServerOnline()) {
        requestQueue->wait();
    }

    return nullptr;
}

static ErrCode init(void* arg=nullptr) {
    ErrCode opStatus = RC_SUCCESS;

    // Pre-Allocate Memory for Commonly used Types via Memory Pool
    preAllocateMemory();

    // Fetch and Parse:
    // - Custom Target Configs (if present)
    // - Init Configs
    opStatus = fetchInitInfo();
    if(RC_IS_NOTOK(opStatus)) {
        return opStatus;
    }

    // Perform Logical To Physical (Core / Cluster) Mapping
    // Note we don't perform error-checking here since the behaviour of this
    // routine is target / architecture specific, and the initialization flow
    // needs to be generic enough to accomodate them.
    TargetRegistry::getInstance()->readTargetInfo();
    TargetRegistry::getInstance()->displayTargetInfo();

    // Fetch and Parse Resource Configs
    // Resource Parsing which will be considered:
    // - Common Resource Configs
    // - Target Specific Resource Configs
    // - Custom Resource Configs (if present)
    // Note by this point, we will know the Target Info, i.e. number of Core, Clusters etc.
    opStatus = fetchResources();
    if(RC_IS_NOTOK(opStatus)) {
        return opStatus;
    }

    // By this point, all the Extension Appliers / Resources would have been registered.
    ResourceRegistry::getInstance()->pluginModifications();

    // Create the Processor thread:
    try {
        serverThread = std::thread(serverThreadStart);
    } catch(const std::system_error& e) {
        TYPELOGV(SYSTEM_THREAD_CREATION_FAILURE, "Server", e.what());
        opStatus = RC_MODULE_INIT_FAILURE;
    }

    // Wait for the thread to initialize
    std::this_thread::sleep_for(std::chrono::milliseconds(300));

    return opStatus;
}

static ErrCode terminate(void* arg=nullptr) {
    // Check if the thread is joinable, to prevent undefined behaviour
    if(serverThread.joinable()) {
        RequestQueue::getInstance()->forcefulAwake();
        serverThread.join();
    } else {
        TYPELOGV(SYSTEM_THREAD_NOT_JOINABLE, "Server");
    }
    return RC_SUCCESS;
}

RESTUNE_REGISTER_MODULE(MOD_CORE, init, terminate, submitResProvisionRequest);
RESTUNE_REGISTER_EVENT_CALLBACK(PROP_ON_MSG_RECV, submitPropRequest);
