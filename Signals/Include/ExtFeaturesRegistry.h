// Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
// SPDX-License-Identifier: BSD-3-Clause-Clear

#ifndef EXT_FEATURES_REGISTRY_H
#define EXT_FEATURES_REGISTRY_H

#include <vector>
#include <memory>
#include <unordered_map>
#include <dlfcn.h>

#include "SignalExtFeatureMapper.h"
#include "Utils.h"
#include "Logger.h"

#define INITIALIZE_FEATURE_ROUTINE "initFeature"
#define TEARDOWN_FEATURE_ROUTINE "tearFeature"
#define RELAY_FEATURE_ROUTINE "relayFeature"

typedef struct {
    uint32_t mFeatureId;
    std::string mFeatureLib;
    std::string mFeatureName;
    std::vector<uint32_t>* mSignalsSubscribedTo;
} ExtFeatureInfo;

class ExtFeaturesRegistry {
private:
    static std::shared_ptr<ExtFeaturesRegistry> extFeaturesRegistryInstance;
    int32_t mTotalExtFeatures;
    std::vector<ExtFeatureInfo*> mExtFeaturesConfigs;

    std::unordered_map<uint32_t, int32_t> mSystemIndependentLayerMappings;

    ExtFeaturesRegistry();

public:
    ~ExtFeaturesRegistry();

    void registerExtFeature(ExtFeatureInfo* extFeatureInfo);

    std::vector<ExtFeatureInfo*> getExtFeaturesConfigs();

    ExtFeatureInfo* getExtFeatureConfigById(int32_t extFeatureId);

    int32_t getExgFeaturesConfigCount();

    void displayExtFeatures();

    void initializeFeatures();

    void teardownFeatures();

    ErrCode relayToFeature(uint32_t featureId);

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

    ErrCode setId(const std::string& featureIdString);
    ErrCode setName(const std::string& featureName);
    ErrCode setLib(const std::string& featureLib);
    ErrCode addSignalSubscribedTo(const std::string& signalOpCodeString);

    ExtFeatureInfo* build();
};

#endif
