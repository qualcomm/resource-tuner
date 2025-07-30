// Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
// SPDX-License-Identifier: BSD-3-Clause-Clear

#ifndef EXT_FEATURES_CONFIG_PROCESSOR_H
#define EXT_FEATURES_CONFIG_PROCESSOR_H

#include <iostream>
#include <memory>

#include "JsonParser.h"
#include "ExtFeaturesRegistry.h"
#include "ComponentRegistry.h"

#define EXT_FEATURES_CONFIGS_FILE  "../Configs/Signals/extFeaturesConfigs.json"
#define EXT_FEATURES_CONFIGS_ROOT "ExtFeaturesConfigs"

#define EXT_FEATURE_ID "FeatureId"
#define EXT_FEATURE_LIB "Lib"
#define EXT_FEATURE_SIGNAL_RANGE "Range"
#define EXT_FEATURE_SIGNAL_INDIVIDUAL "Individual"

class ExtFeaturesConfigProcessor {
private:
    static std::shared_ptr<ExtFeaturesConfigProcessor> extFeaturesConfigProcessorInstance;
    JsonParser* mJsonParser;
    std::string mExtFeaturesConfigsJsonFilePath;
    int8_t mCustomExtFeaturesFileSpecified;

    void ExtFeaturesConfigsParserCB(const Json::Value& item);

public:
    ExtFeaturesConfigProcessor(const std::string& jsonFilePath);
    ~ExtFeaturesConfigProcessor();

    ErrCode parseExtFeaturesConfigs();
};

#endif
