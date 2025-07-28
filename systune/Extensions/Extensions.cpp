
// Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
// SPDX-License-Identifier: BSD-3-Clause-Clear

#include "Extensions.h"

std::vector<std::string> Extensions::mModifiedConfigFiles (TOTAL_CONFIGS_COUNT, "");
std::unordered_map<int32_t, void (*)(void*)> Extensions::mModifiedResourceConfigs {};

Extensions::Extensions(int32_t resourceOpcode, ResourceApplierCallback resourceApplierCallback) {
    this->mModifiedResourceConfigs[resourceOpcode] = resourceApplierCallback;
}

Extensions::Extensions(ConfigType configType, std::string jsonFile) {
    if(configType < 0 || configType >= mModifiedConfigFiles.size()) return;
    mModifiedConfigFiles[configType] = jsonFile;
}

std::vector<std::pair<int32_t, ResourceApplierCallback>> Extensions::getModifiedResources() {
    std::vector<std::pair<int32_t, ResourceApplierCallback>> modifiedResources;
    for(std::pair<int32_t, ResourceApplierCallback> resource: mModifiedResourceConfigs) {
        modifiedResources.push_back(resource);
    }
    return modifiedResources;
}

std::string Extensions::getResourceConfigFilePath() {
    return mModifiedConfigFiles[RESOURCE_CONFIG];
}

std::string Extensions::getPropertiesConfigFilePath() {
    return mModifiedConfigFiles[PROPERTIES_CONFIG];
}

std::string Extensions::getSignalsConfigFilePath() {
    return mModifiedConfigFiles[SIGNALS_CONFIG];
}

std::string Extensions::getExtFeaturesConfigFilePath() {
    return mModifiedConfigFiles[EXT_FEATURES_CONFIG];
}

std::string Extensions::getTargetConfigFilePath() {
    return mModifiedConfigFiles[TARGET_CONFIG];
}
