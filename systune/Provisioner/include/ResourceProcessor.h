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

#include "JsonParser.h"
#include "ResourceRegistry.h"
#include "Utils.h"

#define RESOURCE_CONFIGS_FILE "../Configs/resourceConfigs.json"

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

/**
 * @brief Handles processing of the resource config files. It stores them in a vector where
 *        each config is represented by a struct ResourceConfigInfo.
 *
 */
class ResourceProcessor {
private:
    std::string mResourceConfigJsonFilePath;
    JsonParser* mJsonParser;
    int8_t mCustomResourceFileSpecified;

    /**
     * @brief callback function for processing json file targetResourceConfig.
     */
    void TargetResourcesCB(const Json::Value& item);

public:
    ResourceProcessor(std::string jsonFilePath);
    ~ResourceProcessor();

    ErrCode parseResourceConfigs();
};

#endif
