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

#define SYS_CONFIGS_PROPS_FILE "/etc/systune/propertiesConfigs.json"

/**
 * @brief SysConfigProcessor
 * @details Responsible for Parsing the SysConfig (JSON) file.
 *          Note, this class uses the JsonParser class for actually Reading and
 *          Parsing the JSON data.
 *
 * The configuration file must follow a specific structure.
 * Example JSON configuration:
 * @code{.json}
 *{
    {"Name": "systune.maximum.concurrent.requests", "Value" : "60"},
    {"Name": "systune.maximum.resources.per.request", "Value" :"64"},
    {"Name": "systune.listening.port", "Value" :"12000"},
    {"Name": "systune.pulse.duration", "Value" : "60000"},
    {"Name": "systune.garbage_collection.duration", "Value" : "83000"}
 *}
 * @endcode
 *
 * @example Properties_Configs
 * This example shows the expected JSON format for SysConfig configuration.
*/
class SysConfigProcessor {
private:
    static std::shared_ptr<SysConfigProcessor> sysConfigProcessorInstance;
    std::string mPropertiesConfigJsonFilePath;
    JsonParser* mJsonParser;

    void SysConfigsParserCB(const Json::Value& item);

    SysConfigProcessor(const std::string& jsonFilePath);

public:
    ~SysConfigProcessor();

    ErrCode parseSysConfigs();

    static std::shared_ptr<SysConfigProcessor> getInstance(const std::string& jsonFilePath = "") {
        if(sysConfigProcessorInstance == nullptr) {
            std::shared_ptr<SysConfigProcessor> localSysConfigProcessorInstance(new SysConfigProcessor(jsonFilePath));
            localSysConfigProcessorInstance.swap(sysConfigProcessorInstance);
        }
        return sysConfigProcessorInstance;
    }
};

#endif
