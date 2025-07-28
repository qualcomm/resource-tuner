// Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
// SPDX-License-Identifier: BSD-3-Clause-Clear

#include "TargetConfigProcessor.h"

TargetConfigProcessor::TargetConfigProcessor(std::string jsonFile) {
    this->mJsonParser = new (std::nothrow) JsonParser();
    if(this->mJsonParser == nullptr) {
        LOGE("URM_TARGET_CONFIG_PROCESSOR", "SysConfig Properties Parsing Failed");
    }

    if(jsonFile.length() == 0) {
        // No Custom Target Config File Specified
        this->mTargetConfigJsonFilePath = TARGET_CONFIGS_FILE;
    } else {
        this->mTargetConfigJsonFilePath = jsonFile;
    }
}

ErrCode TargetConfigProcessor::parseTargetConfigs() {
    if(this->mJsonParser == nullptr) {
        return RC_JSON_PARSING_ERROR;
    }

    const std::string fTargetConfigFileName(this->mTargetConfigJsonFilePath);
    const std::string jsonTargeteConfigRoot(TARGET_CONFIGS_ROOT);

    int32_t size;
    ErrCode rc = this->mJsonParser->verifyAndGetSize(fTargetConfigFileName, jsonTargeteConfigRoot, size);

    if(RC_IS_OK(rc)) {
        this->mJsonParser->parse(std::bind(&TargetConfigProcessor::TargetConfigCB, this, std::placeholders::_1));
    } else {
        LOGE("URM_TARGET_CONFIG_PROCESSOR", "Failed to parse target configs file");
    }

    return rc;
}

void TargetConfigProcessor::TargetConfigCB(const Json::Value& item) {
    if(item[TARGET_NAME].isString()) {
        TargetRegistry::getInstance()->setTargetName(item[TARGET_NAME].asString());
    }

    if(item[TARGET_TOTAL_CORE_COUNT].isInt()) {
        TargetRegistry::getInstance()->setTotalCoreCount((uint8_t)item[TARGET_TOTAL_CORE_COUNT].asInt());
    }

    if(item[TARGET_CLUSTER_INFO].isArray()) {
        for(int i = 0; i < item[TARGET_CLUSTER_INFO].size(); i++) {
            const Json::Value& clusterInfo = item[TARGET_CLUSTER_INFO][i];
            int8_t id;
            std::string clusterType;

            if(clusterInfo[TARGET_CONFIGS_ID].isInt()) {
                id = static_cast<int8_t>(clusterInfo[TARGET_CONFIGS_ID].asInt());
            }

            if(clusterInfo[TARGET_CONFIGS_TYPE].isString()) {
                clusterType = clusterInfo[TARGET_CONFIGS_TYPE].asString();
            }

            TargetRegistry::getInstance()->addMapping(clusterType, id);
        }
    }

    if(item[TARGET_CLUSTER_SPREAD].isArray()) {
        for(int i = 0; i < item[TARGET_CLUSTER_SPREAD].size(); i++) {
            const Json::Value& clusterSpread = item[TARGET_CLUSTER_SPREAD][i];
            int8_t id;
            int32_t numCores;

            if(clusterSpread[TARGET_CONFIGS_ID].isInt()) {
                id = static_cast<int8_t>(clusterSpread[TARGET_CONFIGS_ID].asInt());
            }

            if(clusterSpread[TARGET_PER_CLUSTER_CORE_COUNT].isInt()) {
                numCores = clusterSpread[TARGET_PER_CLUSTER_CORE_COUNT].asInt();
            }

            TargetRegistry::getInstance()->addClusterSpreadInfo(id, numCores);
        }
    }
}

TargetConfigProcessor::~TargetConfigProcessor() {
    if(this->mJsonParser != nullptr) {
        delete this->mJsonParser;
        this->mJsonParser = nullptr;
    }
}
