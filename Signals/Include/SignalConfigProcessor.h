// Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
// SPDX-License-Identifier: BSD-3-Clause-Clear

#ifndef SIGNAL_CONFIG_PROCESSOR_H
#define SIGNAL_CONFIG_PROCESSOR_H

#include <iostream>
#include <memory>

#include "YamlParser.h"
#include "SignalRegistry.h"

#define SIGNAL_CONFIGS_FILE  "/etc/ResourceTuner/SignalConfigs.yaml"
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
#define SIGNAL_OPINFO "OpInfo"
#define SIGNAL_VALUES "Values"

/**
 * @brief SignalConfigProcessor
 * @details Responsible for Parsing the SignalConfig (YAML) file.
 *          Note, this class uses the YamlParser class for actually Reading and
 *          Parsing the YAML data.
 *
 * The configuration file must follow a specific structure.
 * Example YAML configuration:
 * @code{.yaml}
 * SignalConfigs:
 *   - SigId: "0x0"
 *     Category: "0x1"
 *     Name: INSTALL
 *     Enable: true
 *     TargetsEnabled:
 *       - sun
 *       - moon
 *     Permissions:
 *       - system
 *       - third_party
 *     Derivatives:
 *       - solar
 *     Timeout: 4000
 *     Resources:
 *       - 65536
 *       - 700
 * @endcode
 *
 * @example Signal_Configs
 * This example shows the expected YAML format for Signal configuration.
*/
class SignalConfigProcessor {
private:
    std::string mSignalConfigYamlFilePath;
    int8_t mCustomSignalsFileSpecified;

    void parseYamlNode(const YAML::Node& result);

public:
    SignalConfigProcessor(const std::string& yamlFilePath);

    ErrCode parseSignalConfigs();
};

#endif
