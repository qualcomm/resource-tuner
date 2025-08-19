// Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
// SPDX-License-Identifier: BSD-3-Clause-Clear

#include "ConfigProcessor.h"

void ConfigProcessor::parseSignalConfigYamlNode(const YAML::Node& item, int8_t isBuSpecified) {
    SignalInfoBuilder signalInfoBuilder;

    signalInfoBuilder.setOpID(
        safeExtract<std::string>(item[SIGNAL_SIGID], "-1")
    );

    signalInfoBuilder.setCategory(
        safeExtract<std::string>(item[SIGNAL_CATEGORY], "-1")
    );

    signalInfoBuilder.setName(
        safeExtract<std::string>(item[SIGNAL_NAME], "")
    );

    signalInfoBuilder.setTimeout(
        safeExtract<int32_t>(item[SIGNAL_TIMEOUT], 1)
    );

    signalInfoBuilder.setIsEnabled(
        safeExtract<bool>(item[SIGNAL_ENABLE], false)
    );

    if(isList(item[SIGNAL_PERMISSIONS])) {
        for(int32_t i = 0; i < item[SIGNAL_PERMISSIONS].size(); i++) {
            signalInfoBuilder.addPermission(
                safeExtract<std::string>(item[SIGNAL_PERMISSIONS][i], "")
            );
        }
    }

    if(isList(item[SIGNAL_TARGETS_ENABLED])) {
        for(int32_t i = 0; i < item[SIGNAL_TARGETS_ENABLED].size(); i++) {
            signalInfoBuilder.addTarget(true,
                safeExtract<std::string>(item[SIGNAL_TARGETS_ENABLED][i], "")
            );
        }
    }

    if(isList(item[SIGNAL_TARGETS_DISABLED])) {
        for(int32_t i = 0; i < item[SIGNAL_TARGETS_DISABLED].size(); i++) {
            signalInfoBuilder.addTarget(false,
                safeExtract<std::string>(item[SIGNAL_TARGETS_DISABLED][i], "")
            );
        }
    }

    if(isList(item[SIGNAL_DERIVATIVES])) {
        for(int32_t i = 0; i < item[SIGNAL_DERIVATIVES].size(); i++) {
            signalInfoBuilder.addDerivative(
                safeExtract<std::string>(item[SIGNAL_DERIVATIVES][i], "")
            );
        }
    }

    if(item[SIGNAL_RESOURCES].IsDefined() && item[SIGNAL_RESOURCES].IsSequence()) {
        for(const auto& resourceConfig: item[SIGNAL_RESOURCES]) {
            if(!resourceConfig.IsMap()) break;

            ResourceBuilder resourceBuilder;
            resourceBuilder.setResCode(
                safeExtract<std::string>(resourceConfig[SIGNAL_RESOURCE_CODE], "-1")
            );

            resourceBuilder.setOpInfo(
                safeExtract<std::string>(resourceConfig[SIGNAL_RESINFO], "0")
            );

            if(isList(resourceConfig[SIGNAL_VALUES])) {
                int32_t valuesCount = resourceConfig[SIGNAL_VALUES].size();
                resourceBuilder.setNumValues(valuesCount);

                for(int32_t i = 0; i < valuesCount; i++) {
                    resourceBuilder.addValue(
                        safeExtract<int32_t>(resourceConfig[SIGNAL_VALUES][i], -1)
                    );
                }
            }

            signalInfoBuilder.addResource(resourceBuilder.build());
        }
    }

    SignalRegistry::getInstance()->registerSignal(signalInfoBuilder.build(), isBuSpecified);
}

void ConfigProcessor::parseExtFeatureConfigYamlNode(const YAML::Node& item) {
    ExtFeatureInfoBuilder extFeatureInfoBuilder;

    extFeatureInfoBuilder.setId(
        safeExtract<int32_t>(item[EXT_FEATURE_ID])
    );

    extFeatureInfoBuilder.setLib(
        safeExtract<std::string>(item[EXT_FEATURE_LIB])
    );

    if(isList(item[EXT_FEATURE_SIGNAL_RANGE])) {
        uint32_t lowerBound = (uint32_t)safeExtract<uint32_t>(item[EXT_FEATURE_SIGNAL_RANGE][0]);
        uint32_t upperBound = (uint32_t)safeExtract<uint32_t>(item[EXT_FEATURE_SIGNAL_RANGE][1]);

        for(uint32_t i = lowerBound; i <= upperBound; i++) {
            extFeatureInfoBuilder.addSignalsSubscribedTo(i);
        }
    }

    if(isList(item[EXT_FEATURE_SIGNAL_INDIVIDUAL])) {
        for(int32_t i = 0; i < item[EXT_FEATURE_SIGNAL_INDIVIDUAL].size(); i++) {
            extFeatureInfoBuilder.addSignalsSubscribedTo(
                (uint32_t)(safeExtract<uint32_t>(item[EXT_FEATURE_SIGNAL_INDIVIDUAL][i]))
            );
        }
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
                    LOGI("RTN_SIGNAL_PROCESSOR", "Parsing Signal Config at index = " + std::to_string(i));
                    parseSignalConfigYamlNode(signalConfig, isBuSpecified);
                } catch(const std::invalid_argument& e) {
                    LOGE("RTN_SIGNAL_PROCESSOR", "Error parsing Signal Config: " + std::string(e.what()));
                } catch(const std::bad_alloc& e) {
                    LOGE("RTN_SIGNAL_PROCESSOR", "Error parsing Resource Config: " + std::string(e.what()));
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
            int32_t featuresCount = result[EXT_FEATURES_CONFIGS_ROOT].size();
            ExtFeaturesRegistry::getInstance()->initRegistry(featuresCount);

            for(const auto& featureConfig : result[EXT_FEATURES_CONFIGS_ROOT]) {
                try {
                    parseExtFeatureConfigYamlNode(featureConfig);
                } catch(const std::invalid_argument& e) {
                    LOGE("RTN_EXT_FEATURE_PROCESSOR", "Error parsing Ext Feature Config: " + std::string(e.what()));
                }
            }
        }
    }

    return rc;
}
