// Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
// SPDX-License-Identifier: BSD-3-Clause-Clear

#include "SysConfigProcessor.h"

std::shared_ptr<SysConfigProcessor> SysConfigProcessor::sysConfigProcessorInstance = nullptr;

SysConfigProcessor::SysConfigProcessor(const std::string& yamlFilePath) {
    if(yamlFilePath.length() == 0) {
        // No Custom Properties File Specified
        mPropertiesConfigYamlFilePath = SYS_CONFIGS_PROPS_FILE;
    } else {
        mPropertiesConfigYamlFilePath = yamlFilePath;
    }
}

ErrCode SysConfigProcessor::parseSysConfigs() {
    const std::string fSysConfigPropsFileName(mPropertiesConfigYamlFilePath);

    YAML::Node result;
    ErrCode rc = YamlParser::parse(fSysConfigPropsFileName, result);

    if(RC_IS_OK(rc)) {
        if(result[SYS_CONFIGS_ROOT] && result[SYS_CONFIGS_ROOT].IsSequence()) {
            for(const auto& sysConfigEle : result[SYS_CONFIGS_ROOT]) {
                parseYamlNode(sysConfigEle);
            }
        }
    }

    return rc;
}

void SysConfigProcessor::parseYamlNode(const YAML::Node& item) {
    std::string propKey;
    std::string propVal;

    try {
        propKey = safeExtract<std::string>(item[PROP_NAME]);
        propVal = safeExtract<std::string>(item[PROP_VALUE]);

    } catch(const std::invalid_argument& e) {
        return;
    }

    if(!SysConfigPropRegistry::getInstance()->createProperty(propKey, propVal)) {
        LOGE("URM_SYSCONFIG_PROCESSOR", "Property is malformed or Property with name = " + propKey + " already exists in the Registry");
    }
}
