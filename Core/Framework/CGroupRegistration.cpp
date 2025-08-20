// Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
// SPDX-License-Identifier: BSD-3-Clause-Clear

#include <unistd.h>

#include "Logger.h"
#include "Extensions.h"
#include "TargetRegistry.h"
#include "ResourceRegistry.h"

// This file defines custom the Applier and Tear Callbacks for CGroup Resources

static std::string getCGroupControllerFilePath(Resource* resource, const std::string& cGroupName) {
    ResourceConfigInfo* resourceConfig =
        ResourceRegistry::getInstance()->getResourceById(resource->getOpCode());

    std::string controllerFilePath = resourceConfig->mResourcePath;

    // Replace %s in above file path with the actual cgroup name
    char pathBuffer[128];
    std::snprintf(pathBuffer, sizeof(pathBuffer), controllerFilePath.c_str(), cGroupName.c_str());
    controllerFilePath = std::string(pathBuffer);

    return controllerFilePath;
}

static void addProcessToCgroup(void* context) {
    if(context == nullptr) return;
    Resource* resource = static_cast<Resource*>(context);

    if(resource->getValuesCount() != 2) return;
    if(resource->mConfigValue.valueArray == nullptr) return;

    int32_t cGroupIdentifier = (*resource->mConfigValue.valueArray)[0];
    int32_t pid = (*resource->mConfigValue.valueArray)[1];
    CGroupConfigInfo* cGroupConfig = TargetRegistry::getInstance()->getCGroupConfig(cGroupIdentifier);

    if(cGroupConfig != nullptr) {
        const std::string cGroupName = cGroupConfig->mCgroupName;

        if(cGroupName.length() > 0) {
            std::string controllerFilePath = getCGroupControllerFilePath(resource, cGroupName);

            std::ofstream controllerFile(controllerFilePath, std::ios::app);
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
    } else {
        TYPELOGV(VERIFIER_CGROUP_NOT_FOUND, cGroupIdentifier);
    }
}

static void addThreadToCgroup(void* context) {
    if(context == nullptr) return;
    Resource* resource = static_cast<Resource*>(context);

    if(resource->getValuesCount() != 2) return;
    if(resource->mConfigValue.valueArray == nullptr) return;

    int32_t cGroupIdentifier = (*resource->mConfigValue.valueArray)[0];
    int32_t tid = (*resource->mConfigValue.valueArray)[1];
    CGroupConfigInfo* cGroupConfig = TargetRegistry::getInstance()->getCGroupConfig(cGroupIdentifier);

    if(cGroupConfig != nullptr) {
        const std::string cGroupName = cGroupConfig->mCgroupName;

        if(cGroupName.length() > 0) {
            std::string controllerFilePath = getCGroupControllerFilePath(resource, cGroupName);
            std::ofstream controllerFile(controllerFilePath, std::ios::app);

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
    } else {
        TYPELOGV(VERIFIER_CGROUP_NOT_FOUND, cGroupIdentifier);
    }
}

static void setRunOnCores(void* context) {
    if(context == nullptr) return;
    Resource* resource = static_cast<Resource*>(context);

    if(resource->getValuesCount() < 2) return;
    if(resource->mConfigValue.valueArray == nullptr) return;

    int32_t cGroupIdentifier = (*resource->mConfigValue.valueArray)[0];
    CGroupConfigInfo* cGroupConfig = TargetRegistry::getInstance()->getCGroupConfig(cGroupIdentifier);

    if(cGroupConfig != nullptr) {
        const std::string cGroupName = cGroupConfig->mCgroupName;

        if(cGroupName.length() > 0) {
            std::string cpusString = "";
            for(int32_t i = 1; i < resource->getValuesCount(); i++) {
                cpusString += std::to_string((*resource->mConfigValue.valueArray)[i]);
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

            controllerFile<<cpusString;

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
    if(resource->mConfigValue.valueArray == nullptr) return;

    int32_t cGroupIdentifier = (*resource->mConfigValue.valueArray)[0];
    CGroupConfigInfo* cGroupConfig = TargetRegistry::getInstance()->getCGroupConfig(cGroupIdentifier);

    if(cGroupConfig != nullptr) {
        const std::string cGroupName = cGroupConfig->mCgroupName;

        if(cGroupName.length() > 0) {
            const std::string cGroupControllerFilePath =
                ResourceTunerSettings::mBaseCGroupPath + cGroupName + "/cpuset.cpus";

            std::string cpusString = "";
            for(int32_t i = 1; i < resource->getValuesCount(); i++) {
                cpusString += std::to_string((*resource->mConfigValue.valueArray)[i]);
                if(resource->getValuesCount() > 2 && i < resource->getValuesCount() - 1) {
                    cpusString.push_back(',');
                }
            }

            std::ofstream controllerFile(cGroupControllerFilePath);
            if(!controllerFile.is_open()) {
                TYPELOGV(ERRNO_LOG, "open", strerror(errno));
                return;
            }
            controllerFile<<cpusString;
            controllerFile.close();

            const std::string cGroupCpusetPartitionFilePath =
                ResourceTunerSettings::mBaseCGroupPath + cGroupName + "/cpuset.cpus.partition";

            std::ofstream partitionFile(cGroupCpusetPartitionFilePath);
            if(!partitionFile.is_open()) {
                TYPELOGV(ERRNO_LOG, "open", strerror(errno));
                return;
            }

            partitionFile<<"root";
            partitionFile<<"isolated";

            partitionFile.close();
        }
    } else {
        TYPELOGV(VERIFIER_CGROUP_NOT_FOUND, cGroupIdentifier);
    }
}

static void freezeCgroup(void* context) {
    if(context == nullptr) return;
    Resource* resource = static_cast<Resource*>(context);

    if(resource->getValuesCount() != 2) return;

    int32_t cGroupIdentifier = (*resource->mConfigValue.valueArray)[0];
    int32_t freezeStatus = (*resource->mConfigValue.valueArray)[1];

    if(freezeStatus != 0 && freezeStatus != 1) return;
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

            controllerFile<<freezeStatus;

            if(controllerFile.fail()) {
                TYPELOGV(ERRNO_LOG, "write", strerror(errno));
            }
            controllerFile.close();
        }
    } else {
        TYPELOGV(VERIFIER_CGROUP_NOT_FOUND, cGroupIdentifier);
    }
}

static void setCpuIdle(void* context) {
    if(context == nullptr) return;
    Resource* resource = static_cast<Resource*>(context);

    if(resource->getValuesCount() != 2) return;

    int32_t cGroupIdentifier = (*resource->mConfigValue.valueArray)[0];
    int32_t idleStatus = (*resource->mConfigValue.valueArray)[1];

    if(idleStatus != 0 && idleStatus != 1) return;
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

            controllerFile<<idleStatus;

            if(controllerFile.fail()) {
                TYPELOGV(ERRNO_LOG, "write", strerror(errno));
            }
            controllerFile.close();
        }
    } else {
        TYPELOGV(VERIFIER_CGROUP_NOT_FOUND, cGroupIdentifier);
    }
}

static void setUClampMin(void* context) {
    if(context == nullptr) return;
    Resource* resource = static_cast<Resource*>(context);

    if(resource->getValuesCount() != 2) return;
    if(resource->mConfigValue.valueArray == nullptr) return;

    int32_t cGroupIdentifier = (*resource->mConfigValue.valueArray)[0];
    int32_t clampVal = (*resource->mConfigValue.valueArray)[1];
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
            controllerFile << clampVal;

            if(controllerFile.fail()) {
                TYPELOGV(ERRNO_LOG, "write", strerror(errno));
            }
            controllerFile.close();
        }
    } else {
        TYPELOGV(VERIFIER_CGROUP_NOT_FOUND, cGroupIdentifier);
    }
}

static void setUClampMax(void* context) {
    if(context == nullptr) return;
    Resource* resource = static_cast<Resource*>(context);

    if(resource->getValuesCount() != 2) return;
    if(resource->mConfigValue.valueArray == nullptr) return;

    int32_t cGroupIdentifier = (*resource->mConfigValue.valueArray)[0];
    int32_t clampVal = (*resource->mConfigValue.valueArray)[1];
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

            controllerFile<<clampVal;

            if(controllerFile.fail()) {
                TYPELOGV(ERRNO_LOG, "write", strerror(errno));
            }
            controllerFile.close();
        }
    } else {
        TYPELOGV(VERIFIER_CGROUP_NOT_FOUND, cGroupIdentifier);
    }
}

static void setRelativeCPUShare(void* context) {
    if(context == nullptr) return;
    Resource* resource = static_cast<Resource*>(context);

    if(resource->getValuesCount() != 2) return;
    if(resource->mConfigValue.valueArray == nullptr) return;

    int32_t cGroupIdentifier = (*resource->mConfigValue.valueArray)[0];
    int32_t relativeWeight = (*resource->mConfigValue.valueArray)[1];
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

            controllerFile<<relativeWeight;
            if(controllerFile.fail()) {
                TYPELOGV(ERRNO_LOG, "write", strerror(errno));
            }
            controllerFile.close();
        }
    } else {
        TYPELOGV(VERIFIER_CGROUP_NOT_FOUND, cGroupIdentifier);
    }
}

static void setMaxMemoryLimit(void* context) {
    if(context == nullptr) return;
    Resource* resource = static_cast<Resource*>(context);

    if(resource->getValuesCount() != 2) return;
    if(resource->mConfigValue.valueArray == nullptr) return;

    int32_t cGroupIdentifier = (*resource->mConfigValue.valueArray)[0];
    int64_t maxMemoryLimit = (*resource->mConfigValue.valueArray)[1];
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

            controllerFile<<maxMemoryLimit;

            if(controllerFile.fail()) {
                TYPELOGV(ERRNO_LOG, "write", strerror(errno));
            }
            controllerFile.close();
        }
    } else {
        TYPELOGV(VERIFIER_CGROUP_NOT_FOUND, cGroupIdentifier);
    }
}

static void setMinMemoryFloor(void* context) {
    if(context == nullptr) return;
    Resource* resource = static_cast<Resource*>(context);

    if(resource->getValuesCount() != 2) return;
    if(resource->mConfigValue.valueArray == nullptr) return;

    int32_t cGroupIdentifier = (*resource->mConfigValue.valueArray)[0];
    int64_t minMemoryFloor = (*resource->mConfigValue.valueArray)[1];
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

            controllerFile<<minMemoryFloor;

            if(controllerFile.fail()) {
                TYPELOGV(ERRNO_LOG, "write", strerror(errno));
            }
            controllerFile.close();
        }
    } else {
        TYPELOGV(VERIFIER_CGROUP_NOT_FOUND, cGroupIdentifier);
    }
}

static void limitCpuTime(void* context) {
    if(context == nullptr) return;
    Resource* resource = static_cast<Resource*>(context);

    if(resource->getValuesCount() != 3) return;
    if(resource->mConfigValue.valueArray == nullptr) return;

    int32_t cGroupIdentifier = (*resource->mConfigValue.valueArray)[0];
    int32_t maxUsageMicroseconds = (*resource->mConfigValue.valueArray)[1];
    int32_t periodMicroseconds = (*resource->mConfigValue.valueArray)[2];
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

            controllerFile<<maxUsageMicroseconds<<" "<<periodMicroseconds;

            if(controllerFile.fail()) {
                TYPELOGV(ERRNO_LOG, "write", strerror(errno));
            }
            controllerFile.close();
        }
    } else {
        TYPELOGV(VERIFIER_CGROUP_NOT_FOUND, cGroupIdentifier);
    }
}

static void setCpuLatency(void* context) {
    if(context == nullptr) return;
    Resource* resource = static_cast<Resource*>(context);

    if(resource->getValuesCount() != 2) return;
    if(resource->mConfigValue.valueArray == nullptr) return;

    int32_t cGroupIdentifier = (*resource->mConfigValue.valueArray)[0];
    int64_t latencyValue = (*resource->mConfigValue.valueArray)[1];
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

            controllerFile<<latencyValue;

            if(controllerFile.fail()) {
                TYPELOGV(ERRNO_LOG, "write", strerror(errno));
            }
            controllerFile.close();
        }
    } else {
        TYPELOGV(VERIFIER_CGROUP_NOT_FOUND, cGroupIdentifier);
    }
}

RESTUNE_REGISTER_APPLIER_CB(0x00090000, addProcessToCgroup);
RESTUNE_REGISTER_APPLIER_CB(0x00090001, addThreadToCgroup);
RESTUNE_REGISTER_APPLIER_CB(0x00090002, setRunOnCores);
RESTUNE_REGISTER_APPLIER_CB(0x00090003, setRunOnCoresExclusively);
RESTUNE_REGISTER_APPLIER_CB(0x00090004, freezeCgroup);
RESTUNE_REGISTER_APPLIER_CB(0x00090005, limitCpuTime);
RESTUNE_REGISTER_APPLIER_CB(0x00090006, setCpuIdle);
RESTUNE_REGISTER_APPLIER_CB(0x00090007, setUClampMin);
RESTUNE_REGISTER_APPLIER_CB(0x00090008, setUClampMax);
RESTUNE_REGISTER_APPLIER_CB(0x00090009, setRelativeCPUShare);
RESTUNE_REGISTER_APPLIER_CB(0x0009000a, setMaxMemoryLimit);
RESTUNE_REGISTER_APPLIER_CB(0x0009000b, setMinMemoryFloor);
RESTUNE_REGISTER_APPLIER_CB(0x0009000c, setCpuLatency);

static void removeProcessFromCGroup(void* context) {
    if(context == nullptr) return;
    Resource* resource = static_cast<Resource*>(context);

    if(resource->getValuesCount() != 2) return;
    if(resource->mConfigValue.valueArray == nullptr) return;

    int32_t pid = (*resource->mConfigValue.valueArray)[1];

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
    if(resource->mConfigValue.valueArray == nullptr) return;

    int32_t tid = (*resource->mConfigValue.valueArray)[1];

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

static void resetRunOnCores(void* context) {
    if(context == nullptr) return;
    Resource* resource = static_cast<Resource*>(context);
    ResourceConfigInfo* resourceConfigInfo =
        ResourceRegistry::getInstance()->getResourceById(resource->getOpCode());

    if(resourceConfigInfo == nullptr) return;

    if(resource->getValuesCount() < 2) return;
    if(resource->mConfigValue.valueArray == nullptr) return;

    int32_t cGroupIdentifier = (*resource->mConfigValue.valueArray)[0];
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

            controllerFile<<(*cGroupConfig->mDefaultValues)["cpuset.cpus"];

            if(controllerFile.fail()) {
                TYPELOGV(ERRNO_LOG, "write", strerror(errno));
            }
            controllerFile.close();
        }
    } else {
        TYPELOGV(VERIFIER_CGROUP_NOT_FOUND, cGroupIdentifier);
    }
}

static void resetRunOnCoresExclusively(void* context) {
    if(context == nullptr) return;
    Resource* resource = static_cast<Resource*>(context);

    if(resource->getValuesCount() < 2) return;
    if(resource->mConfigValue.valueArray == nullptr) return;

    int32_t cGroupIdentifier = (*resource->mConfigValue.valueArray)[0];
    CGroupConfigInfo* cGroupConfig = TargetRegistry::getInstance()->getCGroupConfig(cGroupIdentifier);

    if(cGroupConfig != nullptr && cGroupConfig->mDefaultValues != nullptr) {
        const std::string cGroupName = cGroupConfig->mCgroupName;

        if(cGroupName.length() > 0) {
            const std::string cGroupControllerFilePath =
                ResourceTunerSettings::mBaseCGroupPath + cGroupName + "/cpuset.cpus";

            std::ofstream controllerFile(cGroupControllerFilePath);
            if(!controllerFile.is_open()) {
                TYPELOGV(ERRNO_LOG, "open", strerror(errno));
                return;
            }

            controllerFile<<(*cGroupConfig->mDefaultValues)["cpuset.cpus"];

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

            partitionFile<<(*cGroupConfig->mDefaultValues)["cpuset.cpus.partition"];

            if(partitionFile.fail()) {
                TYPELOGV(ERRNO_LOG, "write", strerror(errno));
            }
            partitionFile.close();
        }
    } else {
        TYPELOGV(VERIFIER_CGROUP_NOT_FOUND, cGroupIdentifier);
    }
}

static void resetCgroupFreeze(void* context) {
    if(context == nullptr) return;
    Resource* resource = static_cast<Resource*>(context);
    ResourceConfigInfo* resourceConfigInfo =
        ResourceRegistry::getInstance()->getResourceById(resource->getOpCode());

    if(resourceConfigInfo == nullptr) return;

    if(resource->getValuesCount() != 2) return;

    int32_t cGroupIdentifier = (*resource->mConfigValue.valueArray)[0];
    int32_t freezeStatus = (*resource->mConfigValue.valueArray)[1];

    if(freezeStatus != 0 && freezeStatus != 1) return;
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

            controllerFile<<(*cGroupConfig->mDefaultValues)["cgroup.freeze"];

            if(controllerFile.fail()) {
                TYPELOGV(ERRNO_LOG, "write", strerror(errno));
            }
            controllerFile.close();
        }
    } else {
        TYPELOGV(VERIFIER_CGROUP_NOT_FOUND, cGroupIdentifier);
    }
}

static void resetUClampMin(void* context) {
    if(context == nullptr) return;
    Resource* resource = static_cast<Resource*>(context);
    ResourceConfigInfo* resourceConfigInfo =
        ResourceRegistry::getInstance()->getResourceById(resource->getOpCode());

    if(resourceConfigInfo == nullptr) return;

    if(resource->getValuesCount() != 2) return;
    if(resource->mConfigValue.valueArray == nullptr) return;

    int32_t cGroupIdentifier = (*resource->mConfigValue.valueArray)[0];
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
            controllerFile<<(*cGroupConfig->mDefaultValues)["cpu.uclamp.min"];

            if(controllerFile.fail()) {
                TYPELOGV(ERRNO_LOG, "write", strerror(errno));
            }
            controllerFile.close();
        }
    } else {
        TYPELOGV(VERIFIER_CGROUP_NOT_FOUND, cGroupIdentifier);
    }
}

static void resetUClampMax(void* context) {
    if(context == nullptr) return;
    Resource* resource = static_cast<Resource*>(context);
    ResourceConfigInfo* resourceConfigInfo =
        ResourceRegistry::getInstance()->getResourceById(resource->getOpCode());

    if(resourceConfigInfo == nullptr) return;

    if(resource->getValuesCount() != 2) return;
    if(resource->mConfigValue.valueArray == nullptr) return;

    int32_t cGroupIdentifier = (*resource->mConfigValue.valueArray)[0];
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
            controllerFile<<(*cGroupConfig->mDefaultValues)["cpu.uclamp.max"];

            if(controllerFile.fail()) {
                TYPELOGV(ERRNO_LOG, "write", strerror(errno));
            }
            controllerFile.close();
        }
    } else {
        TYPELOGV(VERIFIER_CGROUP_NOT_FOUND, cGroupIdentifier);
    }
}

static void resetMaxMemoryLimit(void* context) {
    if(context == nullptr) return;
    Resource* resource = static_cast<Resource*>(context);
    ResourceConfigInfo* resourceConfigInfo =
        ResourceRegistry::getInstance()->getResourceById(resource->getOpCode());

    if(resourceConfigInfo == nullptr) return;

    if(resource->getValuesCount() != 2) return;
    if(resource->mConfigValue.valueArray == nullptr) return;

    int32_t cGroupIdentifier = (*resource->mConfigValue.valueArray)[0];
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

            controllerFile<<(*cGroupConfig->mDefaultValues)["memory.max"];

            if(controllerFile.fail()) {
                TYPELOGV(ERRNO_LOG, "write", strerror(errno));
            }
            controllerFile.close();
        }
    } else {
        TYPELOGV(VERIFIER_CGROUP_NOT_FOUND, cGroupIdentifier);
    }
}

static void resetMinMemoryFloor(void* context) {
    if(context == nullptr) return;
    Resource* resource = static_cast<Resource*>(context);
    ResourceConfigInfo* resourceConfigInfo =
        ResourceRegistry::getInstance()->getResourceById(resource->getOpCode());

    if(resourceConfigInfo == nullptr) return;

    if(resource->getValuesCount() != 2) return;
    if(resource->mConfigValue.valueArray == nullptr) return;

    int32_t cGroupIdentifier = (*resource->mConfigValue.valueArray)[0];
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

            controllerFile<<(*cGroupConfig->mDefaultValues)["memory.min"];

            if(controllerFile.fail()) {
                TYPELOGV(ERRNO_LOG, "write", strerror(errno));
            }
            controllerFile.close();
        }
    } else {
        TYPELOGV(VERIFIER_CGROUP_NOT_FOUND, cGroupIdentifier);
    }
}

static void resetCpuTime(void* context) {
    if(context == nullptr) return;
    Resource* resource = static_cast<Resource*>(context);
    ResourceConfigInfo* resourceConfigInfo =
        ResourceRegistry::getInstance()->getResourceById(resource->getOpCode());

    if(resourceConfigInfo == nullptr) return;

    if(resource->getValuesCount() != 3) return;
    if(resource->mConfigValue.valueArray == nullptr) return;

    int32_t cGroupIdentifier = (*resource->mConfigValue.valueArray)[0];
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

            controllerFile<<(*cGroupConfig->mDefaultValues)["cpu.max"];

            if(controllerFile.fail()) {
                TYPELOGV(ERRNO_LOG, "write", strerror(errno));
            }
            controllerFile.close();
        }
    } else {
        TYPELOGV(VERIFIER_CGROUP_NOT_FOUND, cGroupIdentifier);
    }
}

static void resetCpuIdle(void* context) {
    if(context == nullptr) return;
    Resource* resource = static_cast<Resource*>(context);
    ResourceConfigInfo* resourceConfigInfo =
        ResourceRegistry::getInstance()->getResourceById(resource->getOpCode());

    if(resourceConfigInfo == nullptr) return;

    if(resource->getValuesCount() != 1) return;

    int32_t cGroupIdentifier = resource->mConfigValue.singleValue;
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

            controllerFile<<(*cGroupConfig->mDefaultValues)["cpu.idle"];

            if(controllerFile.fail()) {
                TYPELOGV(ERRNO_LOG, "write", strerror(errno));
            }
            controllerFile.close();
        }
    } else {
        TYPELOGV(VERIFIER_CGROUP_NOT_FOUND, cGroupIdentifier);
    }
}

static void resetRelativeCPUShare(void* context) {
    if(context == nullptr) return;
    Resource* resource = static_cast<Resource*>(context);
    ResourceConfigInfo* resourceConfigInfo =
        ResourceRegistry::getInstance()->getResourceById(resource->getOpCode());

    if(resourceConfigInfo == nullptr) return;

    if(resource->getValuesCount() != 2) return;
    if(resource->mConfigValue.valueArray == nullptr) return;

    int32_t cGroupIdentifier = (*resource->mConfigValue.valueArray)[0];
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

            controllerFile<<(*cGroupConfig->mDefaultValues)["cpu.weight"];

            if(controllerFile.fail()) {
                TYPELOGV(ERRNO_LOG, "write", strerror(errno));
            }
            controllerFile.close();
        }
    } else {
        TYPELOGV(VERIFIER_CGROUP_NOT_FOUND, cGroupIdentifier);
    }
}

static void resetCpuLatency(void* context) {
    if(context == nullptr) return;
    Resource* resource = static_cast<Resource*>(context);

    if(resource->getValuesCount() != 2) return;
    if(resource->mConfigValue.valueArray == nullptr) return;

    int32_t cGroupIdentifier = (*resource->mConfigValue.valueArray)[0];
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

            controllerFile<<(*cGroupConfig->mDefaultValues)["cpu.latency_nice"];;

            if(controllerFile.fail()) {
                TYPELOGV(ERRNO_LOG, "write", strerror(errno));
            }
            controllerFile.close();
        }
    } else {
        TYPELOGV(VERIFIER_CGROUP_NOT_FOUND, cGroupIdentifier);
    }
}

RESTUNE_REGISTER_TEAR_CB(0x00090000, removeProcessFromCGroup);
RESTUNE_REGISTER_TEAR_CB(0x00090001, removeThreadFromCGroup);
RESTUNE_REGISTER_TEAR_CB(0x00090002, resetRunOnCores);
RESTUNE_REGISTER_TEAR_CB(0x00090003, resetRunOnCoresExclusively);
RESTUNE_REGISTER_TEAR_CB(0x00090004, resetCgroupFreeze);
RESTUNE_REGISTER_TEAR_CB(0x00090005, resetCpuTime);
RESTUNE_REGISTER_TEAR_CB(0x00090006, resetCpuIdle);
RESTUNE_REGISTER_TEAR_CB(0x00090007, resetUClampMin);
RESTUNE_REGISTER_TEAR_CB(0x00090008, resetUClampMax);
RESTUNE_REGISTER_TEAR_CB(0x00090009, resetRelativeCPUShare);
RESTUNE_REGISTER_TEAR_CB(0x0009000a, resetMaxMemoryLimit);
RESTUNE_REGISTER_TEAR_CB(0x0009000b, resetMinMemoryFloor);
RESTUNE_REGISTER_TEAR_CB(0x0009000c, resetCpuLatency);
