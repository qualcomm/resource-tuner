// Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
// SPDX-License-Identifier: BSD-3-Clause-Clear

#ifndef TARGET_CONFIG_PROCESSOR_H
#define TARGET_CONFIG_PROCESSOR_H

#include <string>
#include <vector>
#include <unordered_map>
#include <memory>

#include "JsonParser.h"
#include "Logger.h"
#include "TargetRegistry.h"

#define TARGET_CONFIGS_ROOT "TargetConfig"
#define TARGET_NAME "TargetName"
#define TARGET_CLUSTER_INFO "ClusterInfo"
#define TARGET_CLUSTER_SPREAD "ClusterSpread"
#define TARGET_PER_CLUSTER_CORE_COUNT "NumCores"
#define TARGET_TOTAL_CORE_COUNT "TotalCoreCount"
#define TARGET_CONFIGS_ID "Id"
#define TARGET_CONFIGS_TYPE "Type"

#define TARGET_CONFIGS_FILE "/etc/systune/targetConfigs.json"

/**
 * @brief TargetConfigProcessor
 * @details Responsible for Parsing the Target Config (JSON) file.
 *          Note, this class uses the JsonParser class for actually Reading and
 *          Parsing the JSON data.
 *
 * The configuration file must follow a specific structure.
 * Example JSON configuration:
 * @code{.json}
 *{
 *  "TargetName": "<>",
 *  "ClusterInfo" : [
 *      {"Id": 0, "Type": "big"},
 *      {"Id": 1, "Type": "little"},
 *      {"Id": 2, "Type": "prime"},
 *      {"Id": 3, "Type": "titanium"}
 *  ],
 *  "ClusterSpread" : [
 *      {"Id": 0, "NumCores": 4},
 *      {"Id": 1, "NumCores": 4},
 *      {"Id": 2, "NumCores": 4},
 *      {"Id": 3, "NumCores": 4}
 *  ],
 *  "TotalCoreCount": 16
 *}
 * @endcode
 *
 * @example Target_Configs
 * This example shows the expected JSON format for Target configuration.
*/
class TargetConfigProcessor {
private:
    JsonParser* mJsonParser;
    std::string mTargetConfigJsonFilePath;

    void TargetConfigCB(const Json::Value& item);

public:
    TargetConfigProcessor(const std::string& jsonFile);
    ~TargetConfigProcessor();

    ErrCode parseTargetConfigs();
};

#endif
