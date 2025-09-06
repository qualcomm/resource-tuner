// Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
// SPDX-License-Identifier: BSD-3-Clause-Clear

#include "ConfigProcessor.h"

void ConfigProcessor::parseResourceConfigYamlNode(const YAML::Node& item, int8_t isBuSpecified) {
    ErrCode rc = RC_SUCCESS;
    ResourceConfigInfoBuilder resourceConfigInfoBuilder;
    NodeExtractionStatus status;

    // No Defaults Available, a Resource with Invalid ResType is considered Malformed
    if(RC_IS_OK(rc)) {
        rc = resourceConfigInfoBuilder.setResType(
            safeExtract<std::string>(item[RESOURCE_CONFIGS_ELEM_RESOURCE_TYPE], "0")
        );
    }

    // No Defaults Available, a Resource with Invalid OpId is considered Malformed
    if(RC_IS_OK(rc)) {
        rc = resourceConfigInfoBuilder.setResID(
            safeExtract<std::string>(item[RESOURCE_CONFIGS_ELEM_RESOURCE_ID], "0")
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
        // Invalid Resource
        resourceConfigInfoBuilder.setResType("0");
    }

    ResourceRegistry::getInstance()->registerResource(resourceConfigInfoBuilder.build(), isBuSpecified);
}

void ConfigProcessor::parsePropertiesConfigYamlNode(const YAML::Node& item) {
    std::string propKey = safeExtract<std::string>(item[PROP_NAME], "");
    std::string propVal = safeExtract<std::string>(item[PROP_VALUE], "");

    if(propKey.length() > 0 || propVal.length() > 0) {
        PropertiesRegistry::getInstance()->createProperty(propKey, propVal);
    }
}

void ConfigProcessor::parseTargetConfigYamlNode(const YAML::Node& item) {
    int8_t isConfigForCurrentTarget = false;
    // Check if there exists a Target Config for this particular target in the Common Configs.
    // Skip this check if the BU has provided their own Target Configs
    if(isList(item[TARGET_NAME_LIST])) {
        for(const auto& targetNameInfo : item[TARGET_NAME_LIST]) {
            std::string name = safeExtract<std::string>(targetNameInfo, "");

            if(name == ResourceTunerSettings::targetConfigs.targetName) {
                isConfigForCurrentTarget = true;
            }
        }
    }

    if(isConfigForCurrentTarget) {
        if(isList(item[TARGET_CLUSTER_INFO])) {
            for(const auto& clusterInfo : item[TARGET_CLUSTER_INFO]) {
                int32_t logicalID = safeExtract<int32_t>(clusterInfo[TARGET_CLUSTER_INFO_LOGICAL_ID], -1);
                int32_t physicalID = safeExtract<int32_t>(clusterInfo[TARGET_CLUSTER_INFO_PHYSICAL_ID], -1);

                if(logicalID != -1 && physicalID != -1) {
                    TargetRegistry::getInstance()->addClusterMapping(logicalID, physicalID);
                }
            }
        }

        if(isList(item[TARGET_CLUSTER_SPREAD])) {
            for(const auto& clusterSpread : item[TARGET_CLUSTER_SPREAD]) {
                int32_t physicalID = safeExtract<int32_t>(clusterSpread[TARGET_CLUSTER_INFO_PHYSICAL_ID], -1);
                int32_t numCores = safeExtract<int32_t>(clusterSpread[TARGET_PER_CLUSTER_CORE_COUNT], -1);

                if(physicalID != -1 && numCores != -1) {
                    TargetRegistry::getInstance()->addClusterSpreadInfo(physicalID, numCores);
                }
            }
        }
    }
}

void ConfigProcessor::parseInitConfigYamlNode(const YAML::Node& item) {
    if(isList(item[INIT_CONFIGS_CGROUPS_LIST])) {
        for(const auto& cGroupConfig : item[INIT_CONFIGS_CGROUPS_LIST]) {
            ErrCode rc = RC_SUCCESS;
            CGroupConfigInfoBuilder cGroupConfigBuilder;

            if(RC_IS_OK(rc)) {
                rc = cGroupConfigBuilder.setCGroupName(
                    safeExtract<std::string>(cGroupConfig[INIT_CONFIGS_CGROUP_NAME], "")
                );
            }

            if(RC_IS_OK(rc)) {
                rc = cGroupConfigBuilder.setCGroupID(
                    safeExtract<int32_t>(cGroupConfig[INIT_CONFIGS_CGROUP_IDENTIFIER], -1)
                );
            }

            if(RC_IS_OK(rc)) {
                // By default, the Cgroup is expected to already exist.
                rc = cGroupConfigBuilder.setCreationNeeded(
                    (int8_t)(safeExtract<bool>(cGroupConfig[INIT_CONFIGS_CGROUP_CREATION], false))
                );
            }

            if(RC_IS_OK(rc)) {
                rc = cGroupConfigBuilder.setThreaded(
                    (int8_t)(safeExtract<bool>(cGroupConfig[INIT_CONFIGS_CGROUP_THREADED], false))
                );
            }

            if(RC_IS_NOTOK(rc)) {
                // Set the ID to -1, so that the Cgroup is not added and is cleaned up
                cGroupConfigBuilder.setCGroupID(-1);
            }

            TargetRegistry::getInstance()->addCGroupMapping(cGroupConfigBuilder.build());
        }
    }

    if(isList(item[INIT_CONFIGS_MPAM_GROUPS_LIST])) {
        for(const auto& mpamGroupConfig : item[INIT_CONFIGS_MPAM_GROUPS_LIST]) {
            ErrCode rc = RC_SUCCESS;
            MpamGroupConfigInfoBuilder mpamGroupConfigBuilder;

            if(RC_IS_OK(rc)) {
                rc = mpamGroupConfigBuilder.setName(
                    safeExtract<std::string>(mpamGroupConfig[INIT_CONFIGS_MPAM_GROUP_NAME], "")
                );
            }

            if(RC_IS_OK(rc)) {
                rc = mpamGroupConfigBuilder.setLgcID(
                    safeExtract<int32_t>(mpamGroupConfig[INIT_CONFIGS_MPAM_GROUP_ID], -1)
                );
            }

            if(RC_IS_OK(rc)) {
                rc = mpamGroupConfigBuilder.setPriority(
                    safeExtract<int32_t>(mpamGroupConfig[INIT_CONFIGS_MPAM_GROUP_PRIORITY], 0)
                );
            }

            if(RC_IS_NOTOK(rc)) {
                // Set the ID to -1, so that the Cgroup is not added and is cleaned up
                mpamGroupConfigBuilder.setLgcID(-1);
            }

            TargetRegistry::getInstance()->addMpamGroupMapping(mpamGroupConfigBuilder.build());
        }
    }

    if(isList(item[INIT_CONFIGS_CACHE_INFO_LIST])) {
        for(const auto& cacheConfig : item[INIT_CONFIGS_CACHE_INFO_LIST]) {
            ErrCode rc = RC_SUCCESS;
            CacheInfoBuilder cacheInfoBuilder;

            if(RC_IS_OK(rc)) {
                rc = cacheInfoBuilder.setType(
                    safeExtract<std::string>(cacheConfig[INIT_CONFIGS_CACHE_INFO_CACHE_TYPE], "")
                );
            }

            if(RC_IS_OK(rc)) {
                rc = cacheInfoBuilder.setNumBlocks(
                    safeExtract<int32_t>(cacheConfig[INIT_CONFIGS_CACHE_INFO_CACHE_BLOCK_COUNT], -1)
                );
            }

            if(RC_IS_OK(rc)) {
                rc = cacheInfoBuilder.setPriorityAware(
                    (int8_t)safeExtract<int8_t>(cacheConfig[INIT_CONFIGS_CACHE_INFO_CACHE_PRIORITY_AWARE], false)
                );
            }

            if(RC_IS_NOTOK(rc)) {
                cacheInfoBuilder.setType("");
                cacheInfoBuilder.setNumBlocks(-1);
            }

            TargetRegistry::getInstance()->addCacheInfoMapping(cacheInfoBuilder.build());
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

ErrCode ConfigProcessor::parsePropertiesConfigs(const std::string& filePath) {
    YAML::Node result;
    ErrCode rc = YamlParser::parse(filePath, result);

    if(RC_IS_OK(rc)) {
        if(result[PROPERTIES_CONFIG_ROOT].IsDefined() && result[PROPERTIES_CONFIG_ROOT].IsSequence()) {
            for(const auto& propertyConfig : result[PROPERTIES_CONFIG_ROOT]) {
                try {
                    parsePropertiesConfigYamlNode(propertyConfig);
                } catch(const std::invalid_argument& e) {
                    LOGE("RESTUNE_CONFIG_PROCESSOR", "Error parsing Properties Config: " + std::string(e.what()));
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
                    LOGE("RESTUNE_CONFIG_PROCESSOR", "Error parsing Init Config: " + std::string(e.what()));
                }
            }
        }
    }

    return rc;
}

ErrCode ConfigProcessor::parse(ConfigType configType, const std::string& filePath, int8_t isBuSpecified) {
    ErrCode rc = RC_SUCCESS;

    switch(configType) {
        case ConfigType::RESOURCE_CONFIG: {
            rc = this->parseResourceConfigs(filePath, isBuSpecified);
            break;
        }
        case ConfigType::PROPERTIES_CONFIG: {
            rc = this->parsePropertiesConfigs(filePath);
            break;
        }
        case ConfigType::TARGET_CONFIG: {
            rc = this->parseTargetConfigs(filePath);
            break;
        }
        case ConfigType::INIT_CONFIG: {
            rc = this->parseInitConfigs(filePath);
            break;
        }
        default: {
            rc = RC_BAD_ARG;
            break;
        }
    }
    return rc;
}
