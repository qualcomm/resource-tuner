// Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
// SPDX-License-Identifier: BSD-3-Clause-Clear

#include <unistd.h>

#include "Extensions.h"
#include "TargetRegistry.h"
#include "ResourceRegistry.h"

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
            const std::string cGroupControllerFilePath =
                "/sys/fs/cgroup/" + cGroupName + "/tasks/cgroup.procs";

            std::ofstream controllerFile(cGroupControllerFilePath, std::ios::app);
            if(!controllerFile.is_open()) {
                TYPELOGV(ERRNO_LOG, "open", strerror(errno));
                return;
            }

            controllerFile<<pid<<std::endl;
            controllerFile.close();
        }
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
            const std::string cGroupControllerFilePath =
                "/sys/fs/cgroup/" + cGroupName + "/cgroup.threads";
            std::ofstream controllerFile(cGroupControllerFilePath, std::ios::app);
            if(!controllerFile.is_open()) {
                TYPELOGV(ERRNO_LOG, "open", strerror(errno));
                return;
            }
            controllerFile<<tid<<std::endl;
            controllerFile.close();
        }
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
            const std::string cGroupControllerFilePath =
                "/sys/fs/cgroup/" + cGroupName + "/cpuset.cpus";

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
        }
    }
}

static void setRunOnCoresExclusively(void* context) {}

static void freezeCgroup(void* context) {
    if(context == nullptr) return;
    Resource* resource = static_cast<Resource*>(context);

    if(resource->getValuesCount() != 1) return;

    int32_t cGroupIdentifier = resource->mConfigValue.singleValue;
    CGroupConfigInfo* cGroupConfig = TargetRegistry::getInstance()->getCGroupConfig(cGroupIdentifier);

    if(cGroupConfig != nullptr) {
        const std::string cGroupPath = cGroupConfig->mCgroupName;
        if(cGroupPath.length() > 0) {
            std::ofstream controllerFile("/sys/fs/cgroup/" + cGroupPath  + "/cgroup.freeze");
            if(!controllerFile.is_open()) {
                TYPELOGV(ERRNO_LOG, "open", strerror(errno));
                return;
            }
            controllerFile << "1";
            controllerFile.close();
        }
    }
}

static void unFreezeCGroup(void* context) {
    if(context == nullptr) return;
    Resource* resource = static_cast<Resource*>(context);

    if(resource->getValuesCount() != 1) return;

    int32_t cGroupIdentifier = resource->mConfigValue.singleValue;
    CGroupConfigInfo* cGroupConfig = TargetRegistry::getInstance()->getCGroupConfig(cGroupIdentifier);

    if(cGroupConfig != nullptr) {
        const std::string cGroupPath = cGroupConfig->mCgroupName;
        if(cGroupPath.length() > 0) {
            std::ofstream controllerFile("/sys/fs/cgroup/" + cGroupPath  + "/cgroup.freeze");
            if(!controllerFile.is_open()) {
                TYPELOGV(ERRNO_LOG, "open", strerror(errno));
                return;
            }
            controllerFile << "0";
            controllerFile.close();
        }
    }
}

static void setCpuIdle(void* context) {
    if(context == nullptr) return;
    Resource* resource = static_cast<Resource*>(context);

    if(resource->getValuesCount() != 1) return;

    int32_t cGroupIdentifier = resource->mConfigValue.singleValue;
    CGroupConfigInfo* cGroupConfig = TargetRegistry::getInstance()->getCGroupConfig(cGroupIdentifier);

    if(cGroupConfig != nullptr) {
        const std::string cGroupPath = cGroupConfig->mCgroupName;
        if(cGroupPath.length() > 0) {
            std::ofstream controllerFile("/sys/fs/cgroup/" + cGroupPath  + "/cpu.idle");
            if(!controllerFile.is_open()) {
                TYPELOGV(ERRNO_LOG, "open", strerror(errno));
                return;
            }
            controllerFile << "1";
            controllerFile.close();
        }
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
        const std::string cGroupPath = cGroupConfig->mCgroupName;
        if(cGroupPath.length() > 0) {
            std::ofstream controllerFile("/sys/fs/cgroup/" + cGroupPath + "/cpu.uclamp.min");
            if(!controllerFile.is_open()) {
                TYPELOGV(ERRNO_LOG, "open", strerror(errno));
                return;
            }
            controllerFile << clampVal;
            controllerFile.close();
        }
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
        const std::string cGroupPath = cGroupConfig->mCgroupName;
        if(cGroupPath.length() > 0) {
            std::ofstream controllerFile("/sys/fs/cgroup/" + cGroupPath + "/cpu.uclamp.max");
            if(!controllerFile.is_open()) {
                TYPELOGV(ERRNO_LOG, "open", strerror(errno));
                return;
            }
            controllerFile << clampVal;
            controllerFile.close();
        }
    }
}

static void setRelativeCPUWeight(void* context) {
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
            std::ofstream controllerFile("/sys/fs/cgroup/" + cGroupName + "/cpu.weight");
            if(!controllerFile.is_open()) {
                TYPELOGV(ERRNO_LOG, "open", strerror(errno));
                return;
            }
            controllerFile << relativeWeight;
            controllerFile.close();
        }
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
            std::ofstream controllerFile("/sys/fs/cgroup/" + cGroupName + "/memory.max");
            if(!controllerFile.is_open()) {
                TYPELOGV(ERRNO_LOG, "open", strerror(errno));
                return;
            }
            controllerFile<<maxMemoryLimit;
            controllerFile.close();
        }
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
            std::ofstream controllerFile("/sys/fs/cgroup/" + cGroupName + "/memory.min");
            if(!controllerFile.is_open()) {
                TYPELOGV(ERRNO_LOG, "open", strerror(errno));
                return;
            }
            controllerFile<<minMemoryFloor;
            controllerFile.close();
        }
    }
}

RTN_REGISTER_RESOURCE(0x00090000, addProcessToCgroup);
RTN_REGISTER_RESOURCE(0x00090001, addThreadToCgroup);
RTN_REGISTER_RESOURCE(0x00090002, setRunOnCores);
RTN_REGISTER_RESOURCE(0x00090003, setRunOnCoresExclusively);
RTN_REGISTER_RESOURCE(0x00090004, freezeCgroup);
RTN_REGISTER_RESOURCE(0x00090005, unFreezeCGroup);
RTN_REGISTER_RESOURCE(0x00090006, setCpuIdle);
RTN_REGISTER_RESOURCE(0x00090007, setUClampMin);
RTN_REGISTER_RESOURCE(0x00090008, setUClampMax);
RTN_REGISTER_RESOURCE(0x00090009, setRelativeCPUWeight);
RTN_REGISTER_RESOURCE(0x0009000a, setMaxMemoryLimit);
RTN_REGISTER_RESOURCE(0x0009000b, setMinMemoryFloor);
