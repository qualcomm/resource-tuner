// Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
// SPDX-License-Identifier: BSD-3-Clause-Clear

#include "Extensions.h"
#include "TargetRegistry.h"
#include "ResourceRegistry.h"

void addProcessToCgroup(void* context) {
    CGroupApplyInfo* info = static_cast<CGroupApplyInfo*>(context);
    Resource* resource = info->mResource;
    int32_t pid = info->mClientPID;
    int8_t cGroupIdentifier = static_cast<int8_t>(resource->getOptionalInfo());
    CGroupConfigInfo* cGroupConfig = TargetRegistry::getInstance()->getCGroupConfig(cGroupIdentifier);

    if(cGroupConfig != nullptr) {
        const std::string cGroupPath = cGroupConfig->mCgroupName;
        if(cGroupPath.length() > 0) {
            std::ofstream procs_file("/sys/fs/cgroup/" + cGroupPath + "/cgroup.procs");
            if(!procs_file.is_open()) {
                return;
            }
            procs_file << pid;
            procs_file.close();
        }
    }
}

void addThreadToCgroup(void* context) {
    CGroupApplyInfo* info = static_cast<CGroupApplyInfo*>(context);
    Resource* resource = info->mResource;
    int32_t tid = info->mClientTID;
    int8_t cGroupIdentifier = static_cast<int8_t>(resource->getOptionalInfo());
    CGroupConfigInfo* cGroupConfig = TargetRegistry::getInstance()->getCGroupConfig(cGroupIdentifier);

    if(cGroupConfig != nullptr) {
        const std::string cGroupPath = cGroupConfig->mCgroupName;
        if(cGroupPath.length() > 0) {
            std::ofstream procs_file("/sys/fs/cgroup/" + cGroupPath  + "/cgroup.threads");
            if(!procs_file.is_open()) {
                return;
            }
            procs_file << tid;
            procs_file.close();
        }
    }
}

void setRunOnCores(void* context) {
    CGroupApplyInfo* info = static_cast<CGroupApplyInfo*>(context);
}

void setRunOnCoresExclusively(void* context) {
    CGroupApplyInfo* info = static_cast<CGroupApplyInfo*>(context);
}

void freezeCgroup(void* context) {
    CGroupApplyInfo* info = static_cast<CGroupApplyInfo*>(context);
}

void unFreezeCGroup(void* context) {
    CGroupApplyInfo* info = static_cast<CGroupApplyInfo*>(context);
}

RTN_REGISTER_RESOURCE(0x00090000, addProcessToCgroup);
RTN_REGISTER_RESOURCE(0x00090001, addThreadToCgroup);
RTN_REGISTER_RESOURCE(0x00090002, setRunOnCores);
RTN_REGISTER_RESOURCE(0x00090003, setRunOnCoresExclusively);
RTN_REGISTER_RESOURCE(0x00090004, freezeCgroup);
RTN_REGISTER_RESOURCE(0x00090005, unFreezeCGroup);
