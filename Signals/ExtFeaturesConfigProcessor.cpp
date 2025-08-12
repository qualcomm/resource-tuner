// Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
// SPDX-License-Identifier: BSD-3-Clause-Clear

#include "ExtFeaturesConfigProcessor.h"

ExtFeaturesConfigProcessor::ExtFeaturesConfigProcessor(const std::string& yamlFilePath) {
    if(yamlFilePath.length() == 0) {
        // No Custom ExtFeature Config File Specified
        this->mCustomExtFeaturesFileSpecified = false;
        this->mExtFeaturesConfigsYamlFilePath = EXT_FEATURES_CONFIGS_FILE;
    } else {
        this->mCustomExtFeaturesFileSpecified = true;
        this->mExtFeaturesConfigsYamlFilePath = yamlFilePath;
    }
}

ErrCode ExtFeaturesConfigProcessor::parseExtFeaturesConfigs() {
    const std::string fExtFeaturesConfigFileName(mExtFeaturesConfigsYamlFilePath);

    YAML::Node result;
    ErrCode rc = YamlParser::parse(fExtFeaturesConfigFileName, result);

    if(RC_IS_OK(rc)) {
        if(result[EXT_FEATURES_CONFIGS_ROOT].IsDefined() && result[EXT_FEATURES_CONFIGS_ROOT].IsSequence()) {
            int32_t featuresCount = result[EXT_FEATURES_CONFIGS_ROOT].size();
            ExtFeaturesRegistry::getInstance()->initRegistry(featuresCount);

            for(const auto& featureConfig : result[EXT_FEATURES_CONFIGS_ROOT]) {
                try {
                    parseYamlNode(featureConfig);
                } catch(const std::invalid_argument& e) {
                    LOGE("RTN_EXT_FEATURE_PROCESSOR", "Error parsing Ext Feature Config: " + std::string(e.what()));
                }
            }
        }
    }

    return rc;
}

void ExtFeaturesConfigProcessor::parseYamlNode(const YAML::Node& item) {
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
