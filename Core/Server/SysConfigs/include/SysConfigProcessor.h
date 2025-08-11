// Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
// SPDX-License-Identifier: BSD-3-Clause-Clear

#ifndef SYSCONFIG_PROCESSOR_H
#define SYSCONFIG_PROCESSOR_H

#include <memory>

#include "SysConfigPropRegistry.h"
#include "Logger.h"
#include "YamlParser.h"

#define SYS_CONFIGS_ROOT "PropertyConfigs"
#define PROP_NAME "Name"
#define PROP_VALUE "Value"

#define SYS_CONFIGS_PROPS_FILE "/etc/ResourceTuner/PropertiesConfigs.yaml"

/**
 * @brief SysConfigProcessor
 * @details Responsible for Parsing the SysConfig (YAML) file.
 *          Note, this class uses the YamlParser class for actually Reading and
 *          Parsing the YAML data.
 *
 * The configuration file must follow a specific structure.
 * Example YAML configuration:
 * @code{.yaml}
 * PropertyConfigs:
 *   - Name: "resource_tuner.maximum.concurrent.requests"
 *     Value: "60"
 *
 *   - Name: "resource_tuner.maximum.resources.per.request"
 *     Value: "64"
 *
 *   - Name: "resource_tuner.listening.port"
 *     Value: "12000"
 *
 *   - Name: "resource_tuner.pulse.duration"
 *     Value: "60000"
 *
 *   - Name: "resource_tuner.garbage_collection.duration"
 *     Value: "83000"
 *
 *   - Name: "resource_tuner.rate_limiter.delta"
 *     Value: "5"
 *
 *   - Name: "resource_tuner.penalty.factor"
 *     Value: "2.0"
 *
 *   - Name: "resource_tuner.reward.factor"
 *     Value: "0.4"
 * @endcode
 *
 * @example Properties_Configs
 * This example shows the expected YAML format for SysConfig configuration.
*/
class SysConfigProcessor {
private:
    static std::shared_ptr<SysConfigProcessor> sysConfigProcessorInstance;
    std::string mPropertiesConfigYamlFilePath;

    SysConfigProcessor(const std::string& yamlFilePath);

    void parseYamlNode(const YAML::Node& result);

public:
    ErrCode parseSysConfigs();

    static std::shared_ptr<SysConfigProcessor> getInstance(const std::string& yamlFilePath = "") {
        if(sysConfigProcessorInstance == nullptr) {
            std::shared_ptr<SysConfigProcessor> localSysConfigProcessorInstance(new SysConfigProcessor(yamlFilePath));
            localSysConfigProcessorInstance.swap(sysConfigProcessorInstance);
        }
        return sysConfigProcessorInstance;
    }
};

#endif
