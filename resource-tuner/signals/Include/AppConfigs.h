// Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
// SPDX-License-Identifier: BSD-3-Clause-Clear

#ifndef APP_CONFIG_REGISTRY_H
#define APP_CONFIG_REGISTRY_H

#include <cstdint>
#include <string>
#include <unordered_map>

#include "ErrCodes.h"

typedef struct {
    std::string mAppName;
    int32_t mNumThreads;
    std::string* mThreadNameList;
    int32_t* mCGroupIds;
    int32_t mNumSignals;
    uint32_t* mSignalCodes;
} AppConfig;

class AppConfigs {
private:
    std::unordered_map<std::string, AppConfig*> mAppConfig;

public:
    void registerAppConfig(AppConfig* appConfig);
    AppConfig* getAppConfig(const std::string& appName);
};

class AppConfigBuilder {
private:
    AppConfig* mAppConfig;

public:
    AppConfigBuilder();

    ErrCode setAppName(const std::string& name);
    ErrCode setNumThreads(int32_t count);
    ErrCode addThreadMapping(const std::string& threadName, const std::string& cGroup);
    ErrCode setNumSigCodes(int32_t sigCount);
    ErrCode addSigCode(const std::string& sigCodeStr);
};

#endif
