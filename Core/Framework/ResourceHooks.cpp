// Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
// SPDX-License-Identifier: BSD-3-Clause-Clear

#include <unistd.h>

#include "Utils.h"
#include "Logger.h"
#include "Extensions.h"
#include "TargetRegistry.h"
#include "ResourceRegistry.h"

static std::string getClusterTypeResourceNodePath(Resource* resource, int32_t clusterID) {
    ResourceConfigInfo* resourceConfig =
        ResourceRegistry::getInstance()->getResourceById(resource->getResCode());

    if(resourceConfig == nullptr) return "";
    std::string filePath = resourceConfig->mResourcePath;

    // Replace %d in above file path with the actual cluster id
    char pathBuffer[128];
    std::snprintf(pathBuffer, sizeof(pathBuffer), filePath.c_str(), clusterID);
    filePath = std::string(pathBuffer);

    return filePath;
}

static std::string getCoreTypeResourceNodePath(Resource* resource, int32_t coreID) {
    ResourceConfigInfo* resourceConfig =
        ResourceRegistry::getInstance()->getResourceById(resource->getResCode());

    if(resourceConfig == nullptr) return "";
    std::string filePath = resourceConfig->mResourcePath;

    // Replace %d in above file path with the actual core id
    char pathBuffer[128];
    std::snprintf(pathBuffer, sizeof(pathBuffer), filePath.c_str(), coreID);
    filePath = std::string(pathBuffer);

    return filePath;
}

static std::string getCGroupTypeResourceNodePath(Resource* resource, const std::string& cGroupName) {
    ResourceConfigInfo* resourceConfig =
        ResourceRegistry::getInstance()->getResourceById(resource->getResCode());

    if(resourceConfig == nullptr) return "";
    std::string filePath = resourceConfig->mResourcePath;

    // Replace %s in above file path with the actual cgroup name
    char pathBuffer[128];
    std::snprintf(pathBuffer, sizeof(pathBuffer), filePath.c_str(), cGroupName.c_str());
    filePath = std::string(pathBuffer);

    return filePath;
}

static void truncateFile(const std::string& filePath) {
    std::ofstream ofStream(filePath, std::ofstream::out | std::ofstream::trunc);
    ofStream<<""<<std::endl;
    ofStream.close();
}

void defaultClusterLevelApplierCb(void* context) {
    if(context == nullptr) return;
    Resource* resource = static_cast<Resource*>(context);

    // Get the Cluster ID
    int32_t clusterID = resource->getClusterValue();
    std::string resourceNodePath = getClusterTypeResourceNodePath(resource, clusterID);
    std::ofstream controllerFile(resourceNodePath);

    if(!controllerFile.is_open()) {
        TYPELOGV(ERRNO_LOG, "open", strerror(errno));
        return;
    }

    controllerFile<<resource->mResValue.value<<std::endl;

    if(controllerFile.fail()) {
        TYPELOGV(ERRNO_LOG, "write", strerror(errno));
    }
    controllerFile.close();
}

void defaultClusterLevelTearCb(void* context) {
    if(context == nullptr) return;
    Resource* resource = static_cast<Resource*>(context);

    // Get the Cluster ID
    int32_t clusterID = resource->getClusterValue();
    std::string resourceNodePath = getClusterTypeResourceNodePath(resource, clusterID);
    std::ofstream controllerFile(resourceNodePath);

    if(!controllerFile.is_open()) {
        TYPELOGV(ERRNO_LOG, "open", strerror(errno));
        return;
    }

    std::string defaultValue =
        ResourceRegistry::getInstance()->getDefaultValue(resourceNodePath);

    controllerFile<<defaultValue<<std::endl;

    if(controllerFile.fail()) {
        TYPELOGV(ERRNO_LOG, "write", strerror(errno));
    }
    controllerFile.close();
}

void defaultCoreLevelApplierCb(void* context) {
    if(context == nullptr) return;
    Resource* resource = static_cast<Resource*>(context);

    // Get the Core ID
    int32_t coreID = resource->getCoreValue();
    std::string resourceNodePath = getCoreTypeResourceNodePath(resource, coreID);
    std::ofstream controllerFile(resourceNodePath);

    if(!controllerFile.is_open()) {
        TYPELOGV(ERRNO_LOG, "open", strerror(errno));
        return;
    }

    controllerFile<<resource->mResValue.value<<std::endl;

    if(controllerFile.fail()) {
        TYPELOGV(ERRNO_LOG, "write", strerror(errno));
    }
    controllerFile.close();
}

void defaultCoreLevelTearCb(void* context) {
    if(context == nullptr) return;
    Resource* resource = static_cast<Resource*>(context);

    // Get the Core ID
    int32_t coreID = resource->getCoreValue();
    std::string resourceNodePath = getClusterTypeResourceNodePath(resource, coreID);
    std::ofstream controllerFile(resourceNodePath);

    if(!controllerFile.is_open()) {
        TYPELOGV(ERRNO_LOG, "open", strerror(errno));
        return;
    }

    std::string defaultValue =
        ResourceRegistry::getInstance()->getDefaultValue(resourceNodePath);

    controllerFile<<defaultValue<<std::endl;

    if(controllerFile.fail()) {
        TYPELOGV(ERRNO_LOG, "write", strerror(errno));
    }
    controllerFile.close();
}

void defaultCGroupLevelApplierCb(void* context) {
    if(context == nullptr) return;
    Resource* resource = static_cast<Resource*>(context);

    if(resource->getValuesCount() != 2) return;

    int32_t cGroupIdentifier = (*resource->mResValue.values)[0];
    int32_t valueToBeWritten = (*resource->mResValue.values)[1];

    // Get the corresponding cGroupConfig, this is needed to identify the
    // correct CGroup Name.
    CGroupConfigInfo* cGroupConfig =
        TargetRegistry::getInstance()->getCGroupConfig(cGroupIdentifier);

    if(cGroupConfig != nullptr) {
        const std::string cGroupName = cGroupConfig->mCgroupName;

        if(cGroupName.length() > 0) {
            std::string controllerFilePath = getCGroupTypeResourceNodePath(resource, cGroupName);
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

void defaultCGroupLevelTearCb(void* context) {
    if(context == nullptr) return;
    Resource* resource = static_cast<Resource*>(context);
    ResourceConfigInfo* resourceConfigInfo =
        ResourceRegistry::getInstance()->getResourceById(resource->getResCode());

    if(resourceConfigInfo == nullptr) return;

    if(resource->mResValue.values == nullptr) return;

    int32_t cGroupIdentifier = (*resource->mResValue.values)[0];
    CGroupConfigInfo* cGroupConfig =
        TargetRegistry::getInstance()->getCGroupConfig(cGroupIdentifier);

    if(cGroupConfig == nullptr) {
        TYPELOGV(VERIFIER_CGROUP_NOT_FOUND, cGroupIdentifier);
        return;
    }

    const std::string cGroupName = cGroupConfig->mCgroupName;

    if(cGroupName.length() > 0) {
        std::string controllerFilePath = getCGroupTypeResourceNodePath(resource, cGroupName);
        std::string defaultValue =
            ResourceRegistry::getInstance()->getDefaultValue(controllerFilePath);

        if(defaultValue.length() == 0) {
            truncateFile(controllerFilePath);
        } else {
            std::ofstream controllerFile(controllerFilePath);

            if(!controllerFile.is_open()) {
                TYPELOGV(ERRNO_LOG, "open", strerror(errno));
                return;
            }

            controllerFile<<defaultValue<<std::endl;

            if(controllerFile.fail()) {
                TYPELOGV(ERRNO_LOG, "write", strerror(errno));
            }
            controllerFile.close();
        }
    }
}

void defaultGlobalLevelApplierCb(void* context) {
    if(context == nullptr) return;
    Resource* resource = static_cast<Resource*>(context);

    ResourceConfigInfo* resourceConfig =
        ResourceRegistry::getInstance()->getResourceById(resource->getResCode());

    if(resourceConfig != nullptr) {
        AuxRoutines::writeToFile(resourceConfig->mResourcePath, std::to_string(resource->mResValue.value));
    }
}

void defaultGlobalLevelTearCb(void* context) {
    if(context == nullptr) return;
    Resource* resource = static_cast<Resource*>(context);

    ResourceConfigInfo* resourceConfig =
        ResourceRegistry::getInstance()->getResourceById(resource->getResCode());

    if(resourceConfig != nullptr) {
        std::string defaultValue =
            ResourceRegistry::getInstance()->getDefaultValue(resourceConfig->mResourcePath);

        AuxRoutines::writeToFile(resourceConfig->mResourcePath, defaultValue);
    }
}

// Special Callbacks for some Resources
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

            std::string controllerFilePath = getCGroupTypeResourceNodePath(resource, cGroupName);
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

            partitionFile<<"isolated"<<std::endl;
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
            std::string controllerFilePath = getCGroupTypeResourceNodePath(resource, cGroupName);
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

    if(cGroupConfig != nullptr) {
        const std::string cGroupName = cGroupConfig->mCgroupName;

        if(cGroupName.length() > 0) {
            const std::string cGroupCpuSetFilePath =
                ResourceTunerSettings::mBaseCGroupPath + cGroupName + "/cpuset.cpus";

            std::ofstream controllerFile(cGroupCpuSetFilePath);
            if(!controllerFile.is_open()) {
                TYPELOGV(ERRNO_LOG, "open", strerror(errno));
                return;
            }

            std::string defaultValue =
                ResourceRegistry::getInstance()->getDefaultValue(cGroupCpuSetFilePath);

            controllerFile<<defaultValue<<std::endl;

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

            defaultValue = ResourceRegistry::getInstance()->getDefaultValue(cGroupCpusetPartitionFilePath);

            partitionFile<<defaultValue<<std::endl;

            if(partitionFile.fail()) {
                TYPELOGV(ERRNO_LOG, "write", strerror(errno));
            }
            partitionFile.close();
        }
    } else {
        TYPELOGV(VERIFIER_CGROUP_NOT_FOUND, cGroupIdentifier);
    }
}

RESTUNE_REGISTER_APPLIER_CB(0x00090002, setRunOnCores);
RESTUNE_REGISTER_APPLIER_CB(0x00090003, setRunOnCoresExclusively);
RESTUNE_REGISTER_APPLIER_CB(0x00090005, limitCpuTime);
RESTUNE_REGISTER_TEAR_CB(0x00090000, removeProcessFromCGroup);
RESTUNE_REGISTER_TEAR_CB(0x00090001, removeThreadFromCGroup);
RESTUNE_REGISTER_TEAR_CB(0x00090003, resetRunOnCoresExclusively);
