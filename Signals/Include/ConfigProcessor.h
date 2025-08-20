// Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
// SPDX-License-Identifier: BSD-3-Clause-Clear

#ifndef SIGNAL_CONFIG_PROCESSOR_H
#define SIGNAL_CONFIG_PROCESSOR_H

#include <iostream>
#include <memory>

#include "YamlParser.h"
#include "SignalRegistry.h"
#include "ExtFeaturesRegistry.h"

#define SIGNAL_CONFIGS_ROOT "SignalConfigs"
#define SIGNAL_SIGID "SigId"
#define SIGNAL_CATEGORY "Category"
#define SIGNAL_NAME "Name"
#define SIGNAL_TIMEOUT "Timeout"
#define SIGNAL_ENABLE "Enable"
#define SIGNAL_TARGETS_ENABLED "TargetsEnabled"
#define SIGNAL_TARGETS_DISABLED "TargetsDisabled"
#define SIGNAL_PERMISSIONS "Permissions"
#define SIGNAL_DERIVATIVES "Derivatives"
#define SIGNAL_RESOURCES "Resources"

#define SIGNAL_RESOURCE_CODE "ResCode"
#define SIGNAL_RESINFO "ResInfo"
#define SIGNAL_VALUES "Values"

#define EXT_FEATURES_CONFIGS_ROOT "ExtFeaturesConfigs"
#define EXT_FEATURE_ID "FeatureId"
#define EXT_FEATURE_LIB "Lib"
#define EXT_FEATURE_SIGNAL_RANGE "Range"
#define EXT_FEATURE_SIGNAL_INDIVIDUAL "Individual"

/**
 * The Signal configuration file (SignalsConfig.yaml) must follow a specific structure.
 * Example YAML configuration:
 * @code{.yaml}
 * SignalConfigs:
 *  - SigId: "0x0000"
 *    Category: "0x01"
 *    Name: INSTALL
 *    Enable: true
 *    Permissions: ["system", "third_party"]
 *    Timeout: 4000
 *    Resources:
 *      - {ResCode: "0x80030000", ResInfo: "0x00000000", Values: [700]}
 *      - {ResCode: "0x80040001", ResInfo: "0x00000702", Values: [1388256]}
 *      - {ResCode: "0x80040102", ResInfo: "0x00000104", Values: [1344100, 1590871]}
 * @endcode
 *
 * @example Signal_Configs
 * This example shows the expected YAML format for Signal configuration.
 * For Information on each of these fields as well as some which have been omitted from
 * this example, refer Config Files Format Documentation.
*/

class ConfigProcessor {
private:
    void parseSignalConfigYamlNode(const YAML::Node& result, int8_t isBuSpecified);
    void parseExtFeatureConfigYamlNode(const YAML::Node& result);

public:
    ErrCode parseSignalConfigs(const std::string& filePath, int8_t isBuSpecified=false);
    ErrCode parseExtFeaturesConfigs(const std::string& filePath);
};

#endif
