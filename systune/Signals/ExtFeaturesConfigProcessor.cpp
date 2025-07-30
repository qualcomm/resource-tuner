// Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
// SPDX-License-Identifier: BSD-3-Clause-Clear

#include "ExtFeaturesConfigProcessor.h"

ExtFeaturesConfigProcessor::ExtFeaturesConfigProcessor(const std::string& jsonFilePath) {
    this->mJsonParser = new (std::nothrow) JsonParser();
    if(this->mJsonParser == nullptr) {
        LOGE("URM_EXT_FEATURES_CONFIG_PROCESSOR", "ExtFeatureConfig Parsing Failed");
    }

    if(jsonFilePath.length() == 0) {
        // No Custom ExtFeature Config File Specified
        this->mCustomExtFeaturesFileSpecified = false;
        this->mExtFeaturesConfigsJsonFilePath = EXT_FEATURES_CONFIGS_FILE;
    } else {
        this->mCustomExtFeaturesFileSpecified = true;
        this->mExtFeaturesConfigsJsonFilePath = jsonFilePath;
    }
}

ErrCode ExtFeaturesConfigProcessor::parseExtFeaturesConfigs() {
    if(this->mJsonParser == nullptr) {
        return RC_JSON_PARSING_ERROR;
    }

    const std::string fExtFeaturesConfigFileName(mExtFeaturesConfigsJsonFilePath);
    const std::string jsonExtFeaturesConfigRoot(EXT_FEATURES_CONFIGS_ROOT);

    int32_t size;
    ErrCode rc = this->mJsonParser->verifyAndGetSize(fExtFeaturesConfigFileName, jsonExtFeaturesConfigRoot, size);

    if(RC_IS_OK(rc)) {
        ExtFeaturesRegistry::getInstance()->initRegistry(size);
        this->mJsonParser->parse(std::bind(&ExtFeaturesConfigProcessor::ExtFeaturesConfigsParserCB, this, std::placeholders::_1));
    } else {
        LOGE("URM_EXT_FEATURES_CONFIG_PROCESSOR", "Couldn't parse: " + fExtFeaturesConfigFileName);
    }

    return rc;
}

void ExtFeaturesConfigProcessor::ExtFeaturesConfigsParserCB(const Json::Value& item) {
    ExtFeatureInfoBuilder extFeatureInfoBuilder;

    if(item[EXT_FEATURE_ID].isInt()) {
        extFeatureInfoBuilder.setId(item[EXT_FEATURE_ID].asInt());
    }

    if(item[EXT_FEATURE_LIB].isString()) {
        extFeatureInfoBuilder.setLib(item[EXT_FEATURE_LIB].asString());
    }

    if(item[EXT_FEATURE_SIGNAL_RANGE].isArray()) {
        uint32_t lowerBound = (uint32_t)item[EXT_FEATURE_SIGNAL_RANGE][0].asInt();
        uint32_t upperBound = (uint32_t)item[EXT_FEATURE_SIGNAL_RANGE][1].asInt();

        for(uint32_t i = lowerBound; i <= upperBound; i++) {
            extFeatureInfoBuilder.addSignalsSubscribedTo(i);
        }
    }

    if(item[EXT_FEATURE_SIGNAL_INDIVIDUAL].isArray()) {
        for(int32_t i = 0; i < item[EXT_FEATURE_SIGNAL_INDIVIDUAL].size(); i++) {
            extFeatureInfoBuilder.addSignalsSubscribedTo((uint32_t)item[EXT_FEATURE_SIGNAL_INDIVIDUAL][i].asInt());
        }
    }

    ExtFeaturesRegistry::getInstance()->registerExtFeature(extFeatureInfoBuilder.build()); 
}

ExtFeaturesConfigProcessor::~ExtFeaturesConfigProcessor() {
    delete this->mJsonParser;
    this->mJsonParser = nullptr;
}
