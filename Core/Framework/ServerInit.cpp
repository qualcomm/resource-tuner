// Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
// SPDX-License-Identifier: BSD-3-Clause-Clear

#include <csignal>
#include <fcntl.h>

#include "ServerRequests.h"
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

static void writeToCgroupFile(const std::string& propName, const std::string& value) {
    std::ofstream cGroupFile(propName);
    if(!cGroupFile) {
        cGroupFile.close();
        return;
    }

    cGroupFile << value;
    cGroupFile.close();
}

static ErrCode createCGroups() {
    std::vector<CGroupConfigInfo*> cGroupConfigs;
    TargetRegistry::getInstance()->getCGroupConfigs(cGroupConfigs);

    for(CGroupConfigInfo* cGroupConfig : cGroupConfigs) {
        const std::string cGroupPath = "/sys/fs/cgroup/" + cGroupConfig->mCgroupName;
        if(mkdir(cGroupPath.c_str(), 0755) == 0) {
            if(cGroupConfig->isThreaded) {
                writeToCgroupFile(cGroupPath + "/cgroup.type", "threaded");
            } else {
                // Enable cpu, cpuset and memory Controllers for the cgroup
                writeToCgroupFile(cGroupPath + "/cgroup.subtree_control", "+cpu +cpuset +memory");

                // Create a child cgroup to hold the PIDs
                const std::string childCGroupPath =
                    "/sys/fs/cgroup/" + cGroupConfig->mCgroupName + "/tasks";
                mkdir(childCGroupPath.c_str(), 0755);
            }
        } else {
            TYPELOGV(ERRNO_LOG, "mkdir", strerror(errno));
        }
    }
    return RC_SUCCESS;
}

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

static ErrCode initServer() {
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
    TargetConfigProcessor targetConfigProcessor(Extensions::getTargetConfigFilePath(), "");
    targetConfigProcessor.parseTargetConfigs();

    opStatus = TargetRegistry::getInstance()->readPhysicalCoreClusterInfo();
    if(RC_IS_NOTOK(opStatus)) {
        LOGE("RTN_SERVER_INIT", "Reading Physical Core, Cluster Info Failed, Server Init Failed");
        return opStatus;
    }
    LOGI("RTN_SERVER_INIT", "Logical to Physical Core / Cluster mapping successfully created");

    opStatus = createCGroups();

    // By this point, all the Extension Appliers / Resources should be registered.
    ResourceRegistry::getInstance()->pluginModifications(Extensions::getModifiedResources());

    // Create one thread:
    // - Processor Server thread
    serverThread = std::thread(TunerServerThread);

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
        LOGE("RTN_SERVER_TERMINATION", "Server thread is not joinable");
    }
    return RC_SUCCESS;
}

RTN_REGISTER_MODULE(MOD_CORE,
                    initServer,
                    terminateServer,
                    submitResourceProvisioningRequest);
