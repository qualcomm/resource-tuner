// Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
// SPDX-License-Identifier: BSD-3-Clause-Clear

#ifndef TEST_BASELINE_H
#define TEST_BASELINE_H

#include <unordered_map>

#include "YamlParser.h"
#include "ResourceTunerSettings.h"

#define TEST_ROOT "TestConfigs"
#define TARGET_NAME_LIST "TargetName"
#define CLUSTER_EXPECTATIONS "ClusterExpectations"
#define TARGET_CLUSTER_INFO_LOGICAL_ID "LgcId"
#define TARGET_CLUSTER_INFO_PHYSICAL_ID "PhyId"
#define NUM_CLUSERS "NumClusters"
#define NUM_CORES "NumCores"

const static std::string baselineYamlFilePath =
                         "/etc/resource-tuner/custom/Baseline.yaml";

class TestBaseline {
private:
    std::unordered_map<int32_t, int32_t> mLogicalToPhysicalClusterMapping;
    int32_t mTotalClusterCount;
    int32_t mTotalCoreCount;

    int8_t parseTestConfigYamlNode(const YAML::Node& item) {
        int8_t isConfigForCurrentTarget = false;
        // Check if there exists a Target Config for this particular target in the Common Configs.
        // Skip this check if the BU has provided their own Target Configs
        std::string currTargetName = AuxRoutines::readFromFile("/sys/devices/soc0/machine");
        if(isList(item[TARGET_NAME_LIST])) {
            for(const auto& targetNameInfo : item[TARGET_NAME_LIST]) {
                std::string name = safeExtract<std::string>(targetNameInfo, "");

                if(name == "*" || name == currTargetName) {
                    isConfigForCurrentTarget = true;
                }
            }
        }

        if(isConfigForCurrentTarget) {
            if(isList(item[CLUSTER_EXPECTATIONS])) {
                for(const auto& clusterInfo : item[CLUSTER_EXPECTATIONS]) {
                    int32_t logicalID = safeExtract<int32_t>(clusterInfo[TARGET_CLUSTER_INFO_LOGICAL_ID], -1);
                    int32_t physicalID = safeExtract<int32_t>(clusterInfo[TARGET_CLUSTER_INFO_PHYSICAL_ID], -1);

                    if(logicalID != -1 && physicalID != -1) {
                        this->mLogicalToPhysicalClusterMapping[logicalID] = physicalID;
                    }
                }
            }

            this->mTotalClusterCount = safeExtract<int32_t>(item[NUM_CLUSERS], 0);
            this->mTotalCoreCount = safeExtract<int32_t>(item[NUM_CORES], 0);
            return true;
        }

        return false;
    }

public:
    ErrCode fetchBaseline() {
        YAML::Node result;
        ErrCode rc = YamlParser::parse(baselineYamlFilePath, result);

        if(RC_IS_OK(rc)) {
            if(result[TEST_ROOT].IsDefined() && result[TEST_ROOT].IsSequence()) {
                for(const auto& testConfig : result[TEST_ROOT]) {
                    try {
                        if(parseTestConfigYamlNode(testConfig)) {
                            return RC_SUCCESS;
                        }
                    } catch(const std::invalid_argument& e) {
                        LOGE("RESTUNE_TARGET_PROCESSOR", "Error parsing Test Config: " + std::string(e.what()));
                    }
                }
            }
        }

        return rc;
    }

    int32_t getExpectedClusterCount() {
        return this->mTotalClusterCount;
    }

    int32_t getExpectedCoreCount() {
        return this->mTotalCoreCount;
    }

    int32_t getExpectedPhysicalCluster(int32_t logicalID) {
        if(this->mLogicalToPhysicalClusterMapping.find(logicalID) ==
           this->mLogicalToPhysicalClusterMapping.end()) {
            return -1;
        }

        return this->mLogicalToPhysicalClusterMapping[logicalID];
    }
};

#endif
