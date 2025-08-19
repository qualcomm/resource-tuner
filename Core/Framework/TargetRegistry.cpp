// Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
// SPDX-License-Identifier: BSD-3-Clause-Clear

#include "TargetRegistry.h"

static std::vector<int8_t> readRelatedCpus(const std::string& policyPath) {
    std::vector<int8_t> cpus;
    std::ifstream file;

    file.open(policyPath + "/related_cpus");
    if(!file.is_open()) {
        LOGE("RTN_TARGET_REGISTRY",
             "Failed to open file: " + policyPath + "/related_cpus");
        throw std::ifstream::failure("Failed to open file");
    }

    std::string line;
    if(getline(file, line)) {
        size_t start = 0;
        while(start < line.size()) {
            size_t end = line.find(' ', start);
            std::string token = line.substr(start, end - start);
            if(!token.empty()) {
                cpus.push_back(static_cast<int8_t>(stoi(token)));
            }
            if(end == std::string::npos) {
                break;
            }
            start = end + 1;
        }
    }

    ResourceTunerSettings::targetConfigs.totalCoreCount += cpus.size();
    return cpus;
}

static int32_t getOnlineCpuCount() {
    std::ifstream file;
    try {
        file.open(ONLINE_CPU_FILE_PATH);
        if(!file.is_open()) {
            LOGE("RTN_TARGET_CONFIG_PROCESSOR",
                 "Failed to Open file: " ONLINE_CPU_FILE_PATH);
            return 0;
        }

        std::string line;
        std::getline(file, line);

        std::stringstream ss(line);
        std::string token;
        int32_t cpuIndex = 0;

        while(std::getline(ss, token, '-')) {
            cpuIndex = std::max(cpuIndex, std::stoi(token));
        }

        return cpuIndex + 1;

    } catch(const std::invalid_argument& ex) {
        LOGE("RTN_TARGET_REGISTRY",
             "Dynamic Logical to Physical Mapping Failed,\
             std::invalid_argument::what(): " + std::string(ex.what()));
    } catch(const std::out_of_range& ex) {
        LOGE("RTN_TARGET_REGISTRY",
             "Dynamic Logical to Physical Mapping Failed,\
             std::out_of_range::what(): " + std::string(ex.what()));
    } catch(const std::ifstream::failure& ex) {
        LOGE("RTN_TARGET_REGISTRY",
             "Dynamic Logical to Physical Mapping Failed,\
             std::ifstream::failure::what(): " + std::string(ex.what()));
    } catch(const std::exception& ex) {
        LOGE("RTN_TARGET_REGISTRY",
             "Dynamic Logical to Physical Mapping"\
             "Failed, with error: " + std::string(ex.what()));
    }

    return 0;
}

std::shared_ptr<TargetRegistry> TargetRegistry::targetRegistryInstance = nullptr;
TargetRegistry::TargetRegistry() {}

void TargetRegistry::setTargetName(const std::string& targetName) {
    this->mTargetName = targetName;
    ResourceTunerSettings::targetConfigs.targetName = targetName;
}

void TargetRegistry::setTotalCoreCount(uint8_t totalCoreCount) {
    this->mTotalCoreCount = totalCoreCount;
    ResourceTunerSettings::targetConfigs.totalCoreCount = this->mTotalCoreCount;
}

void TargetRegistry::addCGroupMapping(CGroupConfigInfo* cGroupConfigInfo) {
    this->mCGroupMapping[cGroupConfigInfo->mCgroupID] = cGroupConfigInfo;
}

int8_t TargetRegistry::addMapping(const std::string& clusterName, int8_t physicalClusterId) {
    if((clusterName != "little") &&
       (clusterName != "big") &&
       (clusterName != "prime") &&
       (clusterName != "titanium")) {
        LOGE("RTN_TARGET_CONFIG_PROCESSOR", "Incorrect cluster info");
        return RC_INVALID_VALUE;
    }

    if(physicalClusterId < 0 || physicalClusterId >= CLUSTER_TYPE_COUNT) {
        LOGE("RTN_TARGET_CONFIG_PROCESSOR", "Incorrect cluster id");
        return RC_INVALID_VALUE;
    }

    this->mClusterTypeToPhysicalSlotMapping[clusterName] = physicalClusterId;
    return RC_SUCCESS;
}

int8_t TargetRegistry::readPhysicalMapping() {
    std::vector<std::string> policyDirs;

    DIR* dir = opendir(POLICY_DIR_PATH);
    if(dir == nullptr) {
        return RC_FILE_NOT_FOUND;
    }

    struct dirent* entry;
    while((entry = readdir(dir)) != nullptr) {
        if(strncmp(entry->d_name, "policy", 6) == 0) {
            policyDirs.push_back(entry->d_name);
        }
    }
    closedir(dir);

    std::sort(policyDirs.begin(), policyDirs.end());

    int8_t physicalClusterId = 0;
    for(const std::string& dirName : policyDirs) {
        std::string fullPath = std::string(POLICY_DIR_PATH) + dirName;
        std::vector<int8_t> cpus;
        try {
            cpus = readRelatedCpus(fullPath);
        } catch(const std::exception& e) {
            LOGE("RTN_TARGET_CONFIG_PROCESSOR", "Failed to read policy dir: " + fullPath);
            return RC_FILE_NOT_FOUND;
        }

        if(!cpus.empty()) {
            this->mPhysicalMapping[physicalClusterId] = cpus;
            physicalClusterId++;
        }
    }

    if(this->mPhysicalMapping.size() == 0) {
        return RC_FILE_NOT_FOUND;
    }

    return RC_SUCCESS;
}

int8_t TargetRegistry::addClusterSpreadInfo(int8_t physicalClusterId, int32_t coreCount) {
    if(physicalClusterId < 0 || physicalClusterId >= CLUSTER_TYPE_COUNT || coreCount < 0) {
        LOGE("RTN_TARGET_CONFIG_PROCESSOR", "Invalid Cluster/Core Data id");
        return RC_INVALID_VALUE;
    }
    this->mClusterSpreadInfo.push_back({physicalClusterId, coreCount});
    return RC_SUCCESS;
}

ErrCode TargetRegistry::readPhysicalCoreClusterInfo() {
    // Check if a Mapping file has been provided
    if(this->mClusterSpreadInfo.size() > 0) {
        LOGD("RTN_TARGET_CONFIG_PROCESSOR", "Reading Physical Cluster-Core Mapping data from BU-provided Config File");
        std::sort(this->mClusterSpreadInfo.begin(), this->mClusterSpreadInfo.end());

        int32_t coreCpuIndex = 0;
        for(std::pair<int8_t, int32_t> clusterSpreadInfoItem: this->mClusterSpreadInfo) {
            for(int32_t count = 0; count < clusterSpreadInfoItem.second; count++) {
                this->mPhysicalMapping[clusterSpreadInfoItem.first].push_back(coreCpuIndex++);
            }
        }

    } else {
        // No Mapping File Provided, Try to obtain the Mapping Dynamically
        LOGD("RTN_TARGET_CONFIG_PROCESSOR", "Dynamically Reading Physical Cluster-Core Mapping data from /cpufreq/policy/* directories");
        ResourceTunerSettings::targetConfigs.totalCoreCount = 0;
        int8_t opStatus = readPhysicalMapping();

        if(RC_IS_NOTOK(opStatus)) {
            LOGD("RTN_TARGET_CONFIG_PROCESSOR", "Falling back to Default Heurisitc based core assignment");

            // Fallback to a Default Heuristic based assignment
            int32_t totalOnlineCpuCount;
            try {
                totalOnlineCpuCount = getOnlineCpuCount();
            } catch(const std::exception& e) {
                LOGE("RTN_TARGET_CONFIG_PROCESSOR", "Failed to read online cpu count");
                return RC_FILE_NOT_FOUND;
            }

            ResourceTunerSettings::targetConfigs.totalCoreCount = totalOnlineCpuCount;

            int32_t coreCountPerCluster = totalOnlineCpuCount / CLUSTER_TYPE_COUNT;
            int32_t pendingAllocationCount = totalOnlineCpuCount % CLUSTER_TYPE_COUNT;

            int32_t coreCpuIndex = 0;
            for(int8_t physicalClusterId = 0; physicalClusterId < CLUSTER_TYPE_COUNT; physicalClusterId++) {
                for(int32_t coreCount = 0; coreCount < coreCountPerCluster; coreCount++) {
                    this->mPhysicalMapping[physicalClusterId].push_back(coreCpuIndex++);
                }
                if(pendingAllocationCount > 0) {
                    this->mPhysicalMapping[physicalClusterId].push_back(coreCpuIndex++);
                    pendingAllocationCount--;
                }
            }
        }
    }

    // Map the cores to the Correct Cluster Type.
    // This is needed since all Targets need not follow the convention specified by logicalClustersAPIConvention.
    for(std::pair<std::string, int8_t> mapping: this->mClusterTypeToPhysicalSlotMapping) {
        std::string clusterName = mapping.first;
        int8_t physicalSlot = mapping.second;
        if(this->mPhysicalMapping.find(physicalSlot) == this->mPhysicalMapping.end()) {
            return RC_INVALID_VALUE;
        }
        this->mClusterTypeToPhysicalCores[clusterName] = this->mPhysicalMapping[physicalSlot];
    }

    return RC_SUCCESS;
}

void TargetRegistry::displayLogicalToPhysicalMapping() {
    for(std::pair<std::string, std::vector<int8_t>> entry: mClusterTypeToPhysicalCores) {
        LOGD("RTN_LOGICAL_CORE_TRANSLATOR","=============");

        LOGD("RTN_LOGICAL_CORE_TRANSLATOR","LogicalClusterId " + entry.first);
        for(int8_t cores: entry.second) {
            LOGD("RTN_LOGICAL_CORE_TRANSLATOR", "Core " + std::to_string(cores));
        }
    }
}

int32_t TargetRegistry::getPhysicalCoreId(int32_t logicalClusterId, int32_t logicalCoreCount) const {
    if(this->logicalClustersAPIConvention.find(logicalClusterId) ==
            this->logicalClustersAPIConvention.end()) {
        return -1;
    }

    std::string clusterType = this->logicalClustersAPIConvention.at(logicalClusterId);
    int32_t targetIndex = logicalCoreCount - 1;
    if(targetIndex < 0 || targetIndex >= this->mClusterTypeToPhysicalCores.at(clusterType).size()) {
        return -1;
    }

    if(this->mClusterTypeToPhysicalCores.find(clusterType) == this->mClusterTypeToPhysicalCores.end()) {
        // Edge Case, Control Should not reach here.
        return -1;
    }

    return this->mClusterTypeToPhysicalCores.at(clusterType).at(targetIndex);
}

int32_t TargetRegistry::getPhysicalClusterId(int32_t logicalClusterId) const {
    if(this->logicalClustersAPIConvention.find(logicalClusterId) ==
            this->logicalClustersAPIConvention.end()) {
        return -1;
    }
    std::string clusterType = this->logicalClustersAPIConvention.at(logicalClusterId);
    return this->mClusterTypeToPhysicalSlotMapping.at(clusterType);
}

void TargetRegistry::getCGroupConfigs(std::vector<CGroupConfigInfo*>& cGroupConfigs) {
    for(std::pair<int8_t, CGroupConfigInfo*> cGroup: mCGroupMapping) {
        cGroupConfigs.push_back(cGroup.second);
    }
}

CGroupConfigInfo* TargetRegistry::getCGroupConfig(int8_t cGroupID) {
    return this->mCGroupMapping[cGroupID];
}

CGroupConfigInfoBuilder::CGroupConfigInfoBuilder() {
    this->mCGroupConfigInfo = new CGroupConfigInfo;
}

CGroupConfigInfoBuilder* CGroupConfigInfoBuilder::setCGroupName(const std::string& cGroupName) {
    if(this->mCGroupConfigInfo == nullptr) {
        return this;
    }

    this->mCGroupConfigInfo->mCgroupName = cGroupName;
    return this;
}

CGroupConfigInfoBuilder* CGroupConfigInfoBuilder::setCGroupID(int8_t cGroupIdentifier) {
    if(this->mCGroupConfigInfo == nullptr) {
        return this;
    }

    this->mCGroupConfigInfo->mCgroupID = cGroupIdentifier;
    return this;
}

CGroupConfigInfoBuilder* CGroupConfigInfoBuilder::setThreaded(int8_t isThreaded) {
    if(this->mCGroupConfigInfo == nullptr) {
        return this;
    }

    this->mCGroupConfigInfo->isThreaded = isThreaded;
    return this;
}

CGroupConfigInfo* CGroupConfigInfoBuilder::build() {
    return this->mCGroupConfigInfo;
}
