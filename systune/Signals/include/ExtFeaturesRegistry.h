// Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
// SPDX-License-Identifier: BSD-3-Clause-Clear

#ifndef EXT_FEATURES_REGISTRY_H
#define EXT_FEATURES_REGISTRY_H

#include <vector>
#include <memory>
#include <unordered_map>

#include "SignalExtFeatureMapper.h"
#include "Utils.h"
#include "Logger.h"

typedef struct {
    int32_t mFeatureId;
    std::string mFeatureLib;
    std::vector<uint32_t>* mSignalsSubscribedTo;
} ExtFeatureInfo;

class ExtFeaturesRegistry {
private:
    static std::shared_ptr<ExtFeaturesRegistry> extFeaturesRegistryInstance;
    static int32_t mTotalExtFeatures;
    std::vector<ExtFeatureInfo*> mExtFeaturesConfigs;

    std::unordered_map<uint32_t, int32_t> mSystemIndependentLayerMappings;

    ExtFeaturesRegistry();

public:
    ~ExtFeaturesRegistry();

    void initRegistry(int32_t size);

    void registerExtFeature(ExtFeatureInfo* extFeatureInfo);

    std::vector<ExtFeatureInfo*> getExtFeaturesConfigs();

    ExtFeatureInfo* getExtFeatureConfigById(int32_t extFeatureId);

    int32_t getExgFeaturesConfigCount();

    void displayExtFeatures();

    static std::shared_ptr<ExtFeaturesRegistry> getInstance() {
        if(extFeaturesRegistryInstance == nullptr) {
            extFeaturesRegistryInstance = std::shared_ptr<ExtFeaturesRegistry> (new ExtFeaturesRegistry());
        }
        return extFeaturesRegistryInstance;
    }
};

class ExtFeatureInfoBuilder {
private:
    ExtFeatureInfo* mFeatureInfo;

public:
    ExtFeatureInfoBuilder();

    ExtFeatureInfoBuilder* setId(int32_t featureId);
    ExtFeatureInfoBuilder* setLib(const std::string& featureLib);
    ExtFeatureInfoBuilder* addSignalsSubscribedTo(uint32_t signalId);

    ExtFeatureInfo* build();
};

#endif
