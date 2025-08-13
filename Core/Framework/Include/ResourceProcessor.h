// Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
// SPDX-License-Identifier: BSD-3-Clause-Clear

#ifndef RESOURCE_PROCESSOR_H
#define RESOURCE_PROCESSOR_H

#include <string>
#include <vector>
#include <fstream>
#include <memory>
#include <sstream>
#include <cerrno>
#include <stdexcept>

#include "YamlParser.h"
#include "Logger.h"
#include "ResourceRegistry.h"
#include "Utils.h"

#define RESOURCE_CONFIGS_FILE "/etc/ResourceTuner/ResourceConfigs.yaml"

#define RESOURCE_CONFIGS_ROOT "ResourceConfigs"
#define RESOURCE_CONFIGS_ELEM_RESOURCE_TYPE "ResType"
#define RESOURCE_CONFIGS_ELEM_RESOURCE_ID "ResID"
#define RESOURCE_CONFIGS_ELEM_RESOURCENAME "Name"
#define RESOURCE_CONFIGS_ELEM_SUPPORTED "Supported"
#define RESOURCE_CONFIGS_ELEM_HIGHTHRESHOLD "HighThreshold"
#define RESOURCE_CONFIGS_ELEM_LOWTHRESHOLD "LowThreshold"
#define RESOURCE_CONFIGS_ELEM_PERMISSIONS "Permissions"
#define RESOURCE_CONFIGS_ELEM_MODES "Modes"
#define RESOURCE_CONFIGS_ELEM_POLICY "Policy"
#define RESOURCE_CONFIGS_ELEM_CORE_LEVEL_CONFLICT "CoreLevelConflict"
#define RESOURCE_CONFIGS_APPLY_TYPE "ApplyType"

/**
 * @brief ResourceProcessor
 * @details Responsible for Parsing the ResourceConfig (YAML) file.
 *          Note, this class uses the YamlParser class for actually Reading and
 *          Parsing the YAML data.
 *
 * The configuration file must follow a specific structure.
 * Example YAML configuration:
 * @code{.yaml}
 * ResourceConfigs:
 *   - ResType: "0x1"
 *     ResID: "0x0"
 *     Name: "/proc/sys/kernel/sched_util_clamp_min"
 *     Supported: true
 *     HighThreshold: 1024,
 *     LowThreshold: 0
 *     Permissions: "third_party"
 *     Modes:
 *     - "display_on"
 *     - "doze"
 *     Policy: "higher_is_better"
 *     CoreLevelConflict: false
 * @endcode
 *
 * @example Resource_Configs
 * This example shows the expected YAML format for Resource configuration.
*/
class ResourceProcessor {
private:
    std::string mResourceConfigYamlFilePath;
    int8_t mCustomResourceFileSpecified;

    void parseYamlNode(const YAML::Node& result);

public:
    ResourceProcessor(const std::string& yamlFilePath);

    ErrCode parseResourceConfigs();
};

#endif
