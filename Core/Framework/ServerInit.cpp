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

    MakeAlloc<Message> (concurrentRequestsUB);
    MakeAlloc<Request> (concurrentRequestsUB);
    MakeAlloc<Timer> (concurrentRequestsUB);
    MakeAlloc<Resource> (concurrentRequestsUB * resourcesPerRequestUB);
    MakeAlloc<CocoNode> (concurrentRequestsUB * resourcesPerRequestUB);
    MakeAlloc<SysConfig> (concurrentRequestsUB);
    MakeAlloc<ClientInfo> (concurrentRequestsUB * resourcesPerRequestUB);
    MakeAlloc<ClientTidData> (concurrentRequestsUB * resourcesPerRequestUB);
    MakeAlloc<std::unordered_set<int64_t>> (concurrentRequestsUB * resourcesPerRequestUB);
    MakeAlloc<std::vector<Resource*>> (concurrentRequestsUB * resourcesPerRequestUB);
    MakeAlloc<std::vector<int32_t>> (concurrentRequestsUB * resourcesPerRequestUB);
    MakeAlloc<std::vector<CocoNode*>> (concurrentRequestsUB * resourcesPerRequestUB);
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

        submitPropGetRequest("resource_tuner.maximum.concurrent.requests", resultBuffer, sizeof(resultBuffer), "120");
        ResourceTunerSettings::metaConfigs.mMaxConcurrentRequests = (uint32_t)std::stol(resultBuffer);

        submitPropGetRequest("resource_tuner.maximum.resources.per.request", resultBuffer, sizeof(resultBuffer), "5");
        ResourceTunerSettings::metaConfigs.mMaxResourcesPerRequest = (uint32_t)std::stol(resultBuffer);

        // Hard Code this value, as it should not be end-client customisable
        ResourceTunerSettings::metaConfigs.mListeningPort = 12000;

        submitPropGetRequest("resource_tuner.pulse.duration", resultBuffer, sizeof(resultBuffer), "60000");
        ResourceTunerSettings::metaConfigs.mPulseDuration = (uint32_t)std::stol(resultBuffer);

        submitPropGetRequest("resource_tuner.garbage_collection.duration", resultBuffer, sizeof(resultBuffer), "83000");
        ResourceTunerSettings::metaConfigs.mClientGarbageCollectorDuration = (uint32_t)std::stol(resultBuffer);

        submitPropGetRequest("resource_tuner.rate_limiter.delta", resultBuffer, sizeof(resultBuffer), "5");
        ResourceTunerSettings::metaConfigs.mDelta = (uint32_t)std::stol(resultBuffer);

        submitPropGetRequest("resource_tuner.penalty.factor", resultBuffer, sizeof(resultBuffer), "2.0");
        ResourceTunerSettings::metaConfigs.mPenaltyFactor = std::stod(resultBuffer);

        submitPropGetRequest("resource_tuner.reward.factor", resultBuffer, sizeof(resultBuffer), "0.4");
        ResourceTunerSettings::metaConfigs.mRewardFactor = std::stod(resultBuffer);

        submitPropGetRequest("resource_tuner.logging.level", resultBuffer, sizeof(resultBuffer), "2");
        int8_t logLevel = (int8_t)std::stoi(resultBuffer);

        int8_t levelSpecificLogging = false;
        submitPropGetRequest("resource_tuner.logging.level.exact", resultBuffer, sizeof(resultBuffer), "false");
        if(resultBuffer == "true") {
            levelSpecificLogging = true;
        }

        RedirectOptions redirectOutputTo = LOG_FILE;
        submitPropGetRequest("resource_tuner.logging.redirect_to", resultBuffer, sizeof(resultBuffer), "1");
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

ErrCode fetchProperties() {
    // Initialize SysConfigs
    ErrCode opStatus = RC_SUCCESS;
    ConfigProcessor configProcessor;

    TYPELOGV(NOTIFY_PARSING_START, "Common-Properties");
    std::string filePath = ResourceTunerSettings::mCommonPropertiesFilePath;
    opStatus = configProcessor.parsePropertiesConfigs(filePath);

    if(RC_IS_NOTOK(opStatus)) {
        TYPELOGV(NOTIFY_PARSING_FAILURE, "Common-Properties");
        return opStatus;
    }
    TYPELOGV(NOTIFY_PARSING_SUCCESS, "Common-Properties");

    filePath = Extensions::getPropertiesConfigFilePath();
    if(filePath.length() > 0) {
        TYPELOGV(NOTIFY_CUSTOM_CONFIG_FILE, "Properties", filePath.c_str());
        TYPELOGV(NOTIFY_PARSING_START, "Custom-Properties");

        opStatus = configProcessor.parsePropertiesConfigs(filePath);
        if(RC_IS_NOTOK(opStatus)) {
            TYPELOGV(NOTIFY_PARSING_FAILURE, "Custom-Properties");
            return opStatus;
        }
        TYPELOGV(NOTIFY_PARSING_SUCCESS, "Custom-Properties");

    } else {
        TYPELOGV(NOTIFY_PARSING_START, "Custom-Properties");
        filePath = ResourceTunerSettings::mCustomPropertiesFilePath;
        opStatus = configProcessor.parsePropertiesConfigs(filePath);

        if(RC_IS_NOTOK(opStatus)) {
            if(opStatus == RC_FILE_NOT_FOUND) {
                TYPELOGV(NOTIFY_PARSER_FILE_NOT_FOUND, "Custom-Properties", filePath.c_str());
                // Reset the status back to Success, as the custom config is optional
                opStatus = RC_SUCCESS;
            } else {
                TYPELOGV(NOTIFY_PARSING_FAILURE, "Custom-Properties");
                return opStatus;
            }
        } else {
            TYPELOGV(NOTIFY_PARSING_SUCCESS, "Custom-Properties");
        }
    }

    if(RC_IS_OK(opStatus)) {
        opStatus = fetchMetaConfigs();
    }

    return opStatus;
}

static ErrCode fetchResources() {
    ErrCode opStatus = RC_SUCCESS;
    ConfigProcessor configProcessor;

    TYPELOGV(NOTIFY_PARSING_START, "Common-Resource");
    std::string filePath = ResourceTunerSettings::mCommonResourceFilePath;
    opStatus = configProcessor.parseResourceConfigs(filePath);

    if(RC_IS_NOTOK(opStatus)) {
        TYPELOGV(NOTIFY_PARSING_FAILURE, "Common-Resource");
        return opStatus;
    }
    TYPELOGV(NOTIFY_PARSING_SUCCESS, "Common-Resource");

    filePath = Extensions::getResourceConfigFilePath();
    if(filePath.length() > 0) {
        TYPELOGV(NOTIFY_CUSTOM_CONFIG_FILE, "Resource", filePath.c_str());
        TYPELOGV(NOTIFY_PARSING_START, "Custom-Resource");

        opStatus = configProcessor.parseResourceConfigs(filePath, true);
        if(RC_IS_NOTOK(opStatus)) {
            TYPELOGV(NOTIFY_PARSING_FAILURE, "Custom-Resource");
            return opStatus;
        }
        TYPELOGV(NOTIFY_PARSING_SUCCESS, "Custom-Resource");

    } else {
        TYPELOGV(NOTIFY_PARSING_START, "Custom-Resource");
        filePath = ResourceTunerSettings::mCustomResourceFilePath;
        opStatus = configProcessor.parseResourceConfigs(filePath, true);

        if(RC_IS_NOTOK(opStatus)) {
            if(opStatus == RC_FILE_NOT_FOUND) {
                TYPELOGV(NOTIFY_PARSER_FILE_NOT_FOUND, "Custom-Resource", filePath.c_str());
                return RC_SUCCESS;
            }

            TYPELOGV(NOTIFY_PARSING_FAILURE, "Custom-Resource");
            return opStatus;
        }
        TYPELOGV(NOTIFY_PARSING_SUCCESS, "Custom-Resource");
    }

    return opStatus;
}

static ErrCode fetchInitInfo() {
    ErrCode opStatus = RC_SUCCESS;

    // Check if a Custom Target Config is provided, if so process it. Else, resort
    // to the default Target Config File, and see if a config is listed for this Target.
    int8_t isCustomConfigAvailable = false;
    ConfigProcessor configProcessor;
    std::string filePath = Extensions::getTargetConfigFilePath();

    if(filePath.length() > 0) {
        // Custom Target Config file has been provided by BU
        isCustomConfigAvailable = true;
        TYPELOGV(NOTIFY_CUSTOM_CONFIG_FILE, "Target", filePath.c_str());
        opStatus = configProcessor.parseTargetConfigs(filePath, true);
        if(RC_IS_NOTOK(opStatus)) {
            TYPELOGV(NOTIFY_PARSING_FAILURE, "Target");
            return opStatus;
        }
        TYPELOGV(NOTIFY_PARSING_SUCCESS, "Target");

    } else {
        TYPELOGV(NOTIFY_PARSING_START, "Target");
        filePath = ResourceTunerSettings::mCustomTargetFilePath;
        opStatus = configProcessor.parseTargetConfigs(filePath, true);
        if(RC_IS_NOTOK(opStatus)) {
            if(opStatus != RC_FILE_NOT_FOUND) {
                isCustomConfigAvailable = true;
                TYPELOGV(NOTIFY_PARSING_FAILURE, "Target");
                return opStatus;
            } else {
                TYPELOGV(NOTIFY_PARSER_FILE_NOT_FOUND, "Target", filePath.c_str());
                opStatus = RC_SUCCESS;
            }
        } else {
            isCustomConfigAvailable = true;
            TYPELOGV(NOTIFY_PARSING_SUCCESS, "Target");
        }
    }

    if(!isCustomConfigAvailable) {
        TYPELOGV(NOTIFY_PARSING_START, "Common-Target");
        filePath = ResourceTunerSettings::mCommonTargetFilePath;
        opStatus = configProcessor.parseTargetConfigs(filePath);

        if(RC_IS_NOTOK(opStatus)) {
            TYPELOGV(NOTIFY_PARSING_FAILURE, "Common-Target");
            return opStatus;
        }

        TYPELOGV(NOTIFY_PARSING_SUCCESS, "Common-Target");
    }

    TYPELOGV(NOTIFY_PARSING_START, "Common-Init");
    filePath = ResourceTunerSettings::mCommonInitConfigFilePath;
    opStatus = configProcessor.parseInitConfigs(filePath);

    if(RC_IS_NOTOK(opStatus)) {
        TYPELOGV(NOTIFY_PARSING_FAILURE, "Common-Init");
        return opStatus;
    }
    TYPELOGV(NOTIFY_PARSING_SUCCESS, "Common-Init");

    filePath = Extensions::getInitConfigFilePath();
    if(filePath.length() > 0) {
        // Custom Target Config file has been provided by BU
        TYPELOGV(NOTIFY_CUSTOM_CONFIG_FILE, "Init", filePath.c_str());
        opStatus = configProcessor.parseInitConfigs(filePath);
        if(RC_IS_NOTOK(opStatus)) {
            TYPELOGV(NOTIFY_PARSING_FAILURE, "Custom-Init");
            return opStatus;
        }
        TYPELOGV(NOTIFY_PARSING_SUCCESS, "Custom-Init");

    } else {
        TYPELOGV(NOTIFY_PARSING_START, "Custom-Init");
        filePath = ResourceTunerSettings::mCustomInitConfigFilePath;
        opStatus = configProcessor.parseInitConfigs(filePath);
        if(RC_IS_NOTOK(opStatus)) {
            if(opStatus != RC_FILE_NOT_FOUND) {
                TYPELOGV(NOTIFY_PARSING_FAILURE, "Custom-Init");
                return opStatus;
            } else {
                TYPELOGV(NOTIFY_PARSER_FILE_NOT_FOUND, "Custom-Init", filePath.c_str());
                opStatus = RC_SUCCESS;
            }
        } else {
            TYPELOGV(NOTIFY_PARSING_SUCCESS, "Custom-Init");
        }
    }

    return opStatus;
}

void* serverThreadStart() {
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
