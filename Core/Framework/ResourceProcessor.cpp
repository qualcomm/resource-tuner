// Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
// SPDX-License-Identifier: BSD-3-Clause-Clear

#include <iostream>

#include "ResourceProcessor.h"
#include <cstring>


ResourceProcessor::ResourceProcessor(const std::string& yamlFilePath) {
    if(yamlFilePath.length() == 0) {
        // No Custom Resources File Specified
        this->mCustomResourceFileSpecified = false;
        mResourceConfigYamlFilePath = RESOURCE_CONFIGS_FILE;
    } else {
        this->mCustomResourceFileSpecified = true;
        mResourceConfigYamlFilePath = yamlFilePath;
    }
}

ErrCode ResourceProcessor::parseResourceConfigs() {
    const std::string fTargetResourcesName(mResourceConfigYamlFilePath);

    YAML::Node result;
    ErrCode rc = YamlParser::parse(fTargetResourcesName, result);

    if(RC_IS_OK(rc)) {
        if(result[RESOURCE_CONFIGS_ROOT].IsDefined() && result[RESOURCE_CONFIGS_ROOT].IsSequence()) {
            int32_t resourceCount = result[RESOURCE_CONFIGS_ROOT].size();
            ResourceRegistry::getInstance()->initRegistry(this->mCustomResourceFileSpecified);

            for(int32_t i = 0; i < result[RESOURCE_CONFIGS_ROOT].size(); i++) {
                YAML::Node resourceConfig = result[RESOURCE_CONFIGS_ROOT][i];
                try {
                    LOGI("RTN_RESOURCE_PROCESSOR", "Parsing resource at index = " + std::to_string(i));
                    parseYamlNode(resourceConfig);
                } catch(const std::invalid_argument& e) {
                    LOGE("RTN_RESOURCE_PROCESSOR", "Error parsing Resource Config: " + std::string(e.what()));
                } catch(const std::bad_alloc& e) {
                    LOGE("RTN_RESOURCE_PROCESSOR", "Error parsing Resource Config: " + std::string(e.what()));
                }
            }
        }
    }

    return rc;
}

int32_t readFromNode(const std::string& fName) {
    std::ifstream myFile(fName, std::ios::in);
    std::string value;

    if(!myFile.is_open()) {
        LOGE("RTN_RESOURCE_PROCESSOR", "Failed to open file: " + fName + " Error: " + std::strerror(errno));
        return 0;
    }

    if(!getline(myFile, value)) {
        LOGE("RTN_RESOURCE_PROCESSOR", "Failed to read from file: " + fName);
        return 0;
    }

    myFile.close();

    try {
        int32_t result = stoi(value);
        LOGD("RTN_RESOURCE_PROCESSOR", "Read value " + std::to_string(result) + " from file: " + fName);
        return result;
    } catch(const std::invalid_argument& e) {
        LOGE("RTN_RESOURCE_PROCESSOR", "Invalid integer in file: " + fName + " Content: " + value);
    } catch(const std::out_of_range& e) {
        LOGE("RTN_RESOURCE_PROCESSOR", "Integer out of range in file: " + fName + " Content: " + value);
    }

    return 0;
}

void ResourceProcessor::parseYamlNode(const YAML::Node& item) {
    ResourceConfigInfoBuilder resourceConfigInfoBuilder;

    // No Defaults Available, a Resource with Invalid OpType is considered Malformed
    resourceConfigInfoBuilder.setOptype(
        safeExtract<std::string>(item[RESOURCE_CONFIGS_ELEM_RESOURCE_TYPE], "-1")
    );

    // No Defaults Available, a Resource with Invalid OpId is considered Malformed
    resourceConfigInfoBuilder.setOpcode(
        safeExtract<std::string>(item[RESOURCE_CONFIGS_ELEM_RESOURCE_ID], "-1")
    );

    // Defaults to false
    resourceConfigInfoBuilder.setSupported(
        safeExtract<bool>(item[RESOURCE_CONFIGS_ELEM_SUPPORTED], false)
    );

    // Defaults to an empty string
    resourceConfigInfoBuilder.setName(
        safeExtract<std::string>(item[RESOURCE_CONFIGS_ELEM_RESOURCENAME], "")
    );

    int32_t defaultValue = readFromNode(
        safeExtract<std::string>(item[RESOURCE_CONFIGS_ELEM_RESOURCENAME], "")
    );

    // Defaults to 0
    resourceConfigInfoBuilder.setDefaultValue(defaultValue);

    // No Defaults Available, a Resource with Invalid HT is considered Malformed
    resourceConfigInfoBuilder.setHighThreshold(
        safeExtract<int32_t>(item[RESOURCE_CONFIGS_ELEM_HIGHTHRESHOLD], -1)
    );

    // No Defaults Available, a Resource with Invalid LT is considered Malformed
    resourceConfigInfoBuilder.setLowThreshold(
        safeExtract<int32_t>(item[RESOURCE_CONFIGS_ELEM_LOWTHRESHOLD], -1)
    );

    // Default to a Value of Third Party
    resourceConfigInfoBuilder.setPermissions(
        safeExtract<std::string>(item[RESOURCE_CONFIGS_ELEM_PERMISSIONS], "")
    );

    // Defaults to a Value of DISPLAY_ON
    if(isList(item[RESOURCE_CONFIGS_ELEM_MODES])) {
        for(const auto& mode : item[RESOURCE_CONFIGS_ELEM_MODES]) {
            resourceConfigInfoBuilder.setModes(safeExtract<std::string>(mode, ""));
        }
    } else {
        resourceConfigInfoBuilder.setModes("");
    }

    // Defaults to LAZY_APPLY
    resourceConfigInfoBuilder.setPolicy(
        safeExtract<std::string>(item[RESOURCE_CONFIGS_ELEM_POLICY], "")
    );

    // Defaults to false
    resourceConfigInfoBuilder.setCoreLevelConflict(
        safeExtract<bool>(item[RESOURCE_CONFIGS_ELEM_CORE_LEVEL_CONFLICT], false)
    );

    // Defaults to APPLY_GLOBAL
    resourceConfigInfoBuilder.setApplyType(
        safeExtract<std::string>(item[RESOURCE_CONFIGS_APPLY_TYPE], "")
    );

    ResourceRegistry::getInstance()->registerResource(resourceConfigInfoBuilder.build());
}
