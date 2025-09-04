// Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
// SPDX-License-Identifier: BSD-3-Clause-Clear

#include "TargetRegistry.h"

// Create all the CGroups specified via InitConfig.yaml during the init phase.
static ErrCode createCGroup(CGroupConfigInfo* cGroupConfig) {
    if(cGroupConfig == nullptr) return RC_BAD_ARG;
    if(!cGroupConfig->mCreationNeeded) return RC_SUCCESS;

    std::string cGroupPath = ResourceTunerSettings::mBaseCGroupPath + cGroupConfig->mCgroupName;
    if(mkdir(cGroupPath.c_str(), 0755) == 0) {
        if(cGroupConfig->mIsThreaded) {
            AuxRoutines::writeToFile(cGroupPath + "/cgroup.type", "threaded");
        }
    } else {
        TYPELOGV(ERRNO_LOG, "mkdir", strerror(errno));
    }

    return RC_SUCCESS;
}

static ErrCode createMpamGroup(MpamGroupConfigInfo* mpamGroupConfig) {
    if(mpamGroupConfig == nullptr) return RC_BAD_ARG;

    return RC_SUCCESS;
}

// Dynamic Utilities to fetch Target Info
// Number of CPUs currently online
static int32_t getOnlineCpuCount() {
    std::ifstream file;
    try {
        file.open(ONLINE_CPU_FILE_PATH);
        if(!file.is_open()) {
            LOGE("RESTUNE_SERVER_INIT", "Failed to Open file: " ONLINE_CPU_FILE_PATH);
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

    } catch(const std::exception& e) {
        TYPELOGV(CORE_COUNT_EXTRACTION_FAILED, e.what());
    }

    return 0;
}

// This routine gets the list of cpus corresponding to a particular Cluster ID.
// For example: C0 -> cpu0, cpu1, cpu2
static ErrCode readRelatedCpus(const std::string& policyPath,
                               std::vector<int32_t>& cpus) {
    try {
        std::ifstream file;

        file.open(policyPath + "/related_cpus");
        if(!file.is_open()) {
            LOGE("RESTUNE_SERVER_INIT", "Failed to Open file: " + policyPath + "/related_cpus");
            return RC_FILE_NOT_FOUND;
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
    } catch(const std::exception& e) {
        TYPELOGV(CLUSTER_CPU_LIST_EXTRACTION_FAILED, policyPath.c_str(), e.what());
    }

    return RC_SUCCESS;
}

// Get the capacity of a cpu, indexed by it's integer ID.
static int32_t readCpuCapacity(int8_t cpuID) {
    char filePath[128];
    snprintf(filePath, sizeof(filePath), CPU_CAPACITY_FILE_PATH, cpuID);
    std::string capacityString = AuxRoutines::readFromFile(filePath);
    int32_t capacity = 0;

    try {
        capacity = std::stoi(capacityString);
    } catch(std::exception& e) {
        TYPELOGV(CLUSTER_CPU_CAPACITY_EXTRACTION_FAILED, cpuID, e.what());
    }

    return capacity;
}

void TargetRegistry::generatePolicyBasedMapping(std::vector<std::string>& policyDirs) {
    // Sort the directories, to ensure processing always starts with policy0
    std::sort(policyDirs.begin(), policyDirs.end());

    // Number of policy directories, is equivalent to number of clusters
    ResourceTunerSettings::targetConfigs.totalClusterCount = policyDirs.size();

    // Next, get the list of cpus corresponding to each cluster
    std::vector<std::pair<int32_t, ClusterInfo*>> clusterConfigs;

    int8_t physicalClusterId = 0;
    for(const std::string& dirName : policyDirs) {
        std::string fullPath = std::string(POLICY_DIR_PATH) + dirName;
        std::vector<int32_t>* cpuList = new std::vector<int32_t>;

        if(RC_IS_OK(readRelatedCpus(fullPath, *cpuList))) {
            int32_t clusterCapacity = 0;
            ClusterInfo* clusterInfo = new ClusterInfo;
            clusterInfo->mPhysicalID = physicalClusterId;
            clusterInfo->mCpuList = cpuList;
            clusterInfo->mNumCpus = cpuList->size();

            if(!cpuList->empty()) {
                // If this cluster has a non-zero number of cores, then
                // proceed with determining the Cluster Capcity
                int8_t cpuID = (*cpuList)[0];
                clusterCapacity = readCpuCapacity(cpuID);
            }

            clusterInfo->mCapacity = clusterCapacity;
            clusterConfigs.push_back({clusterCapacity, clusterInfo});
            this->mPhysicalClusters[physicalClusterId] = clusterInfo;
            physicalClusterId += cpuList->size();
        }
    }

    std::sort(clusterConfigs.begin(), clusterConfigs.end());

    // Now, Create the Logical to Physical Mappings
    // Note the Clusters are arranged in increasing order of Capacities
    for(int32_t i = 0; i < clusterConfigs.size(); i++) {
        this->mLogicalToPhysicalClusterMapping[i] = clusterConfigs[i].second->mPhysicalID;
    }
}

void TargetRegistry::getClusterIdBasedMapping() {
    const std::string cpuDir = "/sys/devices/system/cpu";
    const std::regex cpuRegex("^cpu([0-9]+)$");

    DIR* dir = opendir(cpuDir.c_str());
    if(dir == nullptr) {
        return;
    }

    std::unordered_map<int32_t, std::vector<int32_t>> clusterToCoreMap;

    struct dirent* entry;
    while((entry = readdir(dir)) != nullptr) {
        if(entry->d_type == DT_DIR) {
            std::cmatch match;
            if(std::regex_match(entry->d_name, match, cpuRegex)) {
                int32_t cpuNum = -1;
                try {
                    cpuNum = std::stoi(match[1]);
                } catch(const std::exception& e) {
                    break;
                }

                std::string clusterIdPath = cpuDir + "/" + entry->d_name + "/topology/cluster_id";
                std::ifstream clusterIdFile(clusterIdPath);
                if(clusterIdFile) {
                    int32_t clusterID;
                    clusterIdFile>>clusterID;
                    if(clusterIdFile) {
                        clusterToCoreMap[clusterID].push_back(cpuNum);
                    }
                }
            }
        }
    }

    closedir(dir);

    ResourceTunerSettings::targetConfigs.totalClusterCount = clusterToCoreMap.size();

    for(std::pair<int32_t, std::vector<int32_t>> entry: clusterToCoreMap) {
        int32_t clusterID = entry.first;
        ClusterInfo* clusterInfo = new ClusterInfo;
        clusterInfo->mPhysicalID = clusterID;
        clusterInfo->mNumCpus = entry.second.size();

        if(entry.second.size() > 0) {
            clusterInfo->mStartCpu = entry.second[0];
            clusterInfo->mCpuList = new std::vector<int32_t>;

            for(int32_t cpu: entry.second) {
                clusterInfo->mStartCpu = std::min(clusterInfo->mStartCpu, cpu);
                clusterInfo->mCpuList->push_back(cpu);
            }
        }
        this->mPhysicalClusters[clusterID] = clusterInfo;
    }

    // Next, get the list of cpus corresponding to each cluster
    std::vector<std::pair<int32_t, ClusterInfo*>> clusterConfigs;

    for(std::pair<int32_t, ClusterInfo*> entry: this->mPhysicalClusters) {
        int32_t clusterCapacity = 0;
        if(entry.second->mCpuList != nullptr && !entry.second->mCpuList->empty()) {
            // If this cluster has a non-zero number of cores, then
            // proceed with determining the Cluster Capcity
            int32_t cpuID = (*entry.second->mCpuList)[0];
            clusterCapacity = readCpuCapacity(cpuID);
        }
        clusterConfigs.push_back({clusterCapacity, entry.second});
    }

    std::sort(clusterConfigs.begin(), clusterConfigs.end());

    // Now, Create the Logical to Physical Mappings
    // Note the Clusters are arranged in increasing order of Capacities
    for(int32_t i = 0; i < clusterConfigs.size(); i++) {
        this->mLogicalToPhysicalClusterMapping[i] = clusterConfigs[i].second->mPhysicalID;
    }
}

std::shared_ptr<TargetRegistry> TargetRegistry::targetRegistryInstance = nullptr;
TargetRegistry::TargetRegistry() {}

void TargetRegistry::readTargetInfo() {
    // Mechanism 1: Check if the User has provided a custom TargetConfig.yaml file
    if(this->mPhysicalClusters.size() > 0) {
        // This case should only be hit if the Target Info has already been provided
        // via the TargetConfig.yaml file, no need to dynamically fetch Target Info.
        return;
    }

    // Get the Online Core Count
    ResourceTunerSettings::targetConfigs.totalCoreCount = getOnlineCpuCount();

    // Mechanism 2: Check if cpufreq/policy directories are available,
    // If yes, we'll use them to generate the mapping info.

    // Get the list of all the policy/ directories
    std::vector<std::string> policyDirs;

    DIR* dir = opendir(POLICY_DIR_PATH);
    if(dir == nullptr) {
        return;
    }

    struct dirent* entry;
    while((entry = readdir(dir)) != nullptr) {
        if(strncmp(entry->d_name, "policy", 6) == 0) {
            policyDirs.push_back(entry->d_name);
        }
    }
    closedir(dir);

    if(policyDirs.size() > 0) {
        // cpufreq/policy directories are available, generate mapping.
        this->generatePolicyBasedMapping(policyDirs);
        return;
    }

    // Mechanism 3, Fallback:
    // Generate mapping using topology/cluster_id node
    this->getClusterIdBasedMapping();
}

void TargetRegistry::getClusterIDs(std::vector<int32_t>& clusterIDs) {
    for(std::pair<int32_t, int32_t> info: this->mLogicalToPhysicalClusterMapping) {
        clusterIDs.push_back(info.second);
    }
}

// Get the Physical Cluster corresponding to a Logical Cluster Id.
int32_t TargetRegistry::getPhysicalClusterId(int32_t logicalClusterId) {
    if(this->mLogicalToPhysicalClusterMapping.find(logicalClusterId) ==
       this->mLogicalToPhysicalClusterMapping.end()) {
        return -1;
    }

    return this->mLogicalToPhysicalClusterMapping[logicalClusterId];
}

// Get the nth Physical Core ID in a particular cluster
int32_t TargetRegistry::getPhysicalCoreId(int32_t logicalClusterId, int32_t logicalCoreCount) {
    if(this->mPhysicalClusters.size() == 0) {
        // In case there are no clusters, i.e. the system is homogeneous then
        // physical core count will be the same as logical core count.
        return logicalCoreCount;
    }

    if(this->mLogicalToPhysicalClusterMapping.find(logicalClusterId) ==
       this->mLogicalToPhysicalClusterMapping.end()) {
        return -1;
    }

    int32_t physicalClusterId = this->mLogicalToPhysicalClusterMapping[logicalClusterId];
    ClusterInfo* clusterInfo = this->mPhysicalClusters[physicalClusterId];

    if(clusterInfo == nullptr || logicalCoreCount <= 0 || logicalCoreCount > clusterInfo->mNumCpus) {
        return -1;
    }

    if(clusterInfo->mCpuList != nullptr) {
        return (*clusterInfo->mCpuList)[logicalCoreCount - 1];
    }
    return clusterInfo->mStartCpu + logicalCoreCount - 1;
}

void TargetRegistry::getCGroupNames(std::vector<std::string>& cGroupNames) {
    for(std::pair<int32_t, CGroupConfigInfo*> cGroup: this->mCGroupMapping) {
        cGroupNames.push_back(cGroup.second->mCgroupName);
    }
}

CGroupConfigInfo* TargetRegistry::getCGroupConfig(int32_t cGroupID) {
    return this->mCGroupMapping[cGroupID];
}

int32_t TargetRegistry::getCreatedCGroupsCount() {
    return this->mCGroupMapping.size();
}


void TargetRegistry::getMpamGroupNames(std::vector<std::string>& mpamGroupNames) {
    for(std::pair<int32_t, MpamGroupConfigInfo*> mpamGroup: this->mMpamGroupMapping) {
        mpamGroupNames.push_back(mpamGroup.second->mMpamGroupName);
    }
}

MpamGroupConfigInfo* TargetRegistry::getMpamGroupConfig(int32_t mpamGroupID) {
    return this->mMpamGroupMapping[mpamGroupID];
}

int32_t TargetRegistry::getCreatedMpamGroupsCount() {
    return this->mMpamGroupMapping.size();
}

void TargetRegistry::addClusterMapping(int32_t logicalID, int32_t physicalID) {
    ClusterInfo* clusterInfo = new ClusterInfo;

    clusterInfo->mPhysicalID = physicalID;
    clusterInfo->mNumCpus = 0;
    clusterInfo->mCapacity = 0;
    clusterInfo->mStartCpu = physicalID;
    clusterInfo->mCpuList = nullptr;

    this->mPhysicalClusters[physicalID] = clusterInfo;
    this->mLogicalToPhysicalClusterMapping[logicalID] = physicalID;

    ResourceTunerSettings::targetConfigs.totalClusterCount = this->mPhysicalClusters.size();
}

void TargetRegistry::addClusterSpreadInfo(int32_t physicalID, int32_t numCores) {
    if(this->mPhysicalClusters.find(physicalID) == this->mPhysicalClusters.end()) {
        return;
    }
    this->mPhysicalClusters[physicalID]->mNumCpus = numCores;
    ResourceTunerSettings::targetConfigs.totalCoreCount += numCores;
}

void TargetRegistry::displayTargetInfo() {
    LOGI("RESTUNE_SERVER_INIT", "Displaying Target Info");
    LOGI("RESTUNE_SERVER_INIT", "Number of Cores = " + std::to_string(ResourceTunerSettings::targetConfigs.totalCoreCount));
    LOGI("RESTUNE_SERVER_INIT", "Number of Clusters = " + std::to_string(ResourceTunerSettings::targetConfigs.totalClusterCount));

    for(std::pair<int32_t, ClusterInfo*> cluster: this->mPhysicalClusters) {
        LOGI("RESTUNE_SERVER_INIT", "Physical ID of cluster: " + std::to_string(cluster.first));
        LOGI("RESTUNE_SERVER_INIT", "Number of Cores in cluster: " + std::to_string(cluster.second->mNumCpus));
        LOGI("RESTUNE_SERVER_INIT", "Starting CPU in this cluster: " + std::to_string(cluster.second->mStartCpu));
        LOGI("RESTUNE_SERVER_INIT", "Cluster Capacity: " + std::to_string(cluster.second->mCapacity));

        if(cluster.second->mCpuList != nullptr) {
            LOGI("RESTUNE_SERVER_INIT", "List of CPUs in this cluster: ");
            for(int32_t i = 0; i < cluster.second->mNumCpus; i++) {
                LOGI("RESTUNE_SERVER_INIT", std::to_string((*cluster.second->mCpuList)[i]));
            }
        }
    }
}

TargetRegistry::~TargetRegistry() {
    for(std::pair<int8_t, CGroupConfigInfo*> cGroupInfo: this->mCGroupMapping) {
        if(cGroupInfo.second == nullptr) return;

        if(cGroupInfo.second != nullptr) {
            delete cGroupInfo.second;
            cGroupInfo.second = nullptr;
        }
    }
}

void TargetRegistry::addCGroupMapping(CGroupConfigInfo* cGroupConfigInfo) {
    if(cGroupConfigInfo == nullptr) return;

    if(cGroupConfigInfo->mCgroupID == -1 || cGroupConfigInfo->mCgroupName.length() == 0) {
        delete cGroupConfigInfo;
        return;
    }

    if(RC_IS_OK(createCGroup(cGroupConfigInfo))) {
        this->mCGroupMapping[cGroupConfigInfo->mCgroupID] = cGroupConfigInfo;
    }
}

void TargetRegistry::addMpamGroupMapping(MpamGroupConfigInfo* mpamGroupConfigInfo) {
    if(mpamGroupConfigInfo == nullptr) return;

    if(mpamGroupConfigInfo->mMpamGroupInfoID == -1 || mpamGroupConfigInfo->mMpamGroupName.length() == 0) {
        delete mpamGroupConfigInfo;
        return;
    }

    if(RC_IS_OK(createMpamGroup(mpamGroupConfigInfo))) {
        this->mMpamGroupMapping[mpamGroupConfigInfo->mMpamGroupInfoID] = mpamGroupConfigInfo;
    }
}

void TargetRegistry::addCacheInfoMapping(CacheInfo* cacheInfo) {
    if(cacheInfo == nullptr) return;

    if(cacheInfo->mCacheType.length() == 0 || cacheInfo->mNumCacheBlocks == -1) {
        delete cacheInfo;
        return;
    }

    this->mCacheInfoMapping[cacheInfo->mCacheType] = cacheInfo;
}

// Methods for Building CGroup Config from InitConfigs.yaml
CGroupConfigInfoBuilder::CGroupConfigInfoBuilder() {
    this->mCGroupConfigInfo = new(std::nothrow) CGroupConfigInfo;
}

ErrCode CGroupConfigInfoBuilder::setCGroupName(const std::string& name) {
    if(this->mCGroupConfigInfo == nullptr) {
        return RC_INVALID_VALUE;
    }

    this->mCGroupConfigInfo->mCgroupName = name;
    return RC_SUCCESS;
}

ErrCode CGroupConfigInfoBuilder::setCGroupID(int32_t cGroupID) {
    if(this->mCGroupConfigInfo == nullptr) {
        return RC_INVALID_VALUE;
    }

    this->mCGroupConfigInfo->mCgroupID = cGroupID;
    return RC_SUCCESS;
}

ErrCode CGroupConfigInfoBuilder::setCreationNeeded(int8_t creationNeeded) {
    if(this->mCGroupConfigInfo == nullptr) {
        return RC_INVALID_VALUE;
    }

    this->mCGroupConfigInfo->mCreationNeeded = creationNeeded;
    return RC_SUCCESS;
}

ErrCode CGroupConfigInfoBuilder::setThreaded(int8_t isThreaded) {
    if(this->mCGroupConfigInfo == nullptr) {
        return RC_INVALID_VALUE;
    }

    this->mCGroupConfigInfo->mIsThreaded = isThreaded;
    return RC_SUCCESS;
}

CGroupConfigInfo* CGroupConfigInfoBuilder::build() {
    return this->mCGroupConfigInfo;
}

MpamGroupConfigInfoBuilder::MpamGroupConfigInfoBuilder() {
    this->mMpamGroupInfo = new(std::nothrow) MpamGroupConfigInfo;
}

ErrCode MpamGroupConfigInfoBuilder::setName(const std::string& name) {
    if(this->mMpamGroupInfo == nullptr) {
        return RC_INVALID_VALUE;
    }

    this->mMpamGroupInfo->mMpamGroupName = name;
    return RC_SUCCESS;
}

ErrCode MpamGroupConfigInfoBuilder::setLgcID(int32_t logicalID) {
    if(this->mMpamGroupInfo == nullptr) {
        return RC_INVALID_VALUE;
    }

    this->mMpamGroupInfo->mMpamGroupInfoID = logicalID;
    return RC_SUCCESS;
}

ErrCode MpamGroupConfigInfoBuilder::setPriority(int32_t priority) {
    if(this->mMpamGroupInfo == nullptr) {
        return RC_INVALID_VALUE;
    }

    this->mMpamGroupInfo->mPriority = priority;
    return RC_SUCCESS;
}

MpamGroupConfigInfo* MpamGroupConfigInfoBuilder::build() {
    return this->mMpamGroupInfo;
}

CacheInfoBuilder::CacheInfoBuilder() {
    this->mCacheInfo = new(std::nothrow) CacheInfo;
}

ErrCode CacheInfoBuilder::setType(const std::string& type) {
    if(this->mCacheInfo == nullptr) {
        return RC_INVALID_VALUE;
    }

    this->mCacheInfo->mCacheType = type;
    return RC_SUCCESS;
}

ErrCode CacheInfoBuilder::setNumBlocks(int32_t numBlocks) {
    if(this->mCacheInfo == nullptr) {
        return RC_INVALID_VALUE;
    }

    this->mCacheInfo->mNumCacheBlocks = numBlocks;
    return RC_SUCCESS;
}

ErrCode CacheInfoBuilder::setPriorityAware(int8_t isPriorityAware) {
    if(this->mCacheInfo == nullptr) {
        return RC_INVALID_VALUE;
    }

    this->mCacheInfo->mPriorityAware = isPriorityAware;
    return RC_SUCCESS;
}

CacheInfo* CacheInfoBuilder::build() {
    return this->mCacheInfo;
}
