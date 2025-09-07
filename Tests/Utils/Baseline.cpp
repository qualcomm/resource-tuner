// Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
// SPDX-License-Identifier: BSD-3-Clause-Clear

#include "TestBaseline.h"
#include "TestUtils.h"

void TestBaseline::parseTestConfigYamlNode(const YAML::Node& item) {
    int8_t isConfigForCurrentTarget = false;
    // Check if there exists a Target Config for this particular target in the Common Configs.
    // Skip this check if the BU has provided their own Target Configs
    if(isList(item[TARGET_NAME_LIST])) {
        for(const auto& targetNameInfo : item[TARGET_NAME_LIST]) {
            std::string name = safeExtract<std::string>(targetNameInfo, "");

            if(name == "*" || name == ResourceTunerSettings::targetConfigs.targetName) {
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
                    this->mLogicalToPhysicalClusterMapping[logicalI] = physicalID;
                }
            }
        }

        this->mTotalClusterCount = safeExtract<int32_t>(clusterInfo[NUM_CLUSERS], 0);
        return;
    }
}

ErrCode TestBaseline::fetchBaseline() {
    YAML::Node result;
    ErrCode rc = YamlParser::parse(filePath, result);

    if(RC_IS_OK(rc)) {
        if(result[TEST_ROOT].IsDefined() && result[TEST_ROOT].IsSequence()) {
            for(const auto& testConfig : result[TEST_ROOT]) {
                try {
                    parseTestConfigYamlNode(testConfig);
                } catch(const std::invalid_argument& e) {
                    LOGE("RESTUNE_TARGET_PROCESSOR", "Error parsing Test Config: " + std::string(e.what()));
                }
            }
        }
    }

    return rc;
}

int32_t TestBaseline::getExpectedClusterCount() {
    return this->mTotalClusterCount;
}
