// Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
// SPDX-License-Identifier: BSD-3-Clause-Clear

#include "SignalConfigProcessor.h"

SignalConfigProcessor::SignalConfigProcessor(const std::string& yamlFilePath) {
    if(yamlFilePath.length() == 0) {
        // No Custom Signal Config File Specified
        this->mCustomSignalsFileSpecified = false;
        this->mSignalConfigYamlFilePath = SIGNAL_CONFIGS_FILE;
    } else {
        this->mCustomSignalsFileSpecified = true;
        this->mSignalConfigYamlFilePath = yamlFilePath;
    }
}

ErrCode SignalConfigProcessor::parseSignalConfigs() {
    const std::string fSignalConfigFileName(mSignalConfigYamlFilePath);

    YAML::Node result;
    ErrCode rc = YamlParser::parse(fSignalConfigFileName, result);

    if(RC_IS_OK(rc)) {
        if(result[SIGNAL_CONFIGS_ROOT].IsDefined() && result[SIGNAL_CONFIGS_ROOT].IsSequence()) {
            int32_t signalCount = result[SIGNAL_CONFIGS_ROOT].size();
            SignalRegistry::getInstance()->initRegistry(signalCount, this->mCustomSignalsFileSpecified);

            for(const auto& signalConfig : result[SIGNAL_CONFIGS_ROOT]) {
                try {
                    parseYamlNode(signalConfig);
                } catch(const std::invalid_argument& e) {
                    LOGE("URM_SIGNAL_PROCESSOR", "Error parsing Signal Config: " + std::string(e.what()));
                }
            }
        }
    }

    return rc;
}

void SignalConfigProcessor::parseYamlNode(const YAML::Node& item) {
    SignalInfoBuilder signalInfoBuilder;

    signalInfoBuilder.setOpID(
        safeExtract<std::string>(item[SIGNAL_SIGID])
    );

    signalInfoBuilder.setCategory(
        safeExtract<std::string>(item[SIGNAL_CATEGORY])
    );

    signalInfoBuilder.setName(
        safeExtract<std::string>(item[SIGNAL_NAME])
    );

    signalInfoBuilder.setTimeout(
        safeExtract<int32_t>(item[SIGNAL_TIMEOUT])
    );

    signalInfoBuilder.setIsEnabled(
        safeExtract<bool>(item[SIGNAL_ENABLE])
    );

    if(isList(item[SIGNAL_PERMISSIONS])) {
        for(int32_t i = 0; i < item[SIGNAL_PERMISSIONS].size(); i++) {
            signalInfoBuilder.addPermission(
                safeExtract<std::string>(item[SIGNAL_PERMISSIONS][i])
            );
        }
    }

    if(isList(item[SIGNAL_TARGETS_ENABLED])) {
        for(int32_t i = 0; i < item[SIGNAL_TARGETS_ENABLED].size(); i++) {
            signalInfoBuilder.addTarget(true,
                safeExtract<std::string>(item[SIGNAL_TARGETS_ENABLED][i])
            );
        }
    }

    if(isList(item[SIGNAL_TARGETS_DISABLED])) {
        for(int32_t i = 0; i < item[SIGNAL_TARGETS_DISABLED].size(); i++) {
            signalInfoBuilder.addTarget(false,
                safeExtract<std::string>(item[SIGNAL_TARGETS_DISABLED][i])
            );
        }
    }

    if(isList(item[SIGNAL_DERIVATIVES])) {
        for(int32_t i = 0; i < item[SIGNAL_DERIVATIVES].size(); i++) {
            signalInfoBuilder.addDerivative(
                safeExtract<std::string>(item[SIGNAL_DERIVATIVES][i])
            );
        }
    }

    if(item[SIGNAL_RESOURCES].IsDefined() && item[SIGNAL_RESOURCES].IsSequence()) {
        for(const auto& resourceConfig: item[SIGNAL_RESOURCES]) {
            ResourceBuilder resourceBuilder;
            resourceBuilder.setResId(
                safeExtract<std::string>(resourceConfig[SIGNAL_RESOURCE_ID])
            );

            resourceBuilder.setResType(
                safeExtract<std::string>(resourceConfig[SIGNAL_RESOURCE_TYPE])
            );

            resourceBuilder.setOpInfo(
                safeExtract<int32_t>(resourceConfig[SIGNAL_OPINFO])
            );

            if(isList(resourceConfig[SIGNAL_VALUES])) {
                int32_t valuesCount = resourceConfig[SIGNAL_VALUES].size();
                resourceBuilder.setNumValues(valuesCount);

                for(int32_t i = 0; i < valuesCount; i++) {
                    resourceBuilder.addValue(
                        safeExtract<int32_t>(resourceConfig[SIGNAL_VALUES][i])
                    );
                }
            }
            signalInfoBuilder.addResource(resourceBuilder.build());
        }
    }

    SignalRegistry::getInstance()->registerSignal(signalInfoBuilder.build());
}
