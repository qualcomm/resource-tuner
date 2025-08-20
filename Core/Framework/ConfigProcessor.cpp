// Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
// SPDX-License-Identifier: BSD-3-Clause-Clear

#include "ConfigProcessor.h"

void ConfigProcessor::parseResourceConfigYamlNode(const YAML::Node& item, int8_t isBuSpecified) {
    ErrCode rc = RC_SUCCESS;
    ResourceConfigInfoBuilder resourceConfigInfoBuilder;
    NodeExtractionStatus status;

    // No Defaults Available, a Resource with Invalid OpType is considered Malformed
    if(RC_IS_OK(rc)) {
        rc = resourceConfigInfoBuilder.setResType(
            safeExtract<std::string>(item[RESOURCE_CONFIGS_ELEM_RESOURCE_TYPE], "-1")
        );
    }

    // No Defaults Available, a Resource with Invalid OpId is considered Malformed
    if(RC_IS_OK(rc)) {
        rc = resourceConfigInfoBuilder.setResID(
            safeExtract<std::string>(item[RESOURCE_CONFIGS_ELEM_RESOURCE_ID], "-1")
        );
    }

    // Defaults to false
    if(RC_IS_OK(rc)) {
        rc = resourceConfigInfoBuilder.setSupported(
            safeExtract<bool>(item[RESOURCE_CONFIGS_ELEM_SUPPORTED], false, status)
        );

        if(status == NodeExtractionStatus::NODE_PRESENT_VALUE_INVALID) {
            rc = RC_INVALID_VALUE;
        }
    }

    // Defaults to an empty string
    if(RC_IS_OK(rc)) {
        rc = resourceConfigInfoBuilder.setName(
            safeExtract<std::string>(item[RESOURCE_CONFIGS_ELEM_RESOURCENAME], "", status)
        );

        if(status == NodeExtractionStatus::NODE_PRESENT_VALUE_INVALID) {
            rc = RC_INVALID_VALUE;
        }
    }

    // Defaults to an empty string
    if(RC_IS_OK(rc)) {
        rc = resourceConfigInfoBuilder.setPath(
            safeExtract<std::string>(item[RESOURCE_CONFIGS_ELEM_RESOURCEPATH], "", status)
        );

        if(status == NodeExtractionStatus::NODE_PRESENT_VALUE_INVALID) {
            rc = RC_INVALID_VALUE;
        }
    }

    if(RC_IS_OK(rc)) {
        std::string defaultValue = AuxRoutines::readFromFile(
            safeExtract<std::string>(item[RESOURCE_CONFIGS_ELEM_RESOURCEPATH], "", status)
        );

        if(status == NodeExtractionStatus::NODE_PRESENT_VALUE_INVALID) {
            rc = RC_INVALID_VALUE;
        }

        // Defaults to 0
        if(RC_IS_OK(rc)) {
            rc = resourceConfigInfoBuilder.setDefaultValue(defaultValue);
        }
    }

    // No Defaults Available, a Resource with Invalid HT is considered Malformed
    if(RC_IS_OK(rc)) {
        rc = resourceConfigInfoBuilder.setHighThreshold(
            safeExtract<int32_t>(item[RESOURCE_CONFIGS_ELEM_HIGHTHRESHOLD], -1, status)
        );

        if(status == NodeExtractionStatus::NODE_PRESENT_VALUE_INVALID) {
            rc = RC_INVALID_VALUE;
        }
    }

    // No Defaults Available, a Resource with Invalid LT is considered Malformed
    if(RC_IS_OK(rc)) {
        rc = resourceConfigInfoBuilder.setLowThreshold(
            safeExtract<int32_t>(item[RESOURCE_CONFIGS_ELEM_LOWTHRESHOLD], -1, status)
        );

        if(status == NodeExtractionStatus::NODE_PRESENT_VALUE_INVALID) {
            rc = RC_INVALID_VALUE;
        }
    }

    // Default to a Value of Third Party
    if(RC_IS_OK(rc)) {
        rc = resourceConfigInfoBuilder.setPermissions(
            safeExtract<std::string>(item[RESOURCE_CONFIGS_ELEM_PERMISSIONS], "", status)
        );

        if(status == NodeExtractionStatus::NODE_PRESENT_VALUE_INVALID) {
            rc = RC_INVALID_VALUE;
        }
    }

    // Defaults to a Value of DISPLAY_ON
    if(RC_IS_OK(rc)) {
        if(isList(item[RESOURCE_CONFIGS_ELEM_MODES])) {
            for(const auto& mode : item[RESOURCE_CONFIGS_ELEM_MODES]) {
                if(RC_IS_OK(rc)) {
                    rc = resourceConfigInfoBuilder.setModes(
                        safeExtract<std::string>(mode, "", status)
                    );

                    if(status == NodeExtractionStatus::NODE_PRESENT_VALUE_INVALID) {
                        rc = RC_INVALID_VALUE;
                        break;
                    }
                } else {
                    break;
                }
            }
        } else if(!item[RESOURCE_CONFIGS_ELEM_MODES].IsDefined()) {
            rc = resourceConfigInfoBuilder.setModes("display_on");
        } else {
            rc = RC_INVALID_VALUE;
        }
    }

    // Defaults to LAZY_APPLY
    if(RC_IS_OK(rc)) {
        rc = resourceConfigInfoBuilder.setPolicy(
            safeExtract<std::string>(item[RESOURCE_CONFIGS_ELEM_POLICY], "", status)
        );

        if(status == NodeExtractionStatus::NODE_PRESENT_VALUE_INVALID) {
            rc = RC_INVALID_VALUE;
        }
    }

    // Defaults to APPLY_GLOBAL
    if(RC_IS_OK(rc)) {
        rc = resourceConfigInfoBuilder.setApplyType(
            safeExtract<std::string>(item[RESOURCE_CONFIGS_APPLY_TYPE], "", status)
        );

        if(status == NodeExtractionStatus::NODE_PRESENT_VALUE_INVALID) {
            rc = RC_INVALID_VALUE;
        }
    }

    if(RC_IS_NOTOK(rc)) {
        resourceConfigInfoBuilder.setResID("-1");
    }

    ResourceRegistry::getInstance()->registerResource(resourceConfigInfoBuilder.build(), isBuSpecified);
}

void ConfigProcessor::parseTargetConfigYamlNode(const YAML::Node& item) {
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

void ConfigProcessor::parseInitConfigYamlNode(const YAML::Node& item) {
    if(isList(item[INIT_CONFIGS_CGROUPS_LIST])) {
        for(const auto& cGroupConfig : item[INIT_CONFIGS_CGROUPS_LIST]) {
            CGroupConfigInfoBuilder cGroupConfigBuilder;
            cGroupConfigBuilder.setCGroupName(safeExtract<std::string>(cGroupConfig[INIT_CONFIGS_CGROUP_NAME], ""));
            cGroupConfigBuilder.setCGroupID((int8_t)(safeExtract<int8_t>(cGroupConfig[INIT_CONFIGS_CGROUP_IDENTIFIER], -1)));
            cGroupConfigBuilder.setThreaded((int8_t)(safeExtract<bool>(cGroupConfig[INIT_CONFIGS_CGROUP_THREADED], false)));

            TargetRegistry::getInstance()->addCGroupMapping(cGroupConfigBuilder.build());
        }
    }
}

ErrCode ConfigProcessor::parseResourceConfigs(const std::string& filePath, int8_t isBuSpecified) {
    YAML::Node result;
    ErrCode rc = YamlParser::parse(filePath, result);

    if(RC_IS_OK(rc)) {
        if(result[RESOURCE_CONFIGS_ROOT].IsDefined() && result[RESOURCE_CONFIGS_ROOT].IsSequence()) {
            int32_t resourceCount = result[RESOURCE_CONFIGS_ROOT].size();

            for(int32_t i = 0; i < result[RESOURCE_CONFIGS_ROOT].size(); i++) {
                YAML::Node resourceConfig = result[RESOURCE_CONFIGS_ROOT][i];
                try {
                    LOGI("RESTUNE_RESOURCE_PROCESSOR", "Parsing Resource Config at index = " + std::to_string(i));
                    parseResourceConfigYamlNode(resourceConfig, isBuSpecified);
                } catch(const std::invalid_argument& e) {
                    LOGE("RESTUNE_RESOURCE_PROCESSOR", "Error parsing Resource Config: " + std::string(e.what()));
                } catch(const std::bad_alloc& e) {
                    LOGE("RESTUNE_RESOURCE_PROCESSOR", "Error parsing Resource Config: " + std::string(e.what()));
                }
            }
        }
    }

    return rc;
}

ErrCode ConfigProcessor::parseTargetConfigs(const std::string& filePath) {
    YAML::Node result;
    ErrCode rc = YamlParser::parse(filePath, result);

    if(RC_IS_OK(rc)) {
        if(result[TARGET_CONFIGS_ROOT].IsDefined() && result[TARGET_CONFIGS_ROOT].IsSequence()) {
            for(const auto& targetConfig : result[TARGET_CONFIGS_ROOT]) {
                try {
                    parseTargetConfigYamlNode(targetConfig);
                } catch(const std::invalid_argument& e) {
                    LOGE("RESTUNE_TARGET_PROCESSOR", "Error parsing Target Config: " + std::string(e.what()));
                }
            }
        }
    }

    return rc;
}

ErrCode ConfigProcessor::parseInitConfigs(const std::string& filePath) {
    YAML::Node result;
    ErrCode rc = YamlParser::parse(filePath, result);

    if(RC_IS_OK(rc)) {
        if(result[INIT_CONFIGS_ROOT].IsDefined() && result[INIT_CONFIGS_ROOT].IsSequence()) {
            for(const auto& targetConfig : result[INIT_CONFIGS_ROOT]) {
                try {
                    parseInitConfigYamlNode(targetConfig);
                } catch(const std::invalid_argument& e) {
                    LOGE("RESTUNE_TARGET_PROCESSOR", "Error parsing Init Config: " + std::string(e.what()));
                }
            }
        }
    }

    return rc;
}
