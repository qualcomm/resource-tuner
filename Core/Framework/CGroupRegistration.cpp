// Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
// SPDX-License-Identifier: BSD-3-Clause-Clear

#include <unistd.h>

#include "Utils.h"
#include "Logger.h"
#include "Extensions.h"
#include "TargetRegistry.h"
#include "ResourceRegistry.h"

// This file defines custom the Applier and Tear Callbacks for CGroup Resources

static std::string getCGroupControllerFilePath(Resource* resource, const std::string& cGroupName) {
    ResourceConfigInfo* resourceConfig =
        ResourceRegistry::getInstance()->getResourceById(resource->getResCode());

    std::string controllerFilePath = resourceConfig->mResourcePath;

    // Replace %s in above file path with the actual cgroup name
    char pathBuffer[128];
    std::snprintf(pathBuffer, sizeof(pathBuffer), controllerFilePath.c_str(), cGroupName.c_str());
    controllerFilePath = std::string(pathBuffer);

    return controllerFilePath;
}

static void cGroupDefaultApplyCallback(void* context) {
    if(context == nullptr) return;
    Resource* resource = static_cast<Resource*>(context);

    if(resource->getValuesCount() != 2) return;

    int32_t cGroupIdentifier = (*resource->mResValue.values)[0];
    int32_t valueToBeWritten = (*resource->mResValue.values)[1];

    CGroupConfigInfo* cGroupConfig = TargetRegistry::getInstance()->getCGroupConfig(cGroupIdentifier);

    if(cGroupConfig != nullptr) {
        const std::string cGroupName = cGroupConfig->mCgroupName;

        if(cGroupName.length() > 0) {
            std::string controllerFilePath = getCGroupControllerFilePath(resource, cGroupName);
            std::ofstream controllerFile(controllerFilePath);

            if(!controllerFile.is_open()) {
                TYPELOGV(ERRNO_LOG, "open", strerror(errno));
                return;
            }

            controllerFile<<valueToBeWritten<<std::endl;

            if(controllerFile.fail()) {
                TYPELOGV(ERRNO_LOG, "write", strerror(errno));
            }
            controllerFile.close();
        }
    } else {
        TYPELOGV(VERIFIER_CGROUP_NOT_FOUND, cGroupIdentifier);
    }
}

static void cGroupDefaultResetCallback(void* context) {
    if(context == nullptr) return;
    Resource* resource = static_cast<Resource*>(context);
    ResourceConfigInfo* resourceConfigInfo =
        ResourceRegistry::getInstance()->getResourceById(resource->getResCode());

    if(resourceConfigInfo == nullptr) return;

    if(resource->getValuesCount() != 2) return;
    if(resource->mResValue.values == nullptr) return;

    int32_t cGroupIdentifier = (*resource->mResValue.values)[0];
    CGroupConfigInfo* cGroupConfig = TargetRegistry::getInstance()->getCGroupConfig(cGroupIdentifier);

    if(cGroupConfig != nullptr && cGroupConfig->mDefaultValues != nullptr) {
        const std::string cGroupName = cGroupConfig->mCgroupName;

        if(cGroupName.length() > 0) {
            std::string controllerFilePath = getCGroupControllerFilePath(resource, cGroupName);
            std::ofstream controllerFile(controllerFilePath);

            if(!controllerFile.is_open()) {
                TYPELOGV(ERRNO_LOG, "open", strerror(errno));
                return;
            }
            controllerFile<<(*cGroupConfig->mDefaultValues)[controllerFilePath]<<std::endl;

            if(controllerFile.fail()) {
                TYPELOGV(ERRNO_LOG, "write", strerror(errno));
            }
            controllerFile.close();
        }
    } else {
        TYPELOGV(VERIFIER_CGROUP_NOT_FOUND, cGroupIdentifier);
    }
}

static void setRunOnCores(void* context) {
    if(context == nullptr) return;
    Resource* resource = static_cast<Resource*>(context);

    if(resource->getValuesCount() < 2) return;
    if(resource->mResValue.values == nullptr) return;

    int32_t cGroupIdentifier = (*resource->mResValue.values)[0];
    CGroupConfigInfo* cGroupConfig = TargetRegistry::getInstance()->getCGroupConfig(cGroupIdentifier);

    if(cGroupConfig != nullptr) {
        const std::string cGroupName = cGroupConfig->mCgroupName;

        if(cGroupName.length() > 0) {
            std::string cpusString = "";
            for(int32_t i = 1; i < resource->getValuesCount(); i++) {
                cpusString += std::to_string((*resource->mResValue.values)[i]);
                if(resource->getValuesCount() > 2 && i < resource->getValuesCount() - 1) {
                    cpusString.push_back(',');
                }
            }

            std::string controllerFilePath = getCGroupControllerFilePath(resource, cGroupName);
            std::ofstream controllerFile(controllerFilePath);

            if(!controllerFile.is_open()) {
                TYPELOGV(ERRNO_LOG, "open", strerror(errno));
                return;
            }

            controllerFile<<cpusString<<std::endl;

            if(controllerFile.fail()) {
                TYPELOGV(ERRNO_LOG, "write", strerror(errno));
            }
            controllerFile.close();
        }
    } else {
        TYPELOGV(VERIFIER_CGROUP_NOT_FOUND, cGroupIdentifier);
    }
}

static void setRunOnCoresExclusively(void* context) {
    if(context == nullptr) return;
    Resource* resource = static_cast<Resource*>(context);

    if(resource->getValuesCount() < 2) return;
    if(resource->mResValue.values == nullptr) return;

    int32_t cGroupIdentifier = (*resource->mResValue.values)[0];
    CGroupConfigInfo* cGroupConfig = TargetRegistry::getInstance()->getCGroupConfig(cGroupIdentifier);

    if(cGroupConfig != nullptr) {
        const std::string cGroupName = cGroupConfig->mCgroupName;

        if(cGroupName.length() > 0) {
            const std::string cGroupControllerFilePath =
                ResourceTunerSettings::mBaseCGroupPath + cGroupName + "/cpuset.cpus";

            std::string cpusString = "";
            for(int32_t i = 1; i < resource->getValuesCount(); i++) {
                cpusString += std::to_string((*resource->mResValue.values)[i]);
                if(resource->getValuesCount() > 2 && i < resource->getValuesCount() - 1) {
                    cpusString.push_back(',');
                }
            }

            std::ofstream controllerFile(cGroupControllerFilePath);
            if(!controllerFile.is_open()) {
                TYPELOGV(ERRNO_LOG, "open", strerror(errno));
                return;
            }
            controllerFile<<cpusString<<std::endl;
            controllerFile.close();

            const std::string cGroupCpusetPartitionFilePath =
                ResourceTunerSettings::mBaseCGroupPath + cGroupName + "/cpuset.cpus.partition";

            std::ofstream partitionFile(cGroupCpusetPartitionFilePath);
            if(!partitionFile.is_open()) {
                TYPELOGV(ERRNO_LOG, "open", strerror(errno));
                return;
            }

            partitionFile.close();
        }
    } else {
        TYPELOGV(VERIFIER_CGROUP_NOT_FOUND, cGroupIdentifier);
    }
}

static void limitCpuTime(void* context) {
    if(context == nullptr) return;
    Resource* resource = static_cast<Resource*>(context);

    if(resource->getValuesCount() != 3) return;
    if(resource->mResValue.values == nullptr) return;

    int32_t cGroupIdentifier = (*resource->mResValue.values)[0];
    int32_t maxUsageMicroseconds = (*resource->mResValue.values)[1];
    int32_t periodMicroseconds = (*resource->mResValue.values)[2];
    CGroupConfigInfo* cGroupConfig = TargetRegistry::getInstance()->getCGroupConfig(cGroupIdentifier);

    if(cGroupConfig != nullptr) {
        const std::string cGroupName = cGroupConfig->mCgroupName;

        if(cGroupName.length() > 0) {
            std::string controllerFilePath = getCGroupControllerFilePath(resource, cGroupName);
            std::ofstream controllerFile(controllerFilePath);

            if(!controllerFile.is_open()) {
                TYPELOGV(ERRNO_LOG, "open", strerror(errno));
                return;
            }

            controllerFile<<maxUsageMicroseconds<<" "<<periodMicroseconds<<std::endl;

            if(controllerFile.fail()) {
                TYPELOGV(ERRNO_LOG, "write", strerror(errno));
            }
            controllerFile.close();
        }
    } else {
        TYPELOGV(VERIFIER_CGROUP_NOT_FOUND, cGroupIdentifier);
    }
}

RESTUNE_REGISTER_APPLIER_CB(0x00090000, cGroupDefaultApplyCallback);
RESTUNE_REGISTER_APPLIER_CB(0x00090001, cGroupDefaultApplyCallback);
RESTUNE_REGISTER_APPLIER_CB(0x00090002, setRunOnCores);
RESTUNE_REGISTER_APPLIER_CB(0x00090003, setRunOnCoresExclusively);
RESTUNE_REGISTER_APPLIER_CB(0x00090004, cGroupDefaultApplyCallback);
RESTUNE_REGISTER_APPLIER_CB(0x00090005, limitCpuTime);
RESTUNE_REGISTER_APPLIER_CB(0x00090006, cGroupDefaultApplyCallback);
RESTUNE_REGISTER_APPLIER_CB(0x00090007, cGroupDefaultApplyCallback);
RESTUNE_REGISTER_APPLIER_CB(0x00090008, cGroupDefaultApplyCallback);
RESTUNE_REGISTER_APPLIER_CB(0x00090009, cGroupDefaultApplyCallback);
RESTUNE_REGISTER_APPLIER_CB(0x0009000a, cGroupDefaultApplyCallback);
RESTUNE_REGISTER_APPLIER_CB(0x0009000b, cGroupDefaultApplyCallback);
RESTUNE_REGISTER_APPLIER_CB(0x0009000c, cGroupDefaultApplyCallback);

static void removeProcessFromCGroup(void* context) {
    if(context == nullptr) return;
    Resource* resource = static_cast<Resource*>(context);

    if(resource->getValuesCount() != 2) return;
    if(resource->mResValue.values == nullptr) return;

    int32_t pid = (*resource->mResValue.values)[1];

    std::string parentCGroupProcsPath = "/sys/fs/cgroup/cgroup.procs";
    std::ofstream controllerFile(parentCGroupProcsPath, std::ios::app);

    if(!controllerFile.is_open()) {
        TYPELOGV(ERRNO_LOG, "open", strerror(errno));
        return;
    }

    controllerFile<<pid<<std::endl;

    if(controllerFile.fail()) {
        TYPELOGV(ERRNO_LOG, "write", strerror(errno));
    }
    controllerFile.close();
}

static void removeThreadFromCGroup(void* context) {
    if(context == nullptr) return;
    Resource* resource = static_cast<Resource*>(context);

    if(resource->getValuesCount() != 2) return;
    if(resource->mResValue.values == nullptr) return;

    int32_t tid = (*resource->mResValue.values)[1];

    std::string parentCGroupProcsPath = "/sys/fs/cgroup/cgroup.threads";
    std::ofstream controllerFile(parentCGroupProcsPath, std::ios::app);

    if(!controllerFile.is_open()) {
        TYPELOGV(ERRNO_LOG, "open", strerror(errno));
        return;
    }

    controllerFile<<tid<<std::endl;

    if(controllerFile.fail()) {
        TYPELOGV(ERRNO_LOG, "write", strerror(errno));
    }
    controllerFile.close();
}

static void resetRunOnCoresExclusively(void* context) {
    if(context == nullptr) return;
    Resource* resource = static_cast<Resource*>(context);

    if(resource->getValuesCount() < 2) return;
    if(resource->mResValue.values == nullptr) return;

    int32_t cGroupIdentifier = (*resource->mResValue.values)[0];
    CGroupConfigInfo* cGroupConfig = TargetRegistry::getInstance()->getCGroupConfig(cGroupIdentifier);

    if(cGroupConfig != nullptr && cGroupConfig->mDefaultValues != nullptr) {
        const std::string cGroupName = cGroupConfig->mCgroupName;

        if(cGroupName.length() > 0) {
            const std::string cGroupCpuSetFilePath =
                ResourceTunerSettings::mBaseCGroupPath + cGroupName + "/cpuset.cpus";

            std::ofstream controllerFile(cGroupCpuSetFilePath);
            if(!controllerFile.is_open()) {
                TYPELOGV(ERRNO_LOG, "open", strerror(errno));
                return;
            }

            controllerFile<<(*cGroupConfig->mDefaultValues)[cGroupCpuSetFilePath]<<std::endl;

            if(controllerFile.fail()) {
                TYPELOGV(ERRNO_LOG, "write", strerror(errno));
            }
            controllerFile.close();

            const std::string cGroupCpusetPartitionFilePath =
                ResourceTunerSettings::mBaseCGroupPath + cGroupName + "/cpuset.cpus.partition";

            std::ofstream partitionFile(cGroupCpusetPartitionFilePath);
            if(!partitionFile.is_open()) {
                TYPELOGV(ERRNO_LOG, "open", strerror(errno));
                return;
            }

            partitionFile<<(*cGroupConfig->mDefaultValues)[cGroupCpusetPartitionFilePath];

            if(partitionFile.fail()) {
                TYPELOGV(ERRNO_LOG, "write", strerror(errno));
            }
            partitionFile.close();
        }
    } else {
        TYPELOGV(VERIFIER_CGROUP_NOT_FOUND, cGroupIdentifier);
    }
}

RESTUNE_REGISTER_TEAR_CB(0x00090000, removeProcessFromCGroup);
RESTUNE_REGISTER_TEAR_CB(0x00090001, removeThreadFromCGroup);
RESTUNE_REGISTER_TEAR_CB(0x00090002, cGroupDefaultResetCallback);
RESTUNE_REGISTER_TEAR_CB(0x00090003, resetRunOnCoresExclusively);
RESTUNE_REGISTER_TEAR_CB(0x00090004, cGroupDefaultResetCallback);
RESTUNE_REGISTER_TEAR_CB(0x00090005, cGroupDefaultResetCallback);
RESTUNE_REGISTER_TEAR_CB(0x00090006, cGroupDefaultResetCallback);
RESTUNE_REGISTER_TEAR_CB(0x00090007, cGroupDefaultResetCallback);
RESTUNE_REGISTER_TEAR_CB(0x00090008, cGroupDefaultResetCallback);
RESTUNE_REGISTER_TEAR_CB(0x00090009, cGroupDefaultResetCallback);
RESTUNE_REGISTER_TEAR_CB(0x0009000a, cGroupDefaultResetCallback);
RESTUNE_REGISTER_TEAR_CB(0x0009000b, cGroupDefaultResetCallback);
RESTUNE_REGISTER_TEAR_CB(0x0009000c, cGroupDefaultResetCallback);
