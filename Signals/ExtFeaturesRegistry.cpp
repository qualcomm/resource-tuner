// Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
// SPDX-License-Identifier: BSD-3-Clause-Clear

#include "ExtFeaturesRegistry.h"

std::shared_ptr<ExtFeaturesRegistry> ExtFeaturesRegistry::extFeaturesRegistryInstance = nullptr;

ExtFeaturesRegistry::ExtFeaturesRegistry() {
    this->mTotalExtFeatures = 0;
}

void ExtFeaturesRegistry::registerExtFeature(ExtFeatureInfo* featureInfo) {
    if(featureInfo == nullptr) {
        return;
    }

    if(featureInfo->mFeatureLib.length() == 0) {
        if(featureInfo->mSignalsSubscribedTo != nullptr) {
            delete featureInfo->mSignalsSubscribedTo;
        }
        delete featureInfo;
        return;
    }

    this->mSystemIndependentLayerMappings[featureInfo->mFeatureId] = mTotalExtFeatures;
    this->mExtFeaturesConfigs.push_back(featureInfo);

    this->mTotalExtFeatures++;

    for(uint32_t signalId: *featureInfo->mSignalsSubscribedTo) {
        SignalExtFeatureMapper::getInstance()->addFeature(signalId, featureInfo->mFeatureId);
    }
}

std::vector<ExtFeatureInfo*> ExtFeaturesRegistry::getExtFeaturesConfigs() {
    return this->mExtFeaturesConfigs;
}

ExtFeatureInfo* ExtFeaturesRegistry::getExtFeatureConfigById(int32_t featureId) {
    if(this->mSystemIndependentLayerMappings.find(featureId) == this->mSystemIndependentLayerMappings.end()) {
        LOGE("RESTUNE_RESOURCE_PROCESSOR", "Ext Feature ID not found in the registry");
        return nullptr;
    }

    int32_t mExtFeaturesConfigsTableIndex = this->mSystemIndependentLayerMappings[featureId];
    return this->mExtFeaturesConfigs[mExtFeaturesConfigsTableIndex];
}

int32_t ExtFeaturesRegistry::getExgFeaturesConfigCount() {
    return this->mExtFeaturesConfigs.size();
}

void ExtFeaturesRegistry::displayExtFeatures() {
    for(int32_t i = 0; i < this->mTotalExtFeatures; i++) {
        auto& extFeature = this->mExtFeaturesConfigs[i];

        LOGI("RESTUNE_EXT_FEATURES_REGISTRY", "Ext Feature ID: " + std::to_string(extFeature->mFeatureId));
        LOGI("RESTUNE_EXT_FEATURES_REGISTRY", "Ext Feature Name: " + extFeature->mFeatureName);
        LOGI("RESTUNE_EXT_FEATURES_REGISTRY", "Ext Feature Lib: " + extFeature->mFeatureLib);

        for(uint32_t signalID: *extFeature->mSignalsSubscribedTo) {
            LOGI("RESTUNE_EXT_FEATURES_REGISTRY", "Ext Feature Signal ID: " + std::to_string(signalID));
        }
    }
}

ExtFeaturesRegistry::~ExtFeaturesRegistry() {
    for(int32_t i = 0; i < this->mExtFeaturesConfigs.size(); i++) {
        delete(this->mExtFeaturesConfigs[i]);
        this->mExtFeaturesConfigs[i] = nullptr;
    }
}

ExtFeatureInfoBuilder::ExtFeatureInfoBuilder() {
    this->mFeatureInfo = new(std::nothrow) ExtFeatureInfo();
}

ErrCode ExtFeatureInfoBuilder::setId(const std::string& mFeatureId) {
    if(this->mFeatureInfo == nullptr) {
        return RC_INVALID_VALUE;
    }

    this->mFeatureInfo->mFeatureId = 0;
    try {
        this->mFeatureInfo->mFeatureId = (uint32_t)stol(mFeatureId, nullptr, 0);
    } catch(const std::invalid_argument& e) {
        TYPELOGV(SIGNAL_REGISTRY_PARSING_FAILURE, e.what());
        return RC_INVALID_VALUE;

    } catch(const std::out_of_range& e) {
        TYPELOGV(SIGNAL_REGISTRY_PARSING_FAILURE, e.what());
        return RC_INVALID_VALUE;
    }

    return RC_SUCCESS;
}

 ErrCode ExtFeatureInfoBuilder::setName(const std::string& featureName) {
    if(this->mFeatureInfo == nullptr) {
        return RC_INVALID_VALUE;
    }

    this->mFeatureInfo->mFeatureName = featureName;
    return RC_SUCCESS;
 }

ErrCode ExtFeatureInfoBuilder::setLib(const std::string& featureLib) {
    if(this->mFeatureInfo == nullptr) {
        return RC_INVALID_VALUE;
    }

    this->mFeatureInfo->mFeatureLib = featureLib;
    return RC_SUCCESS;
}

ErrCode ExtFeatureInfoBuilder::addSignalSubscribedTo(const std::string& signalOpCodeString) {
    if(this->mFeatureInfo == nullptr || signalOpCodeString.length() == 0) {
        return RC_INVALID_VALUE;
    }

    uint32_t signalOpCode = 0;
    try {
        signalOpCode = (uint32_t)stol(signalOpCodeString, nullptr, 0);
    } catch(const std::invalid_argument& e) {
        TYPELOGV(SIGNAL_REGISTRY_PARSING_FAILURE, e.what());
        return RC_INVALID_VALUE;

    } catch(const std::out_of_range& e) {
        TYPELOGV(SIGNAL_REGISTRY_PARSING_FAILURE, e.what());
        return RC_INVALID_VALUE;
    }

    if(this->mFeatureInfo->mSignalsSubscribedTo == nullptr) {
        this->mFeatureInfo->mSignalsSubscribedTo = new(std::nothrow) std::vector<uint32_t>;
    }

    if(this->mFeatureInfo->mSignalsSubscribedTo == nullptr) {
        return RC_INVALID_VALUE;
    }

    try {
        this->mFeatureInfo->mSignalsSubscribedTo->push_back(signalOpCode);
    } catch(const std::bad_alloc& e) {
        return RC_INVALID_VALUE;
    }

    return RC_SUCCESS;
}

ExtFeatureInfo* ExtFeatureInfoBuilder::build() {
    return this->mFeatureInfo;
}
