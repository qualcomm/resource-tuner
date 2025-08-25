// Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
// SPDX-License-Identifier: BSD-3-Clause-Clear

#include <csignal>
#include <fcntl.h>

#include "ServerRequests.h"
#include "ConfigProcessor.h"
#include "ResourceTunerSettings.h"
#include "Extensions.h"
#include "SysConfigProcessor.h"
#include "ServerUtils.h"
#include "Utils.h"
#include "ErrCodes.h"
#include "Logger.h"
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

ErrCode fetchProperties() {
    // Initialize SysConfigs
    SysConfigProcessor sysConfigProcessor;
    ErrCode opStatus = sysConfigProcessor.parseSysConfigs(ResourceTunerSettings::mPropertiesFilePath);

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

    filePath = Extensions::getResourceConfigFilePath();
    if(filePath.length() > 0) {
        TYPELOGV(NOTIFY_CUSTOM_CONFIG_FILE, "Resource", filePath.c_str());
        TYPELOGV(NOTIFY_PARSING_START, "Custom-Resource");
        opStatus = configProcessor.parseResourceConfigs(filePath, true);
        if(RC_IS_NOTOK(opStatus)) {
            TYPELOGV(NOTIFY_PARSING_FAILURE, "Custom-Resource");
            return opStatus;
        }
    } else {
        TYPELOGV(NOTIFY_PARSING_START, "Custom-Resource");
        filePath = ResourceTunerSettings::mCustomResourceFilePath;
        opStatus = configProcessor.parseResourceConfigs(filePath, true);
        if(RC_IS_NOTOK(opStatus)) {
            if(opStatus == RC_FILE_NOT_FOUND) {
                TYPELOGV(NOTIFY_PARSER_FILE_NOT_FOUND, "Custom-Resource", filePath.c_str());
                TYPELOGV(NOTIFY_PARSING_SUCCESS, "Resource");
                return RC_SUCCESS;
            }
            TYPELOGV(NOTIFY_PARSING_FAILURE, "Resource");
            return opStatus;
        }
    }

    TYPELOGV(NOTIFY_PARSING_SUCCESS, "Resource");
    return opStatus;
}

static ErrCode fetchInitInfo() {
    ErrCode opStatus = RC_SUCCESS;

    // Target Configs is optional, i.e. file TargetConfig.yaml need not be provided.
    // Resource Tuner will dynamically fetch mapping data in such cases
    ConfigProcessor configProcessor;
    std::string filePath = Extensions::getTargetConfigFilePath();
    if(filePath.length() > 0) {
        // Custom Target Config file has been provided by BU
        TYPELOGV(NOTIFY_CUSTOM_CONFIG_FILE, "Target", filePath.c_str());
        opStatus = configProcessor.parseTargetConfigs(filePath);
        if(RC_IS_NOTOK(opStatus)) {
            TYPELOGV(NOTIFY_PARSING_FAILURE, "Target");
            return opStatus;
        } else {
            TYPELOGV(NOTIFY_PARSING_SUCCESS, "Target");
        }
    } else {
        TYPELOGV(NOTIFY_PARSING_START, "Target");
        filePath = ResourceTunerSettings::mCustomTargetFilePath;
        opStatus = configProcessor.parseTargetConfigs(filePath);
        if(RC_IS_NOTOK(opStatus)) {
            if(opStatus != RC_FILE_NOT_FOUND) {
                TYPELOGV(NOTIFY_PARSING_FAILURE, "Target");
                return opStatus;
            } else {
                TYPELOGV(NOTIFY_PARSER_FILE_NOT_FOUND, "Target", filePath.c_str());
            }
        }
        TYPELOGV(NOTIFY_PARSING_SUCCESS, "Target");
    }

    TYPELOGV(NOTIFY_PARSING_START, "Init");

    const std::string initConfigFilePath = ResourceTunerSettings::mInitConfigFilePath;
    opStatus = configProcessor.parseInitConfigs(initConfigFilePath);
    if(RC_IS_NOTOK(opStatus)) {
        TYPELOGV(NOTIFY_PARSING_FAILURE, "Init");
        return opStatus;
    }

    TYPELOGV(NOTIFY_PARSING_SUCCESS, "Init");
    return opStatus;
}

static ErrCode initServer() {
    ErrCode opStatus = RC_SUCCESS;

    // Pre-Allocate Memory for Commonly used Types via Memory Pool
    preAllocateMemory();

    // Fetch and Parse Resource Configs
    // Resource Parsing which will be considered:
    // - Common Resource Configs
    // - Target Specific Resource Configs
    // - Custom Resource Configs (if present)
    opStatus = fetchResources();
    if(RC_IS_NOTOK(opStatus)) {
        return opStatus;
    }

    // Fetch and Parse:
    // - Custom Target Configs (if present)
    // - Init Configs
    opStatus = fetchInitInfo();
    if(RC_IS_NOTOK(opStatus)) {
        return opStatus;
    }

    // Perform Logical To Physical (Core / Cluster) Mapping
    opStatus = TargetRegistry::getInstance()->readPhysicalCoreClusterInfo();
    if(RC_IS_NOTOK(opStatus)) {
        TYPELOGD(LOGICAL_TO_PHYSICAL_MAPPING_GEN_FAILURE);
        return opStatus;
    }
    TYPELOGD(LOGICAL_TO_PHYSICAL_MAPPING_GEN_SUCCESS);

    TargetRegistry::getInstance()->displayLogicalToPhysicalMapping();

    // By this point, all the Extension Appliers / Resources would have been registered.
    ResourceRegistry::getInstance()->pluginModifications();

    // Create the Processor thread:
    try {
        serverThread = std::thread(TunerServerThread);
    } catch(const std::system_error& e) {
        TYPELOGV(SYSTEM_THREAD_CREATION_FAILURE, "Server", e.what());
        opStatus = RC_MODULE_INIT_FAILURE;
    }

    // Wait for the thread to initialize
    std::this_thread::sleep_for(std::chrono::milliseconds(300));

    return opStatus;
}

static ErrCode terminateServer() {
    // Check if the thread is joinable, to prevent undefined behaviour
    if(serverThread.joinable()) {
        RequestQueue::getInstance()->forcefulAwake();
        serverThread.join();
    } else {
        TYPELOGV(SYSTEM_THREAD_NOT_JOINABLE, "Server");
    }
    return RC_SUCCESS;
}

RESTUNE_REGISTER_MODULE(MOD_CORE,
                        initServer,
                        terminateServer,
                        submitResourceProvisioningRequest);
