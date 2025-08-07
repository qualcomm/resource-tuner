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
        if(result[SIGNAL_CONFIGS_ROOT] && result[SIGNAL_CONFIGS_ROOT].IsSequence()) {
            int32_t signalCount = result[SIGNAL_CONFIGS_ROOT].size();
            SignalRegistry::getInstance()->initRegistry(signalCount, this->mCustomSignalsFileSpecified);

            for(const auto& signalConfig : result[SIGNAL_CONFIGS_ROOT]) {
                parseYamlNode(signalConfig);
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

    if(item[SIGNAL_PERMISSIONS].IsSequence()) {
        for(int32_t i = 0; i < item[SIGNAL_PERMISSIONS].size(); i++) {
            signalInfoBuilder.addPermission(
                safeExtract<std::string>(item[SIGNAL_PERMISSIONS][i])
            );
        }
    }

    if(item[SIGNAL_TARGETS_ENABLED].IsDefined() && item[SIGNAL_TARGETS_ENABLED].IsSequence()) {
        for(int32_t i = 0; i < item[SIGNAL_TARGETS_ENABLED].size(); i++) {
            signalInfoBuilder.addTarget(true,
                safeExtract<std::string>(item[SIGNAL_TARGETS_ENABLED][i])
            );
        }
    }

    if(item[SIGNAL_TARGETS_DISABLED].IsDefined() && item[SIGNAL_TARGETS_DISABLED].IsSequence()) {
        for(int32_t i = 0; i < item[SIGNAL_TARGETS_DISABLED].size(); i++) {
            signalInfoBuilder.addTarget(false,
                safeExtract<std::string>(item[SIGNAL_TARGETS_DISABLED][i])
            );
        }
    }

    if(item[SIGNAL_DERIVATIVES].IsSequence()) {
        for(int32_t i = 0; i < item[SIGNAL_DERIVATIVES].size(); i++) {
            signalInfoBuilder.addDerivative(
                safeExtract<std::string>(item[SIGNAL_DERIVATIVES][i])
            );
        }
    }

    if(item[SIGNAL_RESOURCES].IsSequence()) {
        for(int32_t i = 0; i < item[SIGNAL_RESOURCES].size(); i++) {
            signalInfoBuilder.addLock(
                safeExtract<uint32_t>(item[SIGNAL_RESOURCES][i])
            );
        }
    }

    SignalRegistry::getInstance()->registerSignal(signalInfoBuilder.build());
}
