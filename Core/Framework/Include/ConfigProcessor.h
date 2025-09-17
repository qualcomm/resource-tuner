// Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
// SPDX-License-Identifier: BSD-3-Clause-Clear

#ifndef SERVER_CONFIG_PROCESSOR_H
#define SERVER_CONFIG_PROCESSOR_H

#include <string>
#include <vector>
#include <stack>
#include <fstream>
#include <memory>
#include <sstream>
#include <cerrno>
#include <stdexcept>

#include "YamlParser.h"
#include "Logger.h"
#include "ResourceRegistry.h"
#include "PropertiesRegistry.h"
#include "TargetRegistry.h"
#include "Utils.h"
#include "AuxRoutines.h"

// Resource Config
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
#define RESOURCE_CONFIGS_ELEM_APPLY_TYPE "ApplyType"

// Target Info Config
#define TARGET_CONFIGS_ROOT "TargetConfig"
#define TARGET_NAME_LIST "TargetName"
#define TARGET_CLUSTER_INFO "ClusterInfo"
#define TARGET_CLUSTER_INFO_LOGICAL_ID "LgcId"
#define TARGET_CLUSTER_INFO_PHYSICAL_ID "PhyId"
#define TARGET_CLUSTER_SPREAD "ClusterSpread"
#define TARGET_PER_CLUSTER_CORE_COUNT "NumCores"

// CGroup Config
#define INIT_CONFIGS_ROOT "InitConfigs"
#define INIT_CONFIGS_ELEM_CGROUPS_LIST "CgroupsInfo"
#define INIT_CONFIGS_ELEM_CGROUP_NAME "Name"
#define INIT_CONFIGS_ELEM_CGROUP_IDENTIFIER "ID"
#define INIT_CONFIGS_ELEM_CGROUP_CREATION "Create"
#define INIT_CONFIGS_ELEM_CGROUP_THREADED "IsThreaded"

// Cluster Map
#define INIT_CONFIGS_ELEM_CLUSTER_MAP "ClusterMap"
#define INIT_CONFIGS_ELEM_CLUSTER_MAP_CLUSTER_ID "Id"
#define INIT_CONFIGS_ELEM_CLUSTER_MAP_CLUSTER_TYPE "Type"

// Mpam Config
#define INIT_CONFIGS_ELEM_MPAM_GROUPS_LIST "MPAMgroupsInfo"
#define INIT_CONFIGS_ELEM_MPAM_GROUP_NAME "Name"
#define INIT_CONFIGS_ELEM_MPAM_GROUP_ID "ID"
#define INIT_CONFIGS_ELEM_MPAM_GROUP_PRIORITY "Priority"

// Cache Info
#define INIT_CONFIGS_ELEM_CACHE_INFO_LIST "CacheInfo"
#define INIT_CONFIGS_ELEM_CACHE_INFO_TYPE "Type"
#define INIT_CONFIGS_ELEM_CACHE_INFO_BLK_CNT "NumCacheBlocks"
#define INIT_CONFIGS_ELEM_CACHE_INFO_PRIO_AWARE "PriorityAware"

// Properties
#define PROPERTY_CONFIGS_ROOT "PropertyConfigs"
#define PROPERTY_CONFIGS_ELEM_NAME "Name"
#define PROPERTY_CONFIGS_ELEM_VALUE "Value"

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
 *   - TargetName: ["QCS9100"]
 *     ClusterInfo:
 *       - LgcId: 0
 *         PhyId: 4
 *       - LgcId: 1
 *         PhyId: 0
 *       - LgcId: 2
 *         PhyId: 9
 *       - LgcId: 3
 *         PhyId: 7
 *     ClusterSpread:
 *       - PhyId: 0
 *         NumCores: 4
 *       - PhyId: 4
 *         NumCores: 3
 *       - PhyId: 7
 *         NumCores: 2
 *       - PhyId: 9
 *         NumCores: 1
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
 *   # Logical IDs should always be arranged from lower to higher cluster capacities
 *   - ClusterMap:
 *     - Id: 0
 *       Type: little
 *     - Id: 1
 *       Type: big
 *     - Id: 2
 *       Type: prime
 *
 *   - CgroupsInfo:
 *     - Name: "camera-cgroup"
 *       ID: 0
 *     - Name: "audio-cgroup"
 *       Create: true
 *       ID: 1
 *     - Name: "video-cgroup"
 *       IsThreaded: true
 *       ID: 2
 *
 *   - MPAMgroupsInfo:
 *     - Name: "camera-mpam-group"
 *       ID: 0
 *       Priority: 0
 *     - Name: "audio-mpam-group"
 *       ID: 1
 *       Priority: 1
 *     - Name: "video-mpam-group"
 *       ID: 2
 *       Priority: 2
 *
 *   - CacheInfo:
 *     - Type: L2
 *       NumCacheBlocks: 2
 *       PriorityAware: 0
 *     - Type: L3
 *       NumCacheBlocks: 1
 *       PriorityAware: 1
 *
 * @endcode
 *
 * @example Init_Configs
 * This example shows the expected YAML format for Init Configuration, which includes any
 * applicable Control group, Mpam group Creation Information.
 */

/**
 * The Properties Config file (PropertiesConfig.yaml) must follow a specific structure.
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
 * This example shows the expected YAML format for Properties configuration.
 */

/**
 * @brief ConfigProcessor
 * @details Responsible for Parsing the ResourceConfig file,
 *          InitConfig and TargetConfig (if provided) YAML files.
 *          Note, this class uses the YamlParser class for actually Reading and
 *          Parsing the YAML data.
 */
class ConfigProcessor {
private:
    ErrCode parseResourceConfigYamlNode(const std::string& filePath, int8_t isBuSpecified);
    ErrCode parsePropertiesConfigYamlNode(const std::string& filePath);
    ErrCode parseInitConfigYamlNode(const std::string& filePath);
    ErrCode parseTargetConfigYamlNode(const std::string& filePath);

public:
    ErrCode parseResourceConfigs(const std::string& filePath, int8_t isBuSpecified=false);
    ErrCode parsePropertiesConfigs(const std::string& filePath);
    ErrCode parseInitConfigs(const std::string& filePath);
    ErrCode parseTargetConfigs(const std::string& filePath);
    ErrCode parse(ConfigType configType, const std::string& filePath, int8_t isBuSpecified=false);
};

#endif
