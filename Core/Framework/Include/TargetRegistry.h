// Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
// SPDX-License-Identifier: BSD-3-Clause-Clear

#ifndef TARGET_REGISTRY_H
#define TARGET_REGISTRY_H

#include <memory>
#include <unordered_map>
#include <vector>
#include <dirent.h>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <cstring>
#include <cstdlib>
#include <sys/stat.h>
#include <memory>

#include "ResourceTunerSettings.h"
#include "ErrCodes.h"
#include "Logger.h"

#define CLUSTER_TYPE_COUNT 4

#define POLICY_DIR_PATH "/sys/devices/system/cpu/cpufreq/"
#define ONLINE_CPU_FILE_PATH "/sys/devices/system/cpu/online"

typedef struct {
    std::string mCgroupName;
    int8_t mCgroupID;
    int8_t isThreaded;
    std::unordered_map<std::string, std::string>* mDefaultValues;
} CGroupConfigInfo;

class TargetRegistry {
private:
    static std::shared_ptr<TargetRegistry> targetRegistryInstance;

    std::string mTargetName;
    uint8_t mTotalCoreCount;
    std::unordered_map<std::string, int8_t> mClusterTypeToPhysicalSlotMapping;
    std::unordered_map<std::string, std::vector<int8_t>> mClusterTypeToPhysicalCores;
    std::unordered_map<int8_t, std::vector<int8_t>> mPhysicalMapping;
    std::vector<std::pair<int8_t, int32_t>> mClusterSpreadInfo;

    const std::unordered_map<int8_t, std::string> logicalClustersAPIConvention = {
        {0, "big"},
        {1, "little"},
        {2, "prime"},
        {3, "titanium"}
    };

    std::unordered_map<int8_t, CGroupConfigInfo*> mCGroupMapping;

    TargetRegistry();

    int8_t readPhysicalMapping();

public:
    ~TargetRegistry();

    // Add a new Cluste Type to Physical ID mapping to mClusterTypeToPhysicalSlotMapping
    int8_t addMapping(const std::string& clusterName, int8_t physicalClusterId);

    int8_t addClusterSpreadInfo(int8_t physicalClusterId, int32_t coreCount);

    void setTargetName(const std::string& targetName);

    void setTotalCoreCount(uint8_t totalCoreCount);

    void addCGroupMapping(CGroupConfigInfo* cGroupConfigInfo);

    void getCGroupConfigs(std::vector<CGroupConfigInfo*>& cGroupNames);

    int32_t getCreatedCGroupsCount();

    CGroupConfigInfo* getCGroupConfig(int8_t cGroupID);

    /**
    * @brief Called by the Verifier to get the physical core ID corresponding to the Logical Core ID value.
    * @details This routine performs Logical to Physical Core Translation.
    * @param logicalClusterId The Logical Cluster ID, passed via the mOpInfo field (part of the Resource struct)
    *                         when issuing a tuneResources API call.
    * @param logicalCoreId The Logical Core ID, passed via the mOpInfo field (part of the Resource struct)
    *                         when issuing a tuneResources API call.
    * @return: int32_t
    *              A Non-Negative Integer, representing the corresponding physical Core ID.
    *              -1: otherwise
    */
    int32_t getPhysicalCoreId(int32_t logicalClusterId, int32_t logicalCoreId) const;

    /**
    * @brief Called by the Verifier to get the physical Cluster ID corresponding to the Logical Cluster ID value.
    * @details This routine performs Logical to Physical Cluster Translation.
    * @param logicalClusterId The Logical Cluster ID, passed via the mOpInfo field (part of the Resource struct)
    *                         when issuing a tuneResources API call.
    * @return: int32_t
    *              A Non-Negative Integer, representing the corresponding physical Cluster ID.
    *              -1: otherwise
    */
    int32_t getPhysicalClusterId(int32_t logicalClusterId) const;

    /**
    * @brief Called during Server Init, to read and Parse the Logical To Physical Core / Cluster Mappings.
    * @details This routine will extract the physical Core IDs and the list of CPU cores part of each Physical Cluster
    *          This data will be used to perform Logical to Physical Translation for each incoming tuneResources Request
    *          later on, if it contains any Resource which has ApplyType set to Core.
    *
    *          Note: This function tries to use different strategies to get the Core / Cluster Enumeration and Mapping data:
    *          - If the BU has provided a mapping file, it will be used. This mapping file should contain data specifying
    *            the total Number of Cores / Clusters, mapping of Cluster Type to Physical IDs and Number of Cores
    *            in each Cluster.
    *          - If no such file is provided, we try to generate the mappings dynamically by reading the policy directories
    *            located in /sys/devices/system/cpu/cpufreq/
    *          - If even the above strategy fails, we default to using Simple Heuristic based approach, where we divided
    *            the number of Online CPU cores as evenly as possible among the Cluster Types.
    *          - If even the above strategy fails, the function will return an Error.
    * @return: int32_t
    *               1 If the Data was successfully Read and Parsed.
    *              -1: otherwise
    */
    ErrCode readPhysicalCoreClusterInfo();

    // Utility to display the Parsed Mapping of Logical Cluster to Physical Cluster
    // Along with the list of cores, part of each Cluster.
    void displayLogicalToPhysicalMapping();

    static std::shared_ptr<TargetRegistry> getInstance() {
        if(targetRegistryInstance == nullptr) {
            targetRegistryInstance = std::shared_ptr<TargetRegistry>(new TargetRegistry());
        }
        return targetRegistryInstance;
    }
};

class CGroupConfigInfoBuilder {
private:
    CGroupConfigInfo* mCGroupConfigInfo;

public:
    CGroupConfigInfoBuilder();

    CGroupConfigInfoBuilder* setCGroupName(const std::string& cGroupName);
    CGroupConfigInfoBuilder* setCGroupID(int8_t cGroupIdentifier);
    CGroupConfigInfoBuilder* setThreaded(int8_t isThreaded);

    CGroupConfigInfo* build();
};

#endif
