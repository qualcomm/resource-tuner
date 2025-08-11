// Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
// SPDX-License-Identifier: BSD-3-Clause-Clear

#include "ExtFeaturesRegistry.h"

std::shared_ptr<ExtFeaturesRegistry> ExtFeaturesRegistry::extFeaturesRegistryInstance = nullptr;
int32_t ExtFeaturesRegistry::mTotalExtFeatures = 0;

ExtFeaturesRegistry::ExtFeaturesRegistry() {}

void ExtFeaturesRegistry::initRegistry(int32_t size) {
    if(mExtFeaturesConfigs.size() == 0 && size > 0) {
        mExtFeaturesConfigs.resize(size);
    }
}

void ExtFeaturesRegistry::registerExtFeature(ExtFeatureInfo* featureInfo) {
    this->mSystemIndependentLayerMappings[featureInfo->mFeatureId] = mTotalExtFeatures;

    this->mExtFeaturesConfigs[mTotalExtFeatures] = featureInfo;
    mTotalExtFeatures++;

    for(uint32_t signalId: *featureInfo->mSignalsSubscribedTo) {
        SignalExtFeatureMapper::getInstance()->addFeature(signalId, featureInfo->mFeatureId);
    }
}

std::vector<ExtFeatureInfo*> ExtFeaturesRegistry::getExtFeaturesConfigs() {
    return this->mExtFeaturesConfigs;
}

ExtFeatureInfo* ExtFeaturesRegistry::getExtFeatureConfigById(int32_t featureId) {
    if(this->mSystemIndependentLayerMappings.find(featureId) == this->mSystemIndependentLayerMappings.end()) {
        LOGE("URM_RESOURCE_PROCESSOR", "Ext Feature ID not found in the registry");
        return nullptr;
    }

    int32_t mExtFeaturesConfigsTableIndex = this->mSystemIndependentLayerMappings[featureId];
    return this->mExtFeaturesConfigs[mExtFeaturesConfigsTableIndex];
}

int32_t ExtFeaturesRegistry::getExgFeaturesConfigCount() {
    return this->mExtFeaturesConfigs.size();
}

void ExtFeaturesRegistry::displayExtFeatures() {
    for(int32_t i = 0; i < mTotalExtFeatures; i++) {
        auto& extFeature = this->mExtFeaturesConfigs[i];

        LOGI("URM_EXT_FEATURES_REGISTRY", "Ext Feature Lib: " + extFeature->mFeatureLib);
        for(uint32_t signalID: *extFeature->mSignalsSubscribedTo) {
            LOGI("URM_EXT_FEATURES_REGISTRY", "Ext Feature Signal ID: " + std::to_string(signalID));
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
    this->mFeatureInfo = new ExtFeatureInfo();
}

ExtFeatureInfoBuilder* ExtFeatureInfoBuilder::setId(int32_t mFeatureId) {
    this->mFeatureInfo->mFeatureId = mFeatureId;
    return this;
}

ExtFeatureInfoBuilder* ExtFeatureInfoBuilder::setLib(const std::string& featureLib) {
    this->mFeatureInfo->mFeatureLib = featureLib;
    return this;
}

ExtFeatureInfoBuilder* ExtFeatureInfoBuilder::addSignalsSubscribedTo(uint32_t signalId) {
    if(this->mFeatureInfo->mSignalsSubscribedTo == nullptr) {
        this->mFeatureInfo->mSignalsSubscribedTo = new std::vector<uint32_t>();
    }
    this->mFeatureInfo->mSignalsSubscribedTo->push_back(signalId);
    return this;
}

ExtFeatureInfo* ExtFeatureInfoBuilder::build() {
    return this->mFeatureInfo;
}
