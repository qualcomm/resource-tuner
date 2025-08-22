// Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
// SPDX-License-Identifier: BSD-3-Clause-Clear

#include "ConfigProcessor.h"

void ConfigProcessor::parseSignalConfigYamlNode(const YAML::Node& item, int8_t isBuSpecified) {
    SignalInfoBuilder signalInfoBuilder;
    ErrCode rc = RC_SUCCESS;
    NodeExtractionStatus status;

    // No defaults applicable
    if(RC_IS_OK(rc)) {
        rc = signalInfoBuilder.setSignalID(
            safeExtract<std::string>(item[SIGNAL_SIGID], "0")
        );
    }

    if(RC_IS_OK(rc)) {
        // No defaults applicable
        rc = signalInfoBuilder.setSignalCategory(
            safeExtract<std::string>(item[SIGNAL_CATEGORY], "0")
        );
    }

    if(RC_IS_OK(rc)) {
        // defaults to empty string
        rc = signalInfoBuilder.setName(
            safeExtract<std::string>(item[SIGNAL_NAME], "", status)
        );

        if(status == NodeExtractionStatus::NODE_PRESENT_VALUE_INVALID) {
            rc = RC_INVALID_VALUE;
        }
    }

    if(RC_IS_OK(rc)) {
        // defaults to 1 ms
        rc = signalInfoBuilder.setTimeout(
            safeExtract<int32_t>(item[SIGNAL_TIMEOUT], 1, status)
        );

        if(status == NodeExtractionStatus::NODE_PRESENT_VALUE_INVALID) {
            rc = RC_INVALID_VALUE;
        }
    }

    if(RC_IS_OK(rc)) {
        // defaults to False
        rc = signalInfoBuilder.setIsEnabled(
            safeExtract<bool>(item[SIGNAL_ENABLE], false, status)
        );

        if(status == NodeExtractionStatus::NODE_PRESENT_VALUE_INVALID) {
            rc = RC_INVALID_VALUE;
        }
    }

    if(RC_IS_OK(rc)) {
        if(isList(item[SIGNAL_PERMISSIONS])) {
            for(int32_t i = 0; i < item[SIGNAL_PERMISSIONS].size(); i++) {
                if(RC_IS_OK(rc)) {
                    rc = signalInfoBuilder.addPermission(
                        // defaults to THIRD_PARTY
                        safeExtract<std::string>(item[SIGNAL_PERMISSIONS][i], "", status)
                    );

                    if(status == NodeExtractionStatus::NODE_PRESENT_VALUE_INVALID) {
                        rc = RC_INVALID_VALUE;
                    }
                } else {
                    break;
                }
            }
        }
    }

    if(RC_IS_OK(rc)) {
        if(isList(item[SIGNAL_TARGETS_ENABLED])) {
            for(int32_t i = 0; i < item[SIGNAL_TARGETS_ENABLED].size(); i++) {
                // Defaults to empty string
                if(RC_IS_OK(rc)) {
                    rc = signalInfoBuilder.addTarget(true,
                        safeExtract<std::string>(item[SIGNAL_TARGETS_ENABLED][i], "", status)
                    );

                    if(status == NodeExtractionStatus::NODE_PRESENT_VALUE_INVALID) {
                        rc = RC_INVALID_VALUE;
                    }
                } else {
                    break;
                }
            }
        }
    }

    if(RC_IS_OK(rc)) {
        if(isList(item[SIGNAL_TARGETS_DISABLED])) {
            for(int32_t i = 0; i < item[SIGNAL_TARGETS_DISABLED].size(); i++) {
                // Defaults to empty string
                if(RC_IS_OK(rc)) {
                    rc = signalInfoBuilder.addTarget(false,
                        safeExtract<std::string>(item[SIGNAL_TARGETS_DISABLED][i], "", status)
                    );

                    if(status == NodeExtractionStatus::NODE_PRESENT_VALUE_INVALID) {
                        rc = RC_INVALID_VALUE;
                    }
                } else {
                    break;
                }
            }
        }
    }

    if(RC_IS_OK(rc)) {
        if(isList(item[SIGNAL_DERIVATIVES])) {
            for(int32_t i = 0; i < item[SIGNAL_DERIVATIVES].size(); i++) {
                // Defaults to empty string
                if(RC_IS_OK(rc)) {
                    rc = signalInfoBuilder.addDerivative(
                        safeExtract<std::string>(item[SIGNAL_DERIVATIVES][i], "", status)
                    );

                    if(status == NodeExtractionStatus::NODE_PRESENT_VALUE_INVALID) {
                        rc = RC_INVALID_VALUE;
                        break;
                    }
                } else {
                    break;
                }
            }
        }
    }

    if(RC_IS_OK(rc)) {
        if(item[SIGNAL_RESOURCES].IsDefined() && item[SIGNAL_RESOURCES].IsSequence()) {
            for(const auto& resourceConfig: item[SIGNAL_RESOURCES]) {
                if(!resourceConfig.IsMap()) {
                    rc = RC_INVALID_VALUE;
                    break;
                }

                ResourceBuilder resourceBuilder;
                std::string resCode =
                    safeExtract<std::string>(resourceConfig[SIGNAL_RESOURCE_CODE], "", status);

                if(resCode.length() == 0) {
                    rc = RC_INVALID_VALUE;
                    break;
                }

                if(status == NodeExtractionStatus::NODE_PRESENT_VALUE_INVALID) {
                    rc = RC_INVALID_VALUE;
                    break;
                }

                // No Defaults Applicable
                if(RC_IS_OK(rc)) {
                    rc = resourceBuilder.setResCode(resCode);
                } else {
                    break;
                }

                if(RC_IS_OK(rc)) {
                    rc = resourceBuilder.setOpInfo(
                        // Defaults to 0
                        safeExtract<std::string>(resourceConfig[SIGNAL_RESINFO], "0", status)
                    );

                    if(status == NodeExtractionStatus::NODE_PRESENT_VALUE_INVALID) {
                        rc = RC_INVALID_VALUE;
                        break;
                    }
                } else {
                    break;
                }

                if(RC_IS_OK(rc)) {
                    if(isList(resourceConfig[SIGNAL_VALUES])) {
                        int32_t valuesCount = resourceConfig[SIGNAL_VALUES].size();
                        rc = resourceBuilder.setNumValues(valuesCount);

                        for(int32_t i = 0; i < valuesCount; i++) {
                            // Defaults to -1
                            if(RC_IS_OK(rc)) {
                                rc = resourceBuilder.addValue(
                                    safeExtract<int32_t>(resourceConfig[SIGNAL_VALUES][i], -1, status)
                                );

                                if(status == NodeExtractionStatus::NODE_PRESENT_VALUE_INVALID) {
                                    rc = RC_INVALID_VALUE;
                                    break;
                                }
                            } else {
                                break;
                            }
                        }
                    }
                }

                if(RC_IS_OK(rc)) {
                    rc = signalInfoBuilder.addResource(resourceBuilder.build());
                } else {
                    break;
                }
            }
        }
    }

    if(RC_IS_NOTOK(rc)) {
        // Set OpId so that the Signal gets discarded by Signal Regsitry
        signalInfoBuilder.setSignalCategory("0");
    }

    SignalRegistry::getInstance()->registerSignal(signalInfoBuilder.build(), isBuSpecified);
}

void ConfigProcessor::parseExtFeatureConfigYamlNode(const YAML::Node& item) {
    ExtFeatureInfoBuilder extFeatureInfoBuilder;
    ErrCode rc = RC_SUCCESS;
    NodeExtractionStatus status;

    if(RC_IS_OK(rc)) {
        rc = extFeatureInfoBuilder.setId(
            safeExtract<std::string>(item[EXT_FEATURE_ID], "0")
        );

        if(status == NodeExtractionStatus::NODE_PRESENT_VALUE_INVALID) {
            rc = RC_INVALID_VALUE;
        }
    }

    if(RC_IS_OK(rc)) {
        rc = extFeatureInfoBuilder.setLib(
            safeExtract<std::string>(item[EXT_FEATURE_LIB], "", status)
        );

        if(status == NodeExtractionStatus::NODE_PRESENT_VALUE_INVALID) {
            rc = RC_INVALID_VALUE;
        }
    }

    if(RC_IS_OK(rc)) {
        rc = extFeatureInfoBuilder.setName(
            safeExtract<std::string>(item[EXT_FEATURE_NAME], "", status)
        );

        if(status == NodeExtractionStatus::NODE_PRESENT_VALUE_INVALID) {
            rc = RC_INVALID_VALUE;
        }
    }

    if(RC_IS_OK(rc)) {
        if(isList(item[EXT_FEATURE_SUBSCRIBER_LIST])) {
            for(int32_t i = 0; i < item[EXT_FEATURE_SUBSCRIBER_LIST].size(); i++) {
                if(RC_IS_OK(rc)) {
                    rc = extFeatureInfoBuilder.addSignalSubscribedTo(
                        (safeExtract<std::string>(item[EXT_FEATURE_SUBSCRIBER_LIST][i], "", status))
                    );

                    if(status == NodeExtractionStatus::NODE_PRESENT_VALUE_INVALID) {
                        rc = RC_INVALID_VALUE;
                        break;
                    }
                } else {
                    rc = RC_INVALID_VALUE;
                    break;
                }
            }
        } else {
            rc = RC_INVALID_VALUE;
        }
    }

    if(RC_IS_NOTOK(rc)) {
        extFeatureInfoBuilder.setLib("");
    }

    ExtFeaturesRegistry::getInstance()->registerExtFeature(extFeatureInfoBuilder.build());
}

ErrCode ConfigProcessor::parseSignalConfigs(const std::string& filePath, int8_t isBuSpecified) {
    YAML::Node result;
    ErrCode rc = YamlParser::parse(filePath, result);

    if(RC_IS_OK(rc)) {
        if(result[SIGNAL_CONFIGS_ROOT].IsDefined() && result[SIGNAL_CONFIGS_ROOT].IsSequence()) {
            for(int32_t i = 0; i < result[SIGNAL_CONFIGS_ROOT].size(); i++) {
                YAML::Node signalConfig = result[SIGNAL_CONFIGS_ROOT][i];
                try {
                    LOGI("RESTUNE_SIGNAL_PROCESSOR", "Parsing Signal Config at index = " + std::to_string(i));
                    parseSignalConfigYamlNode(signalConfig, isBuSpecified);
                } catch(const std::invalid_argument& e) {
                    LOGE("RESTUNE_SIGNAL_PROCESSOR", "Error parsing Signal Config: " + std::string(e.what()));
                } catch(const std::bad_alloc& e) {
                    LOGE("RESTUNE_SIGNAL_PROCESSOR", "Error parsing Resource Config: " + std::string(e.what()));
                }
            }
        }
    }

    return rc;
}

ErrCode ConfigProcessor::parseExtFeaturesConfigs(const std::string& filePath) {
    YAML::Node result;
    ErrCode rc = YamlParser::parse(filePath, result);

    if(RC_IS_OK(rc)) {
        if(result[EXT_FEATURES_CONFIGS_ROOT].IsDefined() && result[EXT_FEATURES_CONFIGS_ROOT].IsSequence()) {
            for(const auto& featureConfig : result[EXT_FEATURES_CONFIGS_ROOT]) {
                try {
                    parseExtFeatureConfigYamlNode(featureConfig);
                } catch(const std::invalid_argument& e) {
                    LOGE("RESTUNE_EXT_FEATURE_PROCESSOR", "Error parsing Ext Feature Config: " + std::string(e.what()));
                } catch(const std::bad_alloc& e) {
                    LOGE("RESTUNE_EXT_FEATURE_PROCESSOR", "Error parsing Ext Feature Config: " + std::string(e.what()));
                }
            }
        }
    }

    return rc;
}
