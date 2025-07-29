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

class TargetConfigProcessor {
private:
    JsonParser* mJsonParser;
    std::string mTargetConfigJsonFilePath;

    void TargetConfigCB(const Json::Value& item);

public:
    TargetConfigProcessor(std::string jsonFile);
    ~TargetConfigProcessor();

    ErrCode parseTargetConfigs();
};

#endif
