// Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
// SPDX-License-Identifier: BSD-3-Clause-Clear

#include <csignal>
#include <fcntl.h>

#include "SyslockServerRequests.h"
#include "ResourceProcessor.h"
#include "TargetConfigProcessor.h"
#include "SystuneSettings.h"
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
    std::shared_ptr<SysConfigProcessor> sysConfigProcessor = SysConfigProcessor::getInstance();
    ErrCode opStatus = sysConfigProcessor->parseSysConfigs();

    if(RC_IS_NOTOK(opStatus)) {
        return opStatus;
    }

    fetchMetaConfigs();
    return RC_SUCCESS;
}

ErrCode initProvisioner() {
    ErrCode opStatus = RC_SUCCESS;
    preAllocateMemory();

    // Read Resource Configs
    ResourceProcessor resourceProcessor(Extensions::getResourceConfigFilePath());

    opStatus = resourceProcessor.parseResourceConfigs();
    if(RC_IS_NOTOK(opStatus)) {
        LOGE("URM_PROVISIONER_SERVER", "Resource Config Parsing Failed, Server Init Failed");
        return opStatus;
    }

    TargetConfigProcessor targetConfigProcessor(Extensions::getTargetConfigFilePath());

    opStatus = targetConfigProcessor.parseTargetConfigs();
    if(RC_IS_NOTOK(opStatus)) {
        LOGE("URM_PROVISIONER_SERVER", "Target Config Parsing Failed, Server Init Failed");
        return opStatus;
    }

    opStatus = TargetRegistry::getInstance()->readPhysicalCoreClusterInfo();
    if(RC_IS_NOTOK(opStatus)) {
        LOGE("URM_PROVISIONER_SERVER", "Reading Physical Core, Cluster Info Failed, Server Init Failed");
        return opStatus;
    }

    // By this point, all the Extension Appliers / Resources should be registered.
    ResourceRegistry::getInstance()->pluginModifications(Extensions::getModifiedResources());

    // Create one thread:
    // - Processor Server thread
    serverThread = std::thread(SyslocksdServerThread);

    return opStatus;
}

ErrCode terminateProvisioner() {
    // Terminate Provisioner
    // Check if the thread is joinable, to prevent undefined behaviour
    if(serverThread.joinable()) {
        RequestQueue::getInstance()->forcefulAwake();
        serverThread.join();
    } else {
        LOGE("URM_PROVISIONER_SERVER", "Provisioner thread is not joinable");
    }
    return RC_SUCCESS;
}

URM_REGISTER_MODULE(MOD_PROVISIONER,
                    initProvisioner,
                    terminateProvisioner,
                    submitResourceProvisioningRequest);
