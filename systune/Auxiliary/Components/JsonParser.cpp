// Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
// SPDX-License-Identifier: BSD-3-Clause-Clear

#include "JsonParser.h"
#include "Logger.h"

JsonParser::JsonParser() {}

JsonParser::~JsonParser() {}

ErrCode JsonParser::verifyAndGetSize(const std::string& fJsonFileName, const std::string& fJsonRootName, int32_t& size) {
    std::ifstream file(fJsonFileName, std::ifstream::binary);
    if(!file.is_open()) {
        LOGE("URM_JSON_PARSER", "Unable to open file: " + fJsonFileName);
        return RC_FILE_NOT_FOUND;
    }

    Json::CharReaderBuilder builder;
    builder["collectComments"] = false; //Ignore all comments!
    std::string errors;

    if(!Json::parseFromStream(builder, file, &mRoot, &errors)) {
        LOGE("URM_JSON_PARSER",  "Failed to parse JSON: " + errors);
        return RC_JSON_PARSING_ERROR;
    }

    this->mJsonRootName = fJsonRootName;
    if(!mRoot.isMember(mJsonRootName)) {
        LOGE("URM_JSON_PARSER","Root is not present");
        return RC_INVALID_JSON;
    }

    std::vector<std::string> list = mRoot.getMemberNames();

    if(list.size() != 1) {
        LOGE("URM_JSON_PARSER","Multiple roots present");
        return RC_INVALID_JSON;
    }

    if(!mRoot[fJsonRootName].isArray()) {
        LOGE("URM_JSON_PARSER", "Root is not an array");
        return RC_INVALID_JSON;
    }

    size = mRoot[mJsonRootName].size();
    return RC_SUCCESS;
}

int32_t JsonParser::parse(std::function<void(const Json::Value&)> callback) {
    for(const auto& element : mRoot[mJsonRootName]) {
        callback(element);
    }
    LOGD("URM_JSON_PARSER",
         "Parsing of " + mJsonRootName + " finished!");

    return 0;
}
