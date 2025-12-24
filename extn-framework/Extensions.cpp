// Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
// SPDX-License-Identifier: BSD-3-Clause-Clear

#include "Utils.h"
#include "Extensions.h"

std::vector<std::string> Extensions::mModifiedConfigFiles (TOTAL_CONFIGS_COUNT, "");
std::unordered_map<uint32_t, ResourceLifecycleCallback> Extensions::mResourceApplierCallbacks {};
std::unordered_map<uint32_t, ResourceLifecycleCallback> Extensions::mResourceTearCallbacks {};
std::unordered_map<std::string, PostProcessingCallback> Extensions::mPostProcessCallbacks {};

Extensions::Extensions(uint32_t resCode, int8_t callbackType, ResourceLifecycleCallback callback) {
    if(callbackType == 0) {
        mResourceApplierCallbacks[resCode] = callback;
    } else if(callbackType == 1) {
        mResourceTearCallbacks[resCode] = callback;
    }
}

Extensions::Extensions(ConfigType configType, std::string yamlFile) {
    if(configType < 0 || configType >= mModifiedConfigFiles.size()) return;
    mModifiedConfigFiles[configType] = yamlFile;
}

Extensions::Extensions(const std::string& identifier, PostProcessingCallback callback) {
    mPostProcessCallbacks[identifier] = callback;
}

std::vector<std::pair<uint32_t, ResourceLifecycleCallback>> Extensions::getResourceApplierCallbacks() {
    std::vector<std::pair<uint32_t, ResourceLifecycleCallback>> modifiedResources;
    for(std::pair<uint32_t, ResourceLifecycleCallback> resource: mResourceApplierCallbacks) {
        modifiedResources.push_back(resource);
    }
    return modifiedResources;
}

std::vector<std::pair<uint32_t, ResourceLifecycleCallback>> Extensions::getResourceTearCallbacks() {
    std::vector<std::pair<uint32_t, ResourceLifecycleCallback>> modifiedResources;
    for(std::pair<uint32_t, ResourceLifecycleCallback> resource: mResourceTearCallbacks) {
        modifiedResources.push_back(resource);
    }
    return modifiedResources;
}

std::string Extensions::getResourceConfigFilePath() {
    return mModifiedConfigFiles[ConfigType::RESOURCE_CONFIG];
}

std::string Extensions::getPropertiesConfigFilePath() {
    return mModifiedConfigFiles[ConfigType::PROPERTIES_CONFIG];
}

std::string Extensions::getSignalsConfigFilePath() {
    return mModifiedConfigFiles[ConfigType::SIGNALS_CONFIG];
}

std::string Extensions::getExtFeaturesConfigFilePath() {
    return mModifiedConfigFiles[ConfigType::EXT_FEATURES_CONFIG];
}

std::string Extensions::getTargetConfigFilePath() {
    return mModifiedConfigFiles[ConfigType::TARGET_CONFIG];
}

std::string Extensions::getInitConfigFilePath() {
    return mModifiedConfigFiles[ConfigType::INIT_CONFIG];
}

std::string Extensions::getAppConfigFilePath() {
    return mModifiedConfigFiles[ConfigType::APP_CONFIG];
}

PostProcessingCallback Extensions::getPostProcessingCallback(const std::string& identifier) {
    return mPostProcessCallbacks[identifier];
}
