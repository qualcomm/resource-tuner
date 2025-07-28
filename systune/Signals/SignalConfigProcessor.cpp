// Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
// SPDX-License-Identifier: BSD-3-Clause-Clear

#include "SignalConfigProcessor.h"

SignalConfigProcessor::SignalConfigProcessor(std::string jsonFilePath) {
    this->mJsonParser = new (std::nothrow) JsonParser();
    if(this->mJsonParser == nullptr) {
        LOGE("URM_SIGNAL_CONFIG_PROCESSOR", "SignalConfig Parsing Failed");
    }

    if(jsonFilePath.length() == 0) {
        // No Custom Signal Config File Specified
        this->mCustomSignalsFileSpecified = false;
        this->mSignalConfigJsonFilePath = SIGNAL_CONFIGS_FILE;
    } else {
        this->mCustomSignalsFileSpecified = true;
        this->mSignalConfigJsonFilePath = jsonFilePath;
    }
}

ErrCode SignalConfigProcessor::parseSignalConfigs() {
    if(this->mJsonParser == nullptr) {
        return RC_JSON_PARSING_ERROR;
    }

    const std::string fSignalConfigFileName(mSignalConfigJsonFilePath);
    const std::string jsonSignalConfigRoot(SIGNAL_CONFIGS_ROOT);

    int32_t size;
    ErrCode rc = this->mJsonParser->verifyAndGetSize(fSignalConfigFileName, jsonSignalConfigRoot, size);

    if(RC_IS_OK(rc)) {
        SignalRegistry::getInstance()->initRegistry(size, this->mCustomSignalsFileSpecified);
        this->mJsonParser->parse(std::bind(&SignalConfigProcessor::SignalConfigsParserCB, this, std::placeholders::_1));
    } else {
        LOGE("URM_SIGNAL_CONFIG_PROCESSOR", "Couldn't parse: " + fSignalConfigFileName);
    }

    int32_t resourcesAllocatedCount = SignalRegistry::getInstance()->getSignalsConfigCount();
    if(resourcesAllocatedCount != size) {
        LOGE("URM_SIGNAL_CONFIG_PROCESSOR", "Couldn't allocate Memory for all the Signal Configs");
        return RC_MEMORY_ALLOCATION_FAILURE;
    }

    return rc;
}

void SignalConfigProcessor::SignalConfigsParserCB(const Json::Value& item) {
    SignalInfoBuilder signalInfoBuilder;

    if(item[SIGNAL_SIGID].isString()) {
        signalInfoBuilder.setOpID(item[SIGNAL_SIGID].asString());
    }

    if(item[SIGNAL_CATEGORY].isString()) {
        signalInfoBuilder.setCategory(item[SIGNAL_CATEGORY].asString());
    }

    if(item[SIGNAL_NAME].isString()) {
        signalInfoBuilder.setName(item[SIGNAL_NAME].asString());
    }

    if(item[SIGNAL_TIMEOUT].isInt()) {
        signalInfoBuilder.setTimeout(item[SIGNAL_TIMEOUT].asInt());
    }

    if(item[SIGNAL_ENABLE].isBool()) {
        signalInfoBuilder.setIsEnabled(item[SIGNAL_ENABLE].asBool());
    }

    if(item[SIGNAL_PERMISSIONS].isArray()) {
        for(int32_t i = 0; i < item[SIGNAL_PERMISSIONS].size(); i++) {
            signalInfoBuilder.addPermission(item[SIGNAL_PERMISSIONS][i].asString());
        }
    }

    if(item[SIGNAL_TARGETS_ENABLED].isArray()) {
        for(int32_t i = 0; i < item[SIGNAL_TARGETS_ENABLED].size(); i++) {
            signalInfoBuilder.addTarget(true, item[SIGNAL_TARGETS_ENABLED][i].asString());
        }
    }

    if(item[SIGNAL_TARGETS_DISABLED].isArray()) {
        for(int32_t i = 0; i < item[SIGNAL_TARGETS_DISABLED].size(); i++) {
            signalInfoBuilder.addTarget(false, item[SIGNAL_TARGETS_DISABLED][i].asString());
        }
    }

    if(item[SIGNAL_DERIVATIVES].isArray()) {
        for(int32_t i = 0; i < item[SIGNAL_DERIVATIVES].size(); i++) {
            signalInfoBuilder.addDerivative(item[SIGNAL_DERIVATIVES][i].asString());
        }
    }

    if(item[SIGNAL_RESOURCES].isArray()) {
        for(int32_t i = 0; i < item[SIGNAL_RESOURCES].size(); i++) {
            signalInfoBuilder.addLock(item[SIGNAL_RESOURCES][i].asUInt());
        }
    }

    SignalRegistry::getInstance()->registerSignal(signalInfoBuilder.build());
}

SignalConfigProcessor::~SignalConfigProcessor() {
    if(this->mJsonParser != nullptr) {
        delete this->mJsonParser;
        this->mJsonParser = nullptr;
    }
}
