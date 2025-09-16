// Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
// SPDX-License-Identifier: BSD-3-Clause-Clear

#include "ConfigProcessor.h"

#define ADD_TO_RESOURCE_BUILDER(KEY, METHOD)                    \
    if(topKey == KEY && resourceConfigInfoBuilder != nullptr) { \
        if(RC_IS_OK(rc)) {                                      \
            rc = resourceConfigInfoBuilder->METHOD(value);      \
        }                                                       \
        break;                                                  \
    }

#define ADD_TO_CGROUP_BUILDER(KEY, METHOD)                      \
    if(topKey == KEY && cGroupConfigBuilder != nullptr) {       \
        if(RC_IS_OK(rc)) {                                      \
            rc = cGroupConfigBuilder->METHOD(value);            \
        }                                                       \
        break;                                                  \
    }

#define ADD_TO_MPAM_GROUP_BUILDER(KEY, METHOD)                  \
    if(topKey == KEY && mpamGroupConfigBuilder != nullptr) {    \
        if(RC_IS_OK(rc)) {                                      \
            rc = mpamGroupConfigBuilder->METHOD(value);         \
        }                                                       \
        break;                                                  \
    }

#define ADD_TO_CACHE_INFO_BUILDER(KEY, METHOD)                  \
    if(topKey == KEY && cacheInfoBuilder != nullptr) {          \
        if(RC_IS_OK(rc)) {                                      \
            rc = cacheInfoBuilder->METHOD(value);               \
        }                                                       \
        break;                                                  \
    }

static int8_t isKey(const std::string& keyName) {
    if(keyName == TARGET_CONFIGS_ROOT) return true;
    if(keyName == TARGET_NAME_LIST) return true;
    if(keyName == TARGET_CLUSTER_INFO) return true;
    if(keyName == TARGET_CLUSTER_INFO_LOGICAL_ID) return true;
    if(keyName == TARGET_CLUSTER_INFO_PHYSICAL_ID) return true;
    if(keyName == TARGET_CLUSTER_SPREAD) return true;
    if(keyName == TARGET_PER_CLUSTER_CORE_COUNT) return true;

    if(keyName == RESOURCE_CONFIGS_ROOT) return true;
    if(keyName == RESOURCE_CONFIGS_ELEM_RESOURCE_TYPE) return true;
    if(keyName == RESOURCE_CONFIGS_ELEM_RESOURCE_ID) return true;
    if(keyName == RESOURCE_CONFIGS_ELEM_RESOURCENAME) return true;
    if(keyName == RESOURCE_CONFIGS_ELEM_RESOURCEPATH) return true;
    if(keyName == RESOURCE_CONFIGS_ELEM_SUPPORTED) return true;
    if(keyName == RESOURCE_CONFIGS_ELEM_HIGHTHRESHOLD) return true;
    if(keyName == RESOURCE_CONFIGS_ELEM_LOWTHRESHOLD) return true;
    if(keyName == RESOURCE_CONFIGS_ELEM_PERMISSIONS) return true;
    if(keyName == RESOURCE_CONFIGS_ELEM_MODES) return true;
    if(keyName == RESOURCE_CONFIGS_ELEM_POLICY) return true;
    if(keyName == RESOURCE_CONFIGS_APPLY_TYPE) return true;

    if(keyName == INIT_CONFIGS_ROOT) return true;
    if(keyName == INIT_CONFIGS_CGROUPS_LIST) return true;
    if(keyName == INIT_CONFIGS_CGROUP_NAME) return true;
    if(keyName == INIT_CONFIGS_CGROUP_IDENTIFIER) return true;
    if(keyName == INIT_CONFIGS_CGROUP_CREATION) return true;
    if(keyName == INIT_CONFIGS_CGROUP_THREADED) return true;

    if(keyName == INIT_CONFIGS_CLUSTER_MAP) return true;
    if(keyName == INIT_CONFIGS_CLUSTER_MAP_CLUSTER_ID) return true;
    if(keyName == INIT_CONFIGS_CLUSTER_MAP_CLUSTER_TYPE) return true;

    if(keyName == INIT_CONFIGS_MPAM_GROUPS_LIST) return true;
    if(keyName == INIT_CONFIGS_MPAM_GROUP_NAME) return true;
    if(keyName == INIT_CONFIGS_MPAM_GROUP_ID) return true;
    if(keyName == INIT_CONFIGS_MPAM_GROUP_PRIORITY) return true;

    if(keyName == INIT_CONFIGS_CACHE_INFO_LIST) return true;
    if(keyName == INIT_CONFIGS_CACHE_INFO_CACHE_TYPE) return true;
    if(keyName == INIT_CONFIGS_CACHE_INFO_CACHE_BLOCK_COUNT) return true;
    if(keyName == INIT_CONFIGS_CACHE_INFO_CACHE_PRIORITY_AWARE) return true;

    return false;
}

ErrCode ConfigProcessor::parseResourceConfigYamlNode(const std::string& filePath, int8_t isBuSpecified) {
    SETUP_LIBYAML_PARSING(filePath);

    ErrCode rc = RC_SUCCESS;

    int8_t parsingDone = false;
    int8_t docMarker = false;
    int8_t parsingResource = false;

    std::string currentKey;
    std::string value;
    std::string topKey;
    std::stack<std::string> keyTracker;

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
                if(!docMarker) {
                    docMarker = true;
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
                    resourceConfigInfoBuilder = nullptr;
                    parsingResource = false;
                }

                break;

            case YAML_SEQUENCE_END_EVENT:
                if(keyTracker.empty()) {
                    return RC_YAML_INVALID_SYNTAX;
                }

                keyTracker.pop();
                break;

            case YAML_SCALAR_EVENT:
                if(event.data.scalar.value != nullptr) {
                    value = reinterpret_cast<char*>(event.data.scalar.value);
                }

                if(isKey(value)) {
                    keyTracker.push(value);
                    break;
                }

                if(keyTracker.empty()) {
                    return RC_YAML_INVALID_SYNTAX;
                }

                topKey = keyTracker.top();
                if(topKey != RESOURCE_CONFIGS_ELEM_MODES) {
                    keyTracker.pop();
                }

                ADD_TO_RESOURCE_BUILDER(RESOURCE_CONFIGS_ELEM_RESOURCE_TYPE, setResType);
                ADD_TO_RESOURCE_BUILDER(RESOURCE_CONFIGS_ELEM_RESOURCE_ID, setResID);
                ADD_TO_RESOURCE_BUILDER(RESOURCE_CONFIGS_ELEM_RESOURCENAME, setName);
                ADD_TO_RESOURCE_BUILDER(RESOURCE_CONFIGS_ELEM_RESOURCEPATH, setPath);
                ADD_TO_RESOURCE_BUILDER(RESOURCE_CONFIGS_ELEM_SUPPORTED, setSupported);
                ADD_TO_RESOURCE_BUILDER(RESOURCE_CONFIGS_ELEM_HIGHTHRESHOLD, setHighThreshold);
                ADD_TO_RESOURCE_BUILDER(RESOURCE_CONFIGS_ELEM_LOWTHRESHOLD, setLowThreshold);
                ADD_TO_RESOURCE_BUILDER(RESOURCE_CONFIGS_ELEM_PERMISSIONS, setPermissions);
                ADD_TO_RESOURCE_BUILDER(RESOURCE_CONFIGS_ELEM_POLICY, setPolicy);
                ADD_TO_RESOURCE_BUILDER(RESOURCE_CONFIGS_APPLY_TYPE, setApplyType);
                ADD_TO_RESOURCE_BUILDER(RESOURCE_CONFIGS_ELEM_MODES, setModes);

                break;

            default:
                break;
        }

        yaml_event_delete(&event);
    }

    TEARDOWN_LIBYAML_PARSING
    return rc;
}

ErrCode ConfigProcessor::parsePropertiesConfigYamlNode(const std::string& filePath) {
    SETUP_LIBYAML_PARSING(filePath);

    int8_t parsingDone = false;
    int8_t isPropName = false;

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
                    isPropName = true;
                    break;
                } else if(value == PROP_VALUE) {
                    isPropName = false;
                    break;
                }

                if(isPropName) {
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

    TEARDOWN_LIBYAML_PARSING
    return RC_SUCCESS;
}

ErrCode ConfigProcessor::parseTargetConfigYamlNode(const std::string& filePath) {
    SETUP_LIBYAML_PARSING(filePath);

    int8_t parsingDone = false;
    std::string value;
    std::string topKey;

    std::stack<std::string> keyTracker;
    std::vector<std::string> valuesArray;

    int8_t isConfigForCurrentTarget = false;

    while(!parsingDone) {
        if(!yaml_parser_parse(&parser, &event)) {
            return RC_YAML_PARSING_ERROR;
        }

        switch(event.type) {
            case YAML_STREAM_END_EVENT:
                parsingDone = true;
                break;

            case YAML_SEQUENCE_END_EVENT:
                if(topKey.empty()) {
                    return RC_YAML_INVALID_SYNTAX;
                }
                topKey = keyTracker.top();

                if(topKey == TARGET_NAME_LIST) {
                    for(int32_t i = 0; i < valuesArray.size(); i ++) {
                        if(valuesArray[i] == "*" || valuesArray[i] == ResourceTunerSettings::targetConfigs.targetName) {
                            isConfigForCurrentTarget = true;
                        }
                    }
                } else if(topKey == TARGET_CLUSTER_INFO) {
                    for(int32_t i = 0; i < valuesArray.size(); i += 2) {
                        std::string logicalIDString = valuesArray[i];
                        std::string physicalIDString = valuesArray[i + 1];

                        if(isConfigForCurrentTarget) {
                            TargetRegistry::getInstance()->addClusterMapping(logicalIDString, physicalIDString);
                        }
                    }
                } else if(topKey == TARGET_CLUSTER_SPREAD) {
                    for(int32_t i = 0; i < valuesArray.size(); i += 2) {
                        std::string physicalIDString = valuesArray[i];
                        std::string numCoresString = valuesArray[i + 1];

                        if(isConfigForCurrentTarget) {
                            TargetRegistry::getInstance()->addClusterSpreadInfo(physicalIDString, numCoresString);
                        }
                    }
                } else {
                    return RC_YAML_INVALID_SYNTAX;
                }

                valuesArray.clear();
                keyTracker.pop();
                break;

            case YAML_SCALAR_EVENT:
                if(event.data.scalar.value != nullptr) {
                    value = reinterpret_cast<char*>(event.data.scalar.value);
                }

                if(isKey(value) && (value == TARGET_CONFIGS_ROOT)
                                && (value == TARGET_NAME_LIST)
                                && (value == TARGET_CLUSTER_INFO)
                                && (value == TARGET_NAME_LIST)
                                &&(value == TARGET_CLUSTER_SPREAD)) {
                    keyTracker.push(value);
                } else {
                    valuesArray.push_back(value);
                }

                break;

            default:
                break;
        }
        yaml_event_delete(&event);
    }

    TEARDOWN_LIBYAML_PARSING
    return RC_SUCCESS;
}

ErrCode ConfigProcessor::parseInitConfigYamlNode(const std::string& filePath) {
    SETUP_LIBYAML_PARSING(filePath);

    ErrCode rc = RC_SUCCESS;

    int8_t parsingDone = false;
    int8_t docMarker = false;

    std::string value;
    std::string topKey;
    std::stack<std::string> keyTracker;

    CGroupConfigInfoBuilder* cGroupConfigBuilder = nullptr;
    MpamGroupConfigInfoBuilder* mpamGroupConfigBuilder = nullptr;
    CacheInfoBuilder* cacheInfoBuilder = nullptr;

    while(!parsingDone) {
        if(!yaml_parser_parse(&parser, &event)) {
            return RC_YAML_PARSING_ERROR;
        }

        switch(event.type) {
            case YAML_STREAM_END_EVENT:
                parsingDone = true;
                break;

            case YAML_SEQUENCE_END_EVENT:
                if(keyTracker.empty()) {
                    return RC_YAML_INVALID_SYNTAX;
                }

                keyTracker.pop();
                break;

            case YAML_MAPPING_START_EVENT:
                if(!docMarker) {
                    docMarker = true;
                } else {
                    if(keyTracker.empty()) {
                        return RC_YAML_INVALID_SYNTAX;
                    }

                    topKey = keyTracker.top();
                    if(topKey == INIT_CONFIGS_CGROUPS_LIST) {
                        cGroupConfigBuilder = new (std::nothrow) CGroupConfigInfoBuilder;
                    } else if(topKey == INIT_CONFIGS_MPAM_GROUPS_LIST) {
                        mpamGroupConfigBuilder = new (std::nothrow) MpamGroupConfigInfoBuilder;
                    } else if(topKey == INIT_CONFIGS_CACHE_INFO_LIST) {
                        cacheInfoBuilder = new (std::nothrow) CacheInfoBuilder;
                    }
                }

                break;

            case YAML_MAPPING_END_EVENT:
                if(keyTracker.empty()) {
                    return RC_YAML_INVALID_SYNTAX;
                }

                topKey = keyTracker.top();
                if(topKey == INIT_CONFIGS_CGROUPS_LIST) {
                    if(RC_IS_NOTOK(rc)) {
                        // Set the ID to -1, so that the Cgroup is not added and is cleaned up
                        cGroupConfigBuilder->setCGroupID("-1");
                    }

                    TargetRegistry::getInstance()->addCGroupMapping(cGroupConfigBuilder->build());

                    delete cGroupConfigBuilder;
                    cGroupConfigBuilder = nullptr;

                } else if(topKey == INIT_CONFIGS_MPAM_GROUPS_LIST) {
                    if(RC_IS_NOTOK(rc)) {
                        // Set the ID to -1, so that the Cgroup is not added and is cleaned up
                        mpamGroupConfigBuilder->setLgcID("-1");
                    }

                    TargetRegistry::getInstance()->addMpamGroupMapping(mpamGroupConfigBuilder->build());

                    delete mpamGroupConfigBuilder;
                    mpamGroupConfigBuilder = nullptr;

                } else if(topKey == INIT_CONFIGS_CACHE_INFO_LIST) {
                    if(RC_IS_NOTOK(rc)) {
                        cacheInfoBuilder->setType("");
                        cacheInfoBuilder->setNumBlocks("-1");
                    }

                    TargetRegistry::getInstance()->addCacheInfoMapping(cacheInfoBuilder->build());

                    delete cacheInfoBuilder;
                    cacheInfoBuilder = nullptr;
                }

                break;

            case YAML_SCALAR_EVENT:
                if(event.data.scalar.value != nullptr) {
                    value = reinterpret_cast<char*>(event.data.scalar.value);
                }

                if(isKey(value)) {
                    keyTracker.push(value);
                    break;
                }

                if(keyTracker.empty()) {
                    return RC_YAML_INVALID_SYNTAX;
                }

                topKey = keyTracker.top();
                if(topKey != INIT_CONFIGS_CLUSTER_MAP &&
                   topKey !=  INIT_CONFIGS_CGROUPS_LIST &&
                   topKey != INIT_CONFIGS_MPAM_GROUPS_LIST &&
                   topKey != INIT_CONFIGS_CACHE_INFO_LIST) {
                    keyTracker.pop();
                }

                ADD_TO_CGROUP_BUILDER(INIT_CONFIGS_CGROUP_NAME, setCGroupName);
                ADD_TO_CGROUP_BUILDER(INIT_CONFIGS_CGROUP_IDENTIFIER, setCGroupID);
                ADD_TO_CGROUP_BUILDER(INIT_CONFIGS_CGROUP_CREATION, setCreationNeeded);
                ADD_TO_CGROUP_BUILDER(INIT_CONFIGS_CGROUP_THREADED, setThreaded);

                ADD_TO_MPAM_GROUP_BUILDER(INIT_CONFIGS_MPAM_GROUP_NAME, setName);
                ADD_TO_MPAM_GROUP_BUILDER(INIT_CONFIGS_MPAM_GROUP_ID, setLgcID);
                ADD_TO_MPAM_GROUP_BUILDER(INIT_CONFIGS_MPAM_GROUP_PRIORITY, setPriority);

                ADD_TO_CACHE_INFO_BUILDER(INIT_CONFIGS_CACHE_INFO_CACHE_TYPE, setType);
                ADD_TO_CACHE_INFO_BUILDER(INIT_CONFIGS_CACHE_INFO_CACHE_BLOCK_COUNT, setNumBlocks);
                ADD_TO_CACHE_INFO_BUILDER(INIT_CONFIGS_CACHE_INFO_CACHE_PRIORITY_AWARE, setPriorityAware);

                break;

            default:
                break;
        }
        yaml_event_delete(&event);
    }

    TEARDOWN_LIBYAML_PARSING
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
