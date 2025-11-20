// Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
// SPDX-License-Identifier: BSD-3-Clause-Clear

#include "SignalConfigProcessor.h"

#define ADD_TO_SIGNAL_BUILDER(KEY, METHOD)                      \
    if(topKey == KEY && signalInfoBuilder != nullptr) {         \
        if(RC_IS_OK(rc)) {                                      \
            rc = signalInfoBuilder->METHOD(value);              \
        }                                                       \
        break;                                                  \
    }

#define ADD_TO_RESOURCE_BUILDER(KEY, METHOD)                    \
    if(topKey == KEY && resourceBuilder != nullptr) {           \
        if(RC_IS_OK(rc)) {                                      \
            rc = resourceBuilder->METHOD(value);                \
        }                                                       \
        break;                                                  \
    }

#define ADD_TO_EXT_FEATURE_BUILDER(KEY, METHOD)                 \
    if(topKey == KEY && extFeatureInfoBuilder != nullptr) {     \
        if(RC_IS_OK(rc)) {                                      \
            rc = extFeatureInfoBuilder->METHOD(value);          \
        }                                                       \
        break;                                                  \
    }

static int8_t isKey(const std::string& keyName) {
    if(keyName == SIGNAL_CONFIGS_ROOT) return true;
    if(keyName == SIGNAL_CONFIGS_ELEM_SIGID) return true;
    if(keyName == SIGNAL_CONFIGS_ELEM_CATEGORY) return true;
    if(keyName == SIGNAL_CONFIGS_ELEM_NAME) return true;
    if(keyName == SIGNAL_CONFIGS_ELEM_TIMEOUT) return true;
    if(keyName == SIGNAL_CONFIGS_ELEM_ENABLE) return true;
    if(keyName == SIGNAL_CONFIGS_ELEM_TARGETS_ENABLED) return true;
    if(keyName == SIGNAL_CONFIGS_ELEM_TARGETS_DISABLED) return true;
    if(keyName == SIGNAL_CONFIGS_ELEM_PERMISSIONS) return true;
    if(keyName == SIGNAL_CONFIGS_ELEM_DERIVATIVES) return true;
    if(keyName == SIGNAL_CONFIGS_ELEM_RESOURCES) return true;
    if(keyName == SIGNAL_CONFIGS_ELEM_RESOURCE_CODE) return true;
    if(keyName == SIGNAL_CONFIGS_ELEM_RESOURCE_RESINFO) return true;
    if(keyName == SIGNAL_CONFIGS_ELEM_RESOURCE_VALUES) return true;

    if(keyName == EXT_FEATURE_CONFIGS_ROOT) return true;
    if(keyName == EXT_FEATURE_CONFIGS_ELEM_ID) return true;
    if(keyName == EXT_FEATURE_CONFIGS_ELEM_LIB) return true;
    if(keyName == EXT_FEATURE_CONFIGS_ELEM_NAME) return true;
    if(keyName == EXT_FEATURE_CONFIGS_ELEM_DESCRIPTION) return true;
    if(keyName == EXT_FEATURE_CONFIGS_ELEM_SUBSCRIBER_LIST) return true;

    return false;
}

static int8_t isKeyTypeList(const std::string& keyName) {
    if(keyName == SIGNAL_CONFIGS_ELEM_TARGETS_ENABLED) return true;
    if(keyName == SIGNAL_CONFIGS_ELEM_TARGETS_DISABLED) return true;
    if(keyName == SIGNAL_CONFIGS_ELEM_PERMISSIONS) return true;
    if(keyName == SIGNAL_CONFIGS_ELEM_DERIVATIVES) return true;
    if(keyName == SIGNAL_CONFIGS_ELEM_RESOURCES) return true;
    if(keyName == SIGNAL_CONFIGS_ELEM_RESOURCE_VALUES) return true;

    if(keyName == EXT_FEATURE_CONFIGS_ELEM_SUBSCRIBER_LIST) return true;

    return false;
}

ErrCode SignalConfigProcessor::parseSignalConfigYamlNode(const std::string& filePath, int8_t isBuSpecified) {
    SETUP_LIBYAML_PARSING(filePath);

    ErrCode rc = RC_SUCCESS;

    int8_t parsingDone = false;
    int8_t docMarker = false;
    int8_t parsingSignal = false;
    int8_t inResourcesMap = false;

    std::string value;
    std::string topKey;
    std::stack<std::string> keyTracker;
    std::vector<std::string> resValues;

    SignalInfoBuilder* signalInfoBuilder = nullptr;
    ResourceBuilder* resourceBuilder = nullptr;

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
                    if(keyTracker.empty()) {
                        return RC_YAML_INVALID_SYNTAX;
                    }

                    topKey = keyTracker.top();
                    if(topKey == SIGNAL_CONFIGS_ELEM_RESOURCES) {
                        inResourcesMap = true;
                        resourceBuilder = new(std::nothrow) ResourceBuilder;
                        if(resourceBuilder == nullptr) {
                            return RC_YAML_PARSING_ERROR;
                        }

                    } else {
                        parsingSignal = true;
                        signalInfoBuilder = new(std::nothrow) SignalInfoBuilder;
                        if(signalInfoBuilder == nullptr) {
                            return RC_YAML_PARSING_ERROR;
                        }
                    }
                }

                break;

            case YAML_MAPPING_END_EVENT:
                if(inResourcesMap) {
                    inResourcesMap = false;
                    if(RC_IS_OK(rc)) {
                        rc = signalInfoBuilder->addResource(resourceBuilder->build());
                        delete resourceBuilder;
                        resourceBuilder = nullptr;
                    }

                } else if(parsingSignal) {
                    parsingSignal = false;

                    if(RC_IS_NOTOK(rc)) {
                        // Set SigCategory so that the Signal gets discarded by Signal Regsitry
                        rc = signalInfoBuilder->setSignalCategory("0");
                    }

                    SignalRegistry::getInstance()->
                        registerSignal(signalInfoBuilder->build(), isBuSpecified);
                    delete signalInfoBuilder;
                    signalInfoBuilder = nullptr;
                }

                break;

            case YAML_SEQUENCE_END_EVENT:
                if(keyTracker.empty()) {
                    return RC_YAML_INVALID_SYNTAX;
                }

                topKey = keyTracker.top();
                keyTracker.pop();

                if(topKey == SIGNAL_CONFIGS_ELEM_RESOURCE_VALUES) {
                    if(RC_IS_OK(rc)) {
                        rc = resourceBuilder->setNumValues(resValues.size());
                    }

                    for(int32_t idx = 0; idx < resValues.size(); idx++) {
                        if(RC_IS_OK(rc)) {
                            rc = resourceBuilder->addValue(idx, resValues[idx]);
                        }
                    }

                    resValues.clear();
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
                if(!isKeyTypeList(topKey)) {
                    keyTracker.pop();
                }

                ADD_TO_SIGNAL_BUILDER(SIGNAL_CONFIGS_ELEM_TARGETS_ENABLED, addTargetEnabled);
                ADD_TO_SIGNAL_BUILDER(SIGNAL_CONFIGS_ELEM_TARGETS_DISABLED, addTargetDisabled);
                ADD_TO_SIGNAL_BUILDER(SIGNAL_CONFIGS_ELEM_PERMISSIONS, addPermission);
                ADD_TO_SIGNAL_BUILDER(SIGNAL_CONFIGS_ELEM_DERIVATIVES, addDerivative);
                ADD_TO_SIGNAL_BUILDER(SIGNAL_CONFIGS_ELEM_SIGID, setSignalID);
                ADD_TO_SIGNAL_BUILDER(SIGNAL_CONFIGS_ELEM_CATEGORY, setSignalCategory);
                ADD_TO_SIGNAL_BUILDER(SIGNAL_CONFIGS_ELEM_TIMEOUT, setTimeout);
                ADD_TO_SIGNAL_BUILDER(SIGNAL_CONFIGS_ELEM_ENABLE, setIsEnabled);

                ADD_TO_RESOURCE_BUILDER(SIGNAL_CONFIGS_ELEM_RESOURCE_CODE, setResCode);
                ADD_TO_RESOURCE_BUILDER(SIGNAL_CONFIGS_ELEM_RESOURCE_RESINFO, setResInfo);

                if(topKey == SIGNAL_CONFIGS_ELEM_RESOURCE_VALUES) {
                    resValues.push_back(value);
                }

                break;

            default:
                break;
        }

        yaml_event_delete(&event);
    }

    TEARDOWN_LIBYAML_PARSING
    return rc;
}

ErrCode SignalConfigProcessor::parseExtFeatureConfigYamlNode(const std::string& filePath) {
    SETUP_LIBYAML_PARSING(filePath);

    ErrCode rc = RC_SUCCESS;

    int8_t parsingDone = false;
    int8_t docMarker = false;
    int8_t parsingFeature = false;

    std::string value;
    std::string topKey;
    std::stack<std::string> keyTracker;

    ExtFeatureInfoBuilder* extFeatureInfoBuilder = nullptr;

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
                    extFeatureInfoBuilder = new(std::nothrow) ExtFeatureInfoBuilder;
                    if(extFeatureInfoBuilder == nullptr) {
                        return RC_YAML_PARSING_ERROR;
                    }

                    parsingFeature = true;
                }
                break;

            case YAML_MAPPING_END_EVENT:
                if(parsingFeature) {
                    parsingFeature = false;
                    if(RC_IS_NOTOK(rc)) {
                        extFeatureInfoBuilder->setLib("");
                    }

                    ExtFeaturesRegistry::getInstance()->
                        registerExtFeature(extFeatureInfoBuilder->build());

                    delete extFeatureInfoBuilder;
                    extFeatureInfoBuilder = nullptr;
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
                if(!isKeyTypeList(topKey)) {
                    keyTracker.pop();
                }

                ADD_TO_EXT_FEATURE_BUILDER(EXT_FEATURE_CONFIGS_ELEM_ID, setId);
                ADD_TO_EXT_FEATURE_BUILDER(EXT_FEATURE_CONFIGS_ELEM_LIB, setLib);
                ADD_TO_EXT_FEATURE_BUILDER(EXT_FEATURE_CONFIGS_ELEM_NAME, setName);
                ADD_TO_EXT_FEATURE_BUILDER(EXT_FEATURE_CONFIGS_ELEM_SUBSCRIBER_LIST, addSignalSubscribedTo);

                break;

            default:
                break;
        }

        yaml_event_delete(&event);
    }

    TEARDOWN_LIBYAML_PARSING
    return rc;
}

ErrCode SignalConfigProcessor::parseSignalConfigs(const std::string& filePath, int8_t isBuSpecified) {
    return parseSignalConfigYamlNode(filePath, isBuSpecified);
}

ErrCode SignalConfigProcessor::parseExtFeaturesConfigs(const std::string& filePath) {
    return parseExtFeatureConfigYamlNode(filePath);
}

ErrCode SignalConfigProcessor::parse(ConfigType configType, const std::string& filePath, int8_t isBuSpecified) {
    ErrCode rc = RC_SUCCESS;

    switch(configType) {
        case ConfigType::SIGNALS_CONFIG: {
            rc = this->parseSignalConfigs(filePath, isBuSpecified);
            break;
        }
        case ConfigType::EXT_FEATURES_CONFIG: {
            rc = this->parseExtFeaturesConfigs(filePath);
            break;
        }
        default: {
            rc = RC_BAD_ARG;
            break;
        }
    }
    return rc;
}
