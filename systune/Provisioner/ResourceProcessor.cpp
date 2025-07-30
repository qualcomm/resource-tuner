// Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
// SPDX-License-Identifier: BSD-3-Clause-Clear

#include "ResourceProcessor.h"
#include "Logger.h"

#include <iostream>

ResourceProcessor::ResourceProcessor(const std::string& jsonFilePath) {
    this->mJsonParser = new (std::nothrow) JsonParser();
    if(this->mJsonParser == nullptr) {
        LOGE("URM_RESOURCE_PROCESSOR", "SysConfig Properties Parsing Failed");
    }

    if(jsonFilePath.length() == 0) {
        // No Custom Resources File Specified
        this->mCustomResourceFileSpecified = false;
        mResourceConfigJsonFilePath = RESOURCE_CONFIGS_FILE;
    } else {
        this->mCustomResourceFileSpecified = true;
        mResourceConfigJsonFilePath = jsonFilePath;
    }
}

ErrCode ResourceProcessor::parseResourceConfigs() {
    if(this->mJsonParser == nullptr) {
        return RC_JSON_PARSING_ERROR;
    }

    const std::string fTargetResourcesName(mResourceConfigJsonFilePath);
    const std::string jsonResourceConfigRoot(RESOURCE_CONFIGS_ROOT);

    int32_t size;
    ErrCode rc = this->mJsonParser->verifyAndGetSize(fTargetResourcesName, jsonResourceConfigRoot, size);

    if(RC_IS_OK(rc)) {
        ResourceRegistry::getInstance()->initRegistry(size, this->mCustomResourceFileSpecified);
        this->mJsonParser->parse(std::bind(&ResourceProcessor::TargetResourcesCB, this, std::placeholders::_1));
    } else {
        LOGE("URM_RESOURCE_PROCESSOR", "Couldn't parse" + fTargetResourcesName);
    }

    if(RC_IS_OK(rc)) {
        int32_t resourcesAllocatedCount = ResourceRegistry::getInstance()->getTotalResourcesCount();
        if(resourcesAllocatedCount != size) {
            LOGE("URM_RESOURCE_CONFIG_PROCESSOR", "Couldn't allocate Memory for all the Resource Configs");
            return RC_MEMORY_ALLOCATION_FAILURE;
        }
    }

    return rc;
}

int32_t readFromNode(const std::string& fName) {
    std::ifstream myFile(fName, std::ios::in);
    std::string value;

    if(!myFile.is_open()) {
        LOGE("URM_RESOURCE_PROCESSOR", "Failed to open file: " + fName + " Error: " + strerror(errno));
        return 0;
    }

    if(!getline(myFile, value)) {
        LOGE("URM_RESOURCE_PROCESSOR", "Failed to read from file: " + fName);
        return 0;
    }

    myFile.close();

    try {
        int32_t result = stoi(value);
        LOGD("URM_RESOURCE_PROCESSOR", "Read value " + std::to_string(result) + " from file: " + fName);
        return result;
    } catch (const std::invalid_argument& e) {
        LOGE("URM_RESOURCE_PROCESSOR", "Invalid integer in file: " + fName + " Content: " + value);
    } catch (const std::out_of_range& e) {
        LOGE("URM_RESOURCE_PROCESSOR", "Integer out of range in file: " + fName + " Content: " + value);
    }

    return 0;
}

void ResourceProcessor::TargetResourcesCB(const Json::Value& item) {
    ResourceConfigInfoBuilder resourceConfigInfoBuilder;

    if(item[RESOURCE_CONFIGS_ELEM_RESOURCE_TYPE].isString()) {
        resourceConfigInfoBuilder.setOptype(item[RESOURCE_CONFIGS_ELEM_RESOURCE_TYPE].asString());
    }

    if(item[RESOURCE_CONFIGS_ELEM_RESOURCE_ID].isString()) {
        resourceConfigInfoBuilder.setOpcode(item[RESOURCE_CONFIGS_ELEM_RESOURCE_ID].asString());
    }

    if(item[RESOURCE_CONFIGS_ELEM_SUPPORTED].isBool()) {
        resourceConfigInfoBuilder.setSupported(item[RESOURCE_CONFIGS_ELEM_SUPPORTED].asBool());
    }

    if(item[RESOURCE_CONFIGS_ELEM_RESOURCENAME].isString()) {
        resourceConfigInfoBuilder.setName(item[RESOURCE_CONFIGS_ELEM_RESOURCENAME].asString());
        resourceConfigInfoBuilder.setDefaultValue(readFromNode(item[RESOURCE_CONFIGS_ELEM_RESOURCENAME].asString()));
    }

    if(item[RESOURCE_CONFIGS_ELEM_HIGHTHRESHOLD].isInt()) {
        resourceConfigInfoBuilder.setHighThreshold(item[RESOURCE_CONFIGS_ELEM_HIGHTHRESHOLD].asInt());
    }

    if(item[RESOURCE_CONFIGS_ELEM_LOWTHRESHOLD].isInt()) {
        resourceConfigInfoBuilder.setLowThreshold(item[RESOURCE_CONFIGS_ELEM_LOWTHRESHOLD].asInt());
    }

    if(item[RESOURCE_CONFIGS_ELEM_PERMISSIONS].isString()) {
        resourceConfigInfoBuilder.setPermissions(item[RESOURCE_CONFIGS_ELEM_PERMISSIONS].asString());
    }

    if(item[RESOURCE_CONFIGS_ELEM_MODES].isArray()) {
        for(const auto& mode : item[RESOURCE_CONFIGS_ELEM_MODES]) {
            if(mode.isString()) {
                resourceConfigInfoBuilder.setModes(mode.asString());
            }
        }
    }

    if(item[RESOURCE_CONFIGS_ELEM_POLICY].isString()) {
        resourceConfigInfoBuilder.setPolicy(item[RESOURCE_CONFIGS_ELEM_POLICY].asString());
    }

    if(item[RESOURCE_CONFIGS_ELEM_CORE_LEVEL_CONFLICT].isBool()) {
        resourceConfigInfoBuilder.setCoreLevelConflict(item[RESOURCE_CONFIGS_ELEM_CORE_LEVEL_CONFLICT].asBool());
    }

    ResourceRegistry::getInstance()->registerResource(resourceConfigInfoBuilder.build());
}

ResourceProcessor::~ResourceProcessor() {
    if(this->mJsonParser != nullptr) {
        delete this->mJsonParser;
        this->mJsonParser = nullptr;
    }
}
