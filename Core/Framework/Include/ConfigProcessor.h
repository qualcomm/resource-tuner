// Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
// SPDX-License-Identifier: BSD-3-Clause-Clear

#ifndef SERVER_CONFIG_PROCESSOR_H
#define SERVER_CONFIG_PROCESSOR_H

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
#include "TargetRegistry.h"
#include "Utils.h"
#include "AuxRoutines.h"

#define RESOURCE_CONFIGS_ROOT "ResourceConfigs"
#define RESOURCE_CONFIGS_ELEM_RESOURCE_TYPE "ResType"
#define RESOURCE_CONFIGS_ELEM_RESOURCE_ID "ResID"
#define RESOURCE_CONFIGS_ELEM_RESOURCENAME "Name"
#define RESOURCE_CONFIGS_ELEM_RESOURCEPATH "Path"
#define RESOURCE_CONFIGS_ELEM_SUPPORTED "Supported"
#define RESOURCE_CONFIGS_ELEM_HIGHTHRESHOLD "HighThreshold"
#define RESOURCE_CONFIGS_ELEM_LOWTHRESHOLD "LowThreshold"
#define RESOURCE_CONFIGS_ELEM_PERMISSIONS "Permissions"
#define RESOURCE_CONFIGS_ELEM_MODES "Modes"
#define RESOURCE_CONFIGS_ELEM_POLICY "Policy"
#define RESOURCE_CONFIGS_APPLY_TYPE "ApplyType"

#define TARGET_CONFIGS_ROOT "TargetConfig"
#define TARGET_NAME "TargetName"
#define TARGET_CLUSTER_INFO "ClusterInfo"
#define TARGET_CLUSTER_INFO_LOGICAL_ID "LgcId"
#define TARGET_CLUSTER_INFO_PHYSICAL_ID "PhyId"
#define TARGET_CLUSTER_SPREAD "ClusterSpread"
#define TARGET_PER_CLUSTER_CORE_COUNT "NumCores"
#define TARGET_TOTAL_CORE_COUNT "TotalCoreCount"
#define TARGET_CONFIGS_ID "Id"
#define TARGET_CONFIGS_TYPE "Type"

#define INIT_CONFIGS_ROOT "InitConfigs"
#define INIT_CONFIGS_CGROUPS_LIST "CgroupsInfo"
#define INIT_CONFIGS_CGROUP_NAME "Name"
#define INIT_CONFIGS_CGROUP_IDENTIFIER "ID"
#define INIT_CONFIGS_CGROUP_THREADED "IsThreaded"

/**
 * The Resource Config file (ResourcesConfig.yaml) must follow a specific structure.
 * Example YAML configuration:
 * @code{.yaml}
 * ResourceConfigs:
 *   - ResType: "0x03"
 *     ResID: "0x0000"
 *     Name: "SCHED_UTIL_CLAMP_MIN"
 *     Path: "/proc/sys/kernel/sched_util_clamp_min"
 *     Supported: true
 *     HighThreshold: 1024
 *     LowThreshold: 0
 *     Permissions: "third_party"
 *     Modes: ["display_on", "doze"]
 *     Policy: "lower_is_better"
 *     ApplyType: "global"
 * @endcode
 *
 * @example Resource_Configs
 * This example shows the expected YAML format for Resource configuration.
*/

/**
 * The Target Config file (TargetConfig.yaml) must follow a specific structure.
 * Example YAML configuration:
 * @code{.yaml}
 * TargetConfig:
 *  - TargetName: QCS9100
 *    # overide physical clustermap
 *    ClusterInfo:
 *      - LgcId: 0
 *        PhyId: 1
 *      - LgcId: 1
 *        PhyId: 0
 *      - LgcId: 2
 *        PhyId: 2
 *      - LgcId: 3
 *        PhyId: 3
 *    ClusterSpread:
 *      - PhyId: 0
 *        NumCores: 4
 *      - PhyId: 1
 *        NumCores: 4
 *      - PhyId: 2
 *        NumCores: 4
 *      - PhyId: 3
 *        NumCores: 4
 * @endcode
 *
 * @example Target_Configs
 * This example shows the expected YAML format for Target configuration.
*/

/**
 * The Init Config file (InitConfig.yaml) must follow a specific structure.
 * Example YAML configuration:
 * @code{.yaml}
 * InitConfigs:
 *   - CgroupsInfo:
 *     - Name: "camera-cgroup"
 *       ID: 0
 *     - Name: "audio-cgroup"
 *       ID: 1
 *     - Name: "video-cgroup"
 *       IsThreaded: true
 *       ID: 2
 *
 * @endcode
 *
 * @example Init_Configs
 * This example shows the expected YAML format for Init Config, which includes any
 * applicable CGroup Creation Information.
*/

/**
 * @brief ConfigProcessor
 * @details Responsible for Parsing the ResourceConfig file,
*           InitConfig and TargetConfig (if provided) YAML files.
 *          Note, this class uses the YamlParser class for actually Reading and
 *          Parsing the YAML data.
*/
class ConfigProcessor {
private:
    void parseResourceConfigYamlNode(const YAML::Node& result, int8_t isBuSpecified);
    void parseInitConfigYamlNode(const YAML::Node& result);
    void parseTargetConfigYamlNode(const YAML::Node& result);

public:
    ErrCode parseResourceConfigs(const std::string& filePath, int8_t isBuSpecified=false);
    ErrCode parseInitConfigs(const std::string& filePath);
    ErrCode parseTargetConfigs(const std::string& filePath);
};

#endif
