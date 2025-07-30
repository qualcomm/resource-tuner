// Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
// SPDX-License-Identifier: BSD-3-Clause-Clear

#include "SysConfigProcessor.h"

std::shared_ptr<SysConfigProcessor> SysConfigProcessor::sysConfigProcessorInstance = nullptr;

SysConfigProcessor::SysConfigProcessor(const std::string& jsonFilePath) {
    this->mJsonParser = new (std::nothrow) JsonParser();
    if(this->mJsonParser == nullptr) {
        LOGE("URM_SYSCONFIG_PROCESSOR", "SysConfig Properties Parsing Failed");
    }

    if(jsonFilePath.length() == 0) {
        // No Custom Properties File Specified
        mPropertiesConfigJsonFilePath = SYS_CONFIGS_PROPS_FILE;
    } else {
        mPropertiesConfigJsonFilePath = jsonFilePath;
    }
}

ErrCode SysConfigProcessor::parseSysConfigs() {
    if(this->mJsonParser == nullptr) {
        return RC_JSON_PARSING_ERROR;
    }

    const std::string fSysConfigPropsFileName(mPropertiesConfigJsonFilePath);
    const std::string jsonSysConfigPropsRoot(SYS_CONFIGS_ROOT);

    int32_t size;
    ErrCode rc = this->mJsonParser->verifyAndGetSize(fSysConfigPropsFileName, jsonSysConfigPropsRoot, size);

    if(RC_IS_OK(rc)) {
        this->mJsonParser->parse(std::bind(&SysConfigProcessor::SysConfigsParserCB, this, std::placeholders::_1));
    } else {
        LOGE("URM_SYSCONFIG_PROCESSOR", "Couldn't parse: " + fSysConfigPropsFileName);
    }

    return rc;
}

void SysConfigProcessor::SysConfigsParserCB(const Json::Value& item) {
    std::string propKey;
    std::string propVal;

    if(item[PROP_NAME].isString()) {
        propKey = item[PROP_NAME].asString();
    }

    if(item[PROP_VALUE].isString()) {
        propVal = item[PROP_VALUE].asString();
    }

    if(!SysConfigPropRegistry::getInstance()->createProperty(propKey, propVal)) {
        LOGE("URM_SYSCONFIG_PROCESSOR", "Property is malformed or Property with name = " + propKey + " already exists in the Registry");
    }
}

SysConfigProcessor::~SysConfigProcessor() {
    delete this->mJsonParser;
    this->mJsonParser = nullptr;
}
