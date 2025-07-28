// Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
// SPDX-License-Identifier: BSD-3-Clause-Clear

#ifndef SYSCONFIG_PROCESSOR_H
#define SYSCONFIG_PROCESSOR_H

#include <memory>

#include "SysConfigPropRegistry.h"
#include "Logger.h"
#include "JsonParser.h"

#define SYS_CONFIGS_ROOT "PropertyConfigs"
#define PROP_NAME "Name"
#define PROP_VALUE "Value"

#define SYS_CONFIGS_PROPS_FILE "../Configs/propertiesConfigs.json"

class SysConfigProcessor {
private:
    static std::shared_ptr<SysConfigProcessor> sysConfigProcessorInstance;
    std::string mPropertiesConfigJsonFilePath;
    JsonParser* mJsonParser;

    void SysConfigsParserCB(const Json::Value& item);

    SysConfigProcessor(std::string jsonFilePath);

public:
    ~SysConfigProcessor();

    ErrCode parseSysConfigs();

    static std::shared_ptr<SysConfigProcessor> getInstance(std::string jsonFilePath = "") {
        if(sysConfigProcessorInstance == nullptr) {
            std::shared_ptr<SysConfigProcessor> localSysConfigProcessorInstance(new SysConfigProcessor(jsonFilePath));
            localSysConfigProcessorInstance.swap(sysConfigProcessorInstance);
        }
        return sysConfigProcessorInstance;
    }
};

#endif
