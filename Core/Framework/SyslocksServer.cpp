// Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
// SPDX-License-Identifier: BSD-3-Clause-Clear

#include <csignal>
#include <fcntl.h>

#include "SyslockServerRequests.h"
#include "ResourceProcessor.h"
#include "TargetConfigProcessor.h"
#include "ResourceTunerSettings.h"
#include "Extensions.h"
#include "SysConfigProcessor.h"
#include "ServerUtils.h"
#include "Utils.h"
#include "ErrCodes.h"
#include "Logger.h"
#include "ComponentRegistry.h"

static std::thread serverThread;

ErrCode fetchProperties() {
    // Initialize SysConfigs
    std::shared_ptr<SysConfigProcessor> sysConfigProcessor =
        SysConfigProcessor::getInstance(Extensions::getPropertiesConfigFilePath());
    ErrCode opStatus = sysConfigProcessor->parseSysConfigs();

    if(RC_IS_OK(opStatus)) {
        opStatus = fetchMetaConfigs();
    }

    return opStatus;
}

ErrCode initProvisioner() {
    ErrCode opStatus = RC_SUCCESS;
    preAllocateMemory();

    LOGI("RTN_SERVER_INIT", "Parsing Resource Configs");
    ResourceProcessor resourceProcessor(Extensions::getResourceConfigFilePath());
    opStatus = resourceProcessor.parseResourceConfigs();
    if(RC_IS_NOTOK(opStatus)) {
        LOGE("RTN_SERVER_INIT", "Resource Config Parsing Failed, Server Init Failed");
        return opStatus;
    }
    LOGI("RTN_SERVER_INIT", "Resource Configs Successfully Parsed");

    // Target Configs is optional, i.e. file TargetConfigs.yaml need not be provided.
    // Resource Tuner will dynamically fetch mapping data in such cases
    // Hence, no need to error check for TargetConfigProcessor parsing status.
    TargetConfigProcessor targetConfigProcessor(Extensions::getTargetConfigFilePath());
    targetConfigProcessor.parseTargetConfigs();

    opStatus = TargetRegistry::getInstance()->readPhysicalCoreClusterInfo();
    if(RC_IS_NOTOK(opStatus)) {
        LOGE("RTN_SERVER_INIT", "Reading Physical Core, Cluster Info Failed, Server Init Failed");
        return opStatus;
    }
    LOGI("RTN_SERVER_INIT", "Logical to Physical Core / Cluster mapping successfully created");

    // By this point, all the Extension Appliers / Resources should be registered.
    ResourceRegistry::getInstance()->pluginModifications(Extensions::getModifiedResources());

    // Create one thread:
    // - Processor Server thread
    serverThread = std::thread(TunerServerThread);

    return opStatus;
}

ErrCode terminateProvisioner() {
    // Terminate Provisioner
    // Check if the thread is joinable, to prevent undefined behaviour
    if(serverThread.joinable()) {
        RequestQueue::getInstance()->forcefulAwake();
        serverThread.join();
    } else {
        LOGE("RTN_SERVER_TERMINATION", "Provisioner thread is not joinable");
    }
    return RC_SUCCESS;
}

RTN_REGISTER_MODULE(MOD_PROVISIONER,
                     initProvisioner,
                     terminateProvisioner,
                     submitResourceProvisioningRequest);
