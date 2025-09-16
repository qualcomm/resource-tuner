// Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
// SPDX-License-Identifier: BSD-3-Clause-Clear

#include "ConfigProcessor.h"

ErrCode ConfigProcessor::parseResourceConfigYamlNode(const std::string& filePath, int8_t isBuSpecified) {
    FILE* resourceConfigFile = fopen(filePath.c_str(), "r");
    if(resourceConfigFile == nullptr) {
        return RC_FILE_NOT_FOUND;
    }

    yaml_parser_t parser;
    yaml_event_t event;

    if(!yaml_parser_initialize(&parser)) {
        fclose(resourceConfigFile);
        return RC_YAML_PARSING_ERROR;
    }

    yaml_parser_set_input_file(&parser, resourceConfigFile);

    int8_t parsingDone = false;
    int8_t insideResourcesConfig = false;
    int8_t parsingResource = false;
    int8_t inModesList = false;

    std::string currentKey;
    std::string value;
    ErrCode rc = RC_SUCCESS;

    ResourceConfigInfoBuilder* resourceConfigInfoBuilder = nullptr;

    while(!parsingDone) {
        if(!yaml_parser_parse(&parser, &event)) {
            return RC_YAML_PARSING_ERROR;
        }
        switch(event.type) {
            case YAML_STREAM_END_EVENT:
                parsingDone = true;
                break;
            case YAML_MAPPING_START_EVENT:
                if(!insideResourcesConfig) {
                    insideResourcesConfig = true;
                } else {
                    // Individual Resource Config
                    resourceConfigInfoBuilder = new(std::nothrow) ResourceConfigInfoBuilder();
                    if(resourceConfigInfoBuilder == nullptr) {
                        return RC_YAML_PARSING_ERROR;
                    }
                    parsingResource = true;
                }
                break;
            case YAML_MAPPING_END_EVENT:
                if(parsingResource) {
                    if(RC_IS_NOTOK(rc)) {
                        // Invalid Resource
                        resourceConfigInfoBuilder->setResType("0");
                    }

                    ResourceRegistry::getInstance()->
                        registerResource(resourceConfigInfoBuilder->build(), isBuSpecified);

                    delete resourceConfigInfoBuilder;
                    parsingResource = false;
                }
                break;

            case YAML_SEQUENCE_START_EVENT:
                if(currentKey == RESOURCE_CONFIGS_ELEM_MODES) {
                    inModesList = true;
                }
                break;

            case YAML_SEQUENCE_END_EVENT:
                if(inModesList) {
                    currentKey.clear();
                    inModesList = false;
                }
                break;

            case YAML_SCALAR_EVENT:
                if(event.data.scalar.value != nullptr) {
                    value = reinterpret_cast<char*>(event.data.scalar.value);
                }

                if(value == RESOURCE_CONFIGS_ROOT) {
                    break;
                }

                if(currentKey.length() == 0) {
                    currentKey = value;
                } else {
                    if(inModesList) {
                        rc = resourceConfigInfoBuilder->setModes(value);
                        break;
                    } else {
                        if(currentKey == RESOURCE_CONFIGS_ELEM_RESOURCE_TYPE) {
                            rc = resourceConfigInfoBuilder->setResType(value);
                        } else if(currentKey == RESOURCE_CONFIGS_ELEM_RESOURCE_ID) {
                            rc = resourceConfigInfoBuilder->setResID(value);
                        } else if(currentKey == RESOURCE_CONFIGS_ELEM_RESOURCENAME) {
                            rc = resourceConfigInfoBuilder->setName(value);
                        } else if(currentKey == RESOURCE_CONFIGS_ELEM_RESOURCEPATH) {
                            rc = resourceConfigInfoBuilder->setPath(value);
                        } else if(currentKey == RESOURCE_CONFIGS_ELEM_SUPPORTED) {
                            rc = resourceConfigInfoBuilder->setSupported(value);
                        } else if(currentKey == RESOURCE_CONFIGS_ELEM_HIGHTHRESHOLD) {
                            rc = resourceConfigInfoBuilder->setHighThreshold(value);
                        } else if(currentKey == RESOURCE_CONFIGS_ELEM_LOWTHRESHOLD) {
                            rc = resourceConfigInfoBuilder->setLowThreshold(value);
                        } else if(currentKey == RESOURCE_CONFIGS_ELEM_PERMISSIONS) {
                            rc = resourceConfigInfoBuilder->setPermissions(value);
                        } else if(currentKey == RESOURCE_CONFIGS_ELEM_POLICY) {
                            rc = resourceConfigInfoBuilder->setPolicy(value);
                        } else if(currentKey == RESOURCE_CONFIGS_APPLY_TYPE) {
                            rc = resourceConfigInfoBuilder->setApplyType(value);
                        }
                    }
                    currentKey.clear();
                }
                break;

            default:
                break;
        }

        yaml_event_delete(&event);
    }

    yaml_parser_delete(&parser);
    fclose(resourceConfigFile);

    return rc;
}

ErrCode ConfigProcessor::parsePropertiesConfigYamlNode(const std::string& filePath) {
    FILE* propConfigFile = fopen(filePath.c_str(), "r");
    if(propConfigFile == nullptr) {
        return RC_FILE_NOT_FOUND;
    }

    yaml_parser_t parser;
    yaml_event_t event;

    if(!yaml_parser_initialize(&parser)) {
        fclose(propConfigFile);
        return RC_YAML_PARSING_ERROR;
    }

    yaml_parser_set_input_file(&parser, propConfigFile);

    int8_t parsingDone = false;
    int8_t isKey = false;

    std::string currentKey = "";
    std::string currentValue = "";
    std::string value;

    while(!parsingDone) {
        if(!yaml_parser_parse(&parser, &event)) {
            return RC_YAML_PARSING_ERROR;
        }

        switch(event.type) {
            case YAML_STREAM_END_EVENT:
                parsingDone = true;
                break;

            case YAML_MAPPING_END_EVENT:
                if(currentKey.length() > 0 && currentValue.length() > 0) {
                    PropertiesRegistry::getInstance()->createProperty(currentKey, currentValue);
                }

                currentKey.clear();
                currentValue.clear();
                break;

            case YAML_SCALAR_EVENT:
                if(event.data.scalar.value != nullptr) {
                    value = reinterpret_cast<char*>(event.data.scalar.value);
                }

                if(value == PROPERTIES_CONFIG_ROOT) {
                    break;
                } else if(value == PROP_NAME) {
                    isKey = true;
                    break;
                } else if(value == PROP_VALUE) {
                    isKey = false;
                    break;
                }

                if(isKey) {
                    currentKey = value;
                } else {
                    currentValue = value;
                }
                break;

            default:
                break;
        }

        yaml_event_delete(&event);
    }

    yaml_parser_delete(&parser);
    fclose(propConfigFile);

    return RC_SUCCESS;
}

ErrCode ConfigProcessor::parseTargetConfigYamlNode(const std::string& filePath) {
    FILE* targetConfigFile = fopen(filePath.c_str(), "r");
    if(targetConfigFile == nullptr) {
        return RC_FILE_NOT_FOUND;
    }

    yaml_parser_t parser;
    yaml_event_t event;

    if(!yaml_parser_initialize(&parser)) {
        fclose(targetConfigFile);
        return RC_YAML_PARSING_ERROR;
    }

    yaml_parser_set_input_file(&parser, targetConfigFile);

    int8_t parsingDone = false;
    int8_t insideTargetConfigs = false;
    int8_t inTargetNamesList = false;
    int8_t inClusterInfoList = false;
    int8_t inClusterSpreadList = false;
    int8_t isConfigForCurrentTarget = false;

    std::string value;
    std::string currentKey;
    std::string currentValue;

    while(!parsingDone) {
        if(!yaml_parser_parse(&parser, &event)) {
            return RC_YAML_PARSING_ERROR;
        }

        switch(event.type) {
            case YAML_STREAM_END_EVENT:
                parsingDone = true;
                break;

            case YAML_SEQUENCE_START_EVENT:
                if(currentKey == TARGET_NAME_LIST) {
                    inTargetNamesList = true;
                } else if(currentKey == TARGET_CLUSTER_INFO) {
                    inClusterInfoList = true;
                    currentKey.clear();
                } else if(currentKey == TARGET_CLUSTER_SPREAD) {
                    inClusterSpreadList = true;
                    currentKey.clear();
                }
                break;

            case YAML_SEQUENCE_END_EVENT:
                if(inTargetNamesList) {
                    inTargetNamesList = false;
                } else if(inClusterInfoList) {
                    inClusterInfoList = false;
                } else if(inClusterSpreadList) {
                    inClusterSpreadList = false;
                }

                currentKey.clear();
                currentValue.clear();
                break;

            case YAML_MAPPING_START_EVENT:
                if(!insideTargetConfigs) {
                    insideTargetConfigs = true;
                }
                break;

            case YAML_MAPPING_END_EVENT:
                if(inClusterInfoList) {
                    if(isConfigForCurrentTarget) {
                        TargetRegistry::getInstance()->addClusterMapping(currentKey, currentValue);
                    }
                } else if(inClusterSpreadList) {
                    if(isConfigForCurrentTarget) {
                        TargetRegistry::getInstance()->addClusterSpreadInfo(currentKey, currentValue);
                    }
                }

                currentKey.clear();
                currentValue.clear();
                break;

            case YAML_SCALAR_EVENT:
                if(event.data.scalar.value != nullptr) {
                    value = reinterpret_cast<char*>(event.data.scalar.value);
                }

                // Skip Labels
                if(value == TARGET_CONFIGS_ROOT) {
                    break;
                }

                if(value == TARGET_CLUSTER_INFO_LOGICAL_ID || value == TARGET_CLUSTER_INFO_PHYSICAL_ID) {
                    break;
                }

                if(value == TARGET_CLUSTER_INFO_PHYSICAL_ID || value == TARGET_PER_CLUSTER_CORE_COUNT) {
                    break;
                }

                if(currentKey.length() == 0) {
                    currentKey = value;
                } else {
                    currentValue = value;
                }

                if(inTargetNamesList) {
                    if(value == "*" || value == ResourceTunerSettings::targetConfigs.targetName) {
                        isConfigForCurrentTarget = true;
                    }
                }
                break;

            default:
                break;
        }
        yaml_event_delete(&event);
    }

    yaml_parser_delete(&parser);
    fclose(targetConfigFile);

    return RC_SUCCESS;
}

ErrCode ConfigProcessor::parseInitConfigYamlNode(const std::string& filePath) {
    FILE* initConfigFile = fopen(filePath.c_str(), "r");
    if(initConfigFile == nullptr) {
        return RC_FILE_NOT_FOUND;
    }

    yaml_parser_t parser;
    yaml_event_t event;

    if(!yaml_parser_initialize(&parser)) {
        fclose(initConfigFile);
        return RC_YAML_PARSING_ERROR;
    }

    yaml_parser_set_input_file(&parser, initConfigFile);

    ErrCode rc = RC_SUCCESS;

    int8_t parsingDone = false;
    int8_t inClusterMapList = false;
    int8_t inCgroupInfoList = false;
    int8_t inMpamInfoList = false;
    int8_t inCacheInfoList = false;

    std::string value;
    std::string currentKey;
    std::string currentValue;

    CGroupConfigInfoBuilder* cGroupConfigBuilder;
    MpamGroupConfigInfoBuilder* mpamGroupConfigBuilder;
    CacheInfoBuilder* cacheInfoBuilder;

    while(!parsingDone) {
        if(!yaml_parser_parse(&parser, &event)) {
            return RC_YAML_PARSING_ERROR;
        }

        switch(event.type) {
            case YAML_STREAM_END_EVENT:
                parsingDone = true;
                break;

            case YAML_SEQUENCE_START_EVENT:
                if(currentKey == INIT_CONFIGS_CGROUPS_LIST) {
                    inCgroupInfoList = true;
                } else if(currentKey == INIT_CONFIGS_MPAM_GROUPS_LIST) {
                    inMpamInfoList = true;
                } else if(currentKey == INIT_CONFIGS_CACHE_INFO_LIST) {
                    inCacheInfoList = true;
                } else if(currentKey == INIT_CONFIGS_CLUSTER_MAP) {
                    inClusterMapList = true;
                }

                break;

            case YAML_SEQUENCE_END_EVENT:
                if(inCgroupInfoList) {
                    inCgroupInfoList = false;
                } else if(inMpamInfoList) {
                    inMpamInfoList = false;
                } else if(inCacheInfoList) {
                    inCacheInfoList = false;
                } else if(inClusterMapList) {
                    inClusterMapList = false;
                }

                currentKey.clear();
                break;

            case YAML_MAPPING_START_EVENT:
                if(inCgroupInfoList) {
                    cGroupConfigBuilder = new (std::nothrow) CGroupConfigInfoBuilder;
                } else if(inMpamInfoList) {
                    mpamGroupConfigBuilder = new (std::nothrow) MpamGroupConfigInfoBuilder;
                } else if(inCacheInfoList) {
                    cacheInfoBuilder = new (std::nothrow) CacheInfoBuilder;
                }

                currentKey.clear();
                break;

            case YAML_MAPPING_END_EVENT:
                if(inCgroupInfoList) {
                    if(RC_IS_NOTOK(rc)) {
                        // Set the ID to -1, so that the Cgroup is not added and is cleaned up
                        cGroupConfigBuilder->setCGroupID("-1");
                    }

                    TargetRegistry::getInstance()->addCGroupMapping(cGroupConfigBuilder->build());
                    delete cGroupConfigBuilder;

                } else if(inMpamInfoList) {
                    if(RC_IS_NOTOK(rc)) {
                        // Set the ID to -1, so that the Cgroup is not added and is cleaned up
                        mpamGroupConfigBuilder->setLgcID("-1");
                    }

                    TargetRegistry::getInstance()->addMpamGroupMapping(mpamGroupConfigBuilder->build());
                    delete mpamGroupConfigBuilder;

                } else if(inCacheInfoList) {
                    if(RC_IS_NOTOK(rc)) {
                        cacheInfoBuilder->setType("");
                        cacheInfoBuilder->setNumBlocks("-1");
                    }

                    TargetRegistry::getInstance()->addCacheInfoMapping(cacheInfoBuilder->build());
                    delete cacheInfoBuilder;
                }

                break;

            case YAML_SCALAR_EVENT:
                if(event.data.scalar.value != nullptr) {
                    value = reinterpret_cast<char*>(event.data.scalar.value);
                }

                // Skip Labels
                if(value == INIT_CONFIGS_ROOT) {
                    break;
                }

                if(inClusterMapList) {
                    break;
                }

                if(currentKey.length() == 0) {
                    currentKey = value;
                } else {
                    if(inCgroupInfoList) {
                        if(currentKey == INIT_CONFIGS_CGROUP_NAME) {
                            rc = cGroupConfigBuilder->setCGroupName(value);
                        } else if(currentKey == INIT_CONFIGS_CGROUP_IDENTIFIER) {
                            rc = cGroupConfigBuilder->setCGroupID(value);
                        } else if(currentKey == INIT_CONFIGS_CGROUP_CREATION) {
                            rc = cGroupConfigBuilder->setCreationNeeded(value);
                        } else if(currentKey == INIT_CONFIGS_CGROUP_THREADED) {
                            rc = cGroupConfigBuilder->setThreaded(value);
                        }

                    } else if(inMpamInfoList) {
                        if(currentKey == INIT_CONFIGS_MPAM_GROUP_NAME) {
                            rc = mpamGroupConfigBuilder->setName(value);
                        } else if(currentKey == INIT_CONFIGS_MPAM_GROUP_ID) {
                            rc = mpamGroupConfigBuilder->setLgcID(value);
                        } else if(currentKey == INIT_CONFIGS_MPAM_GROUP_PRIORITY) {
                            rc = mpamGroupConfigBuilder->setPriority(value);
                        }

                    } else if(inCacheInfoList) {
                        if(currentKey == INIT_CONFIGS_CACHE_INFO_CACHE_TYPE) {
                            rc = cacheInfoBuilder->setType(value);
                        } else if(currentKey == INIT_CONFIGS_CACHE_INFO_CACHE_BLOCK_COUNT) {
                            cacheInfoBuilder->setNumBlocks(value);
                        } else if(currentKey == INIT_CONFIGS_CACHE_INFO_CACHE_PRIORITY_AWARE) {
                            rc = cacheInfoBuilder->setPriorityAware(value);
                        }
                    }
                    currentKey.clear();
                }

                break;
        }
        yaml_event_delete(&event);
    }

    yaml_parser_delete(&parser);
    fclose(initConfigFile);

    return RC_SUCCESS;
}

ErrCode ConfigProcessor::parseResourceConfigs(const std::string& filePath, int8_t isBuSpecified) {
    return parseResourceConfigYamlNode(filePath, isBuSpecified);
}

ErrCode ConfigProcessor::parsePropertiesConfigs(const std::string& filePath) {
    return parsePropertiesConfigYamlNode(filePath);
}

ErrCode ConfigProcessor::parseTargetConfigs(const std::string& filePath) {
    return parseTargetConfigYamlNode(filePath);
}

ErrCode ConfigProcessor::parseInitConfigs(const std::string& filePath) {
    return parseInitConfigYamlNode(filePath);
}

ErrCode ConfigProcessor::parse(ConfigType configType, const std::string& filePath, int8_t isBuSpecified) {
    ErrCode rc = RC_SUCCESS;

    switch(configType) {
        case ConfigType::RESOURCE_CONFIG: {
            rc = this->parseResourceConfigs(filePath, isBuSpecified);
            break;
        }
        case ConfigType::PROPERTIES_CONFIG: {
            rc = this->parsePropertiesConfigs(filePath);
            break;
        }
        case ConfigType::TARGET_CONFIG: {
            rc = this->parseTargetConfigs(filePath);
            break;
        }
        case ConfigType::INIT_CONFIG: {
            rc = this->parseInitConfigs(filePath);
            break;
        }
        default: {
            rc = RC_BAD_ARG;
            break;
        }
    }
    return rc;
}
