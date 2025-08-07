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
            ResourceRegistry::getInstance()->initRegistry(resourceCount, this->mCustomResourceFileSpecified);

            for(const auto& resourceConfig : result[RESOURCE_CONFIGS_ROOT]) {
                try {
                    parseYamlNode(resourceConfig);
                } catch(const std::invalid_argument& e) {
                    LOGE("URM_RESOURCE_PROCESSOR", "Error parsing Resource Config: " + std::string(e.what()));
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
        LOGE("URM_RESOURCE_PROCESSOR", "Failed to open file: " + fName + " Error: " + std::strerror(errno));
        return 0;
    }

    if(!getline(myFile, value)) {
        LOGE("URM_RESOURCE_PROCESSOR", "Failed to read from file: " + fName);
        return 0;
    }

    myFile.close();

    try {
        int32_t result = stoi(value);
        LOGD("URM_RESOURCE_PROCESSOR", "Read value " + std::to_string(result) + " from file: " + fName);
        return result;
    } catch(const std::invalid_argument& e) {
        LOGE("URM_RESOURCE_PROCESSOR", "Invalid integer in file: " + fName + " Content: " + value);
    } catch(const std::out_of_range& e) {
        LOGE("URM_RESOURCE_PROCESSOR", "Integer out of range in file: " + fName + " Content: " + value);
    }

    return 0;
}

void ResourceProcessor::parseYamlNode(const YAML::Node& item) {
    ResourceConfigInfoBuilder resourceConfigInfoBuilder;

    resourceConfigInfoBuilder.setOptype(
        safeExtract<std::string>(item[RESOURCE_CONFIGS_ELEM_RESOURCE_TYPE])
    );

    resourceConfigInfoBuilder.setOpcode(
        safeExtract<std::string>(item[RESOURCE_CONFIGS_ELEM_RESOURCE_ID])
    );

    resourceConfigInfoBuilder.setSupported(
        safeExtract<bool>(item[RESOURCE_CONFIGS_ELEM_SUPPORTED])
    );

    resourceConfigInfoBuilder.setName(
        safeExtract<std::string>(item[RESOURCE_CONFIGS_ELEM_RESOURCENAME])
    );

    int32_t defaultValue = readFromNode(
        safeExtract<std::string>(item[RESOURCE_CONFIGS_ELEM_RESOURCENAME])
    );

    resourceConfigInfoBuilder.setDefaultValue(defaultValue);

    resourceConfigInfoBuilder.setHighThreshold(
        safeExtract<int32_t>(item[RESOURCE_CONFIGS_ELEM_HIGHTHRESHOLD])
    );

    resourceConfigInfoBuilder.setLowThreshold(
        safeExtract<int32_t>(item[RESOURCE_CONFIGS_ELEM_LOWTHRESHOLD])
    );

    resourceConfigInfoBuilder.setPermissions(
        safeExtract<std::string>(item[RESOURCE_CONFIGS_ELEM_PERMISSIONS])
    );

    if(isList(item[RESOURCE_CONFIGS_ELEM_MODES])) {
        for(const auto& mode : item[RESOURCE_CONFIGS_ELEM_MODES]) {
            resourceConfigInfoBuilder.setModes(safeExtract<std::string>(mode));
        }
    }

    resourceConfigInfoBuilder.setPolicy(
        safeExtract<std::string>(item[RESOURCE_CONFIGS_ELEM_POLICY])
    );

    resourceConfigInfoBuilder.setCoreLevelConflict(
        safeExtract<bool>(item[RESOURCE_CONFIGS_ELEM_CORE_LEVEL_CONFLICT])
    );

    ResourceRegistry::getInstance()->registerResource(resourceConfigInfoBuilder.build());
}
