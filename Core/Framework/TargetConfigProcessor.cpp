// Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
// SPDX-License-Identifier: BSD-3-Clause-Clear

#include "TargetConfigProcessor.h"

TargetConfigProcessor::TargetConfigProcessor(const std::string& targetConfigFile, const std::string& initConfigFile) {
    if(targetConfigFile.length() == 0) {
        // No Custom Target Config File Specified
        this->mTargetConfigYamlFilePath = TARGET_CONFIGS_FILE;
    } else {
        this->mTargetConfigYamlFilePath = targetConfigFile;
    }

    if(initConfigFile.length() == 0) {
        // No Custom Target Config File Specified
        this->mInitConfigYamlFilePath = INIT_CONFIGS_FILE;
    } else {
        this->mInitConfigYamlFilePath = initConfigFile;
    }
}

ErrCode TargetConfigProcessor::parseTargetConfigs() {
    const std::string fTargetConfigFileName(this->mTargetConfigYamlFilePath);
    const std::string fInitConfigFileName(this->mInitConfigYamlFilePath);

    YAML::Node result;
    ErrCode rc = YamlParser::parse(fTargetConfigFileName, result);

    if(RC_IS_OK(rc)) {
        if(result[TARGET_CONFIGS_ROOT].IsDefined() && result[TARGET_CONFIGS_ROOT].IsSequence()) {
            for(const auto& targetConfig : result[TARGET_CONFIGS_ROOT]) {
                try {
                    parseTargetConfig(targetConfig);
                } catch(const std::invalid_argument& e) {
                    LOGE("RTN_TARGET_PROCESSOR", "Error parsing Target Config: " + std::string(e.what()));
                }
            }
        }
    }

    rc = YamlParser::parse(fInitConfigFileName, result);
    if(RC_IS_OK(rc)) {
        if(result[INIT_CONFIGS_ROOT].IsDefined() && result[INIT_CONFIGS_ROOT].IsSequence()) {
            for(const auto& targetConfig : result[INIT_CONFIGS_ROOT]) {
                try {
                    parseInitConfig(targetConfig);
                } catch(const std::invalid_argument& e) {
                    LOGE("RTN_TARGET_PROCESSOR", "Error parsing Init Config: " + std::string(e.what()));
                }
            }
        }
    }

    return rc;
}

void TargetConfigProcessor::parseTargetConfig(const YAML::Node& item) {
    TargetRegistry::getInstance()->setTargetName(
        safeExtract<std::string>(item[TARGET_NAME])
    );

    TargetRegistry::getInstance()->setTotalCoreCount(
        (uint8_t)(safeExtract<uint8_t>(item[TARGET_TOTAL_CORE_COUNT]))
    );

    if(isList(item[TARGET_CLUSTER_INFO])) {
        for(const auto& clusterInfo : item[TARGET_CLUSTER_INFO]) {
            int8_t id;
            std::string clusterType;

            id = static_cast<int8_t>(safeExtract<int32_t>(clusterInfo[TARGET_CONFIGS_ID]));
            clusterType = safeExtract<std::string>(clusterInfo[TARGET_CONFIGS_TYPE]);

            TargetRegistry::getInstance()->addMapping(clusterType, id);
        }
    }

    if(isList(item[TARGET_CLUSTER_SPREAD])) {
        for(const auto& clusterSpread : item[TARGET_CLUSTER_SPREAD]) {
            int8_t id;
            int32_t numCores;

            id = static_cast<int8_t>(safeExtract<int32_t>(clusterSpread[TARGET_CONFIGS_ID]));
            numCores = safeExtract<int32_t>(clusterSpread[TARGET_PER_CLUSTER_CORE_COUNT]);

            TargetRegistry::getInstance()->addClusterSpreadInfo(id, numCores);
        }
    }
}

void TargetConfigProcessor::parseInitConfig(const YAML::Node& item) {
    if(isList(item[INIT_CONFIGS_CGROUPS_LIST])) {
        for(const auto& cGroupConfig : item[INIT_CONFIGS_CGROUPS_LIST]) {
            CGroupConfigInfoBuilder cGroupConfigBuilder;
            cGroupConfigBuilder.setCGroupName(safeExtract<std::string>(cGroupConfig[INIT_CONFIGS_CGROUP_NAME]));
            cGroupConfigBuilder.setCGroupID((int8_t)(safeExtract<int8_t>(cGroupConfig[INIT_CONFIGS_CGROUP_IDENTIFIER])));
            cGroupConfigBuilder.setThreaded((int8_t)(safeExtract<bool>(cGroupConfig[INIT_CONFIGS_CGROUP_THREADED])));

            TargetRegistry::getInstance()->addCGroupMapping(cGroupConfigBuilder.build());
        }
    }
}
