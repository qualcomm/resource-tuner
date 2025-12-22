// Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
// SPDX-License-Identifier: BSD-3-Clause-Clear

#include "AppConfigs.h"

void AppConfigs::registerAppConfig(AppConfig* appConfig) {
    this->mAppConfig[appConfig->mAppName] = appConfig;
}

AppConfig* AppConfigs::getAppConfig(const std::string& name) {
    return this->mAppConfig[name];
}

AppConfigBuilder::AppConfigBuilder() {
    this->mAppConfig = new(std::nothrow) AppConfig();
}

ErrCode AppConfigBuilder::setAppName(const std::string& name) {
    if(this->mAppConfig == nullptr) {
        return RC_MEMORY_ALLOCATION_FAILURE;
    }

    this->mAppConfig->mAppName = name;
}

ErrCode AppConfigBuilder::setNumThreads(int32_t threadCnt) {
    if(this->mAppConfig == nullptr) {
        return RC_MEMORY_ALLOCATION_FAILURE;
    }

    try {
        this->mAppConfig->mNumThreads = threadCnt;
        this->mAppConfig->mThreadNameList = new std::string[this->mAppConfig->mNumThreads];
        this->mAppConfig->mCGroupIds = new std::int32_t[this->mAppConfig->mNumThreads];
        return RC_SUCCESS;

    } catch(const std::exception& e) {
        return RC_INVALID_VALUE;
    }

    return RC_INVALID_VALUE;
}

ErrCode AppConfigBuilder::addThreadMapping(const std::string& threadName, const std::string& cGroup) {
    if(this->mAppConfig == nullptr || this->mAppConfig->mThreadNameList == nullptr) {
        return RC_MEMORY_ALLOCATION_FAILURE;
    }

    static int32_t threadIndex = 0;

    try {
        this->mAppConfig->mThreadNameList[threadIndex] = threadName;
        this->mAppConfig->mCGroupIds[threadIndex] = std::stoi(cGroup);
        threadIndex++;

        return RC_SUCCESS;

    } catch(const std::exception& e) {
        return RC_INVALID_VALUE;
    }

    return RC_INVALID_VALUE;
}

ErrCode AppConfigBuilder::setNumSigCodes(int32_t count) {
    if(this->mAppConfig == nullptr) {
        return RC_MEMORY_ALLOCATION_FAILURE;
    }

    this->mAppConfig->mNumSignals = count;
    return RC_SUCCESS;
}

ErrCode AppConfigBuilder::addSigCode(const std::string& sigCodeStr) {
    if(this->mAppConfig == nullptr || this->mAppConfig->mSignalCodes == nullptr) {
        return RC_MEMORY_ALLOCATION_FAILURE;
    }

    static int32_t signalIndex = 0;

    try {
        uint32_t sigCode = (uint32_t)stol(sigCodeStr, nullptr, 0);
        this->mAppConfig->mSignalCodes[signalIndex++] = sigCode;
        return RC_SUCCESS;

    } catch(const std::exception& e) {
        return RC_INVALID_VALUE;
    }

    return RC_INVALID_VALUE;
}
