// Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
// SPDX-License-Identifier: BSD-3-Clause-Clear

#include "SignalConfigProcessor.h"

#define ADD_TO_EXT_FEATURE_BUILDER(KEY, METHOD)                 \
    if(topKey == KEY && extFeatureInfoBuilder != nullptr) {     \
        if(RC_IS_OK(rc)) {                                      \
            rc = extFeatureInfoBuilder->METHOD(value);          \
        }                                                       \
        break;                                                  \
    }

static int8_t isKey(const std::string& keyName) {
    if(keyName == EXT_FEATURES_CONFIGS_ROOT) return true;
    if(keyName == EXT_FEATURE_ID) return true;
    if(keyName == EXT_FEATURE_LIB) return true;
    if(keyName == EXT_FEATURE_NAME) return true;
    if(keyName == EXT_FEATURE_DESCRIPTION) return true;
    if(keyName == EXT_FEATURE_SUBSCRIBER_LIST) return true;

    return false;
}

ErrCode SignalConfigProcessor::parseSignalConfigYamlNode(const std::string& filePath, int8_t isBuSpecified) {
    SETUP_LIBYAML_PARSING(filePath);

    int8_t parsingDone = false;
    int8_t parsingSignal = false;
    int8_t inSignalConfigs = false;
    int8_t inTargetEnabledList = false;
    int8_t inTargetDisabledList = false;
    int8_t inPermissionsList = false;
    int8_t inDerivativesList = false;
    int8_t inResourcesList = false;
    int8_t inResourceValuesList = false;
    int8_t inResourceItemMap = false;
    int8_t inResourcesMap = false;

    std::vector<std::string> resValues;

    std::string currentKey;
    std::string resourceKey;
    std::string value;

    ErrCode rc = RC_SUCCESS;

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
                if(!inSignalConfigs) {
                    inSignalConfigs = true;
                } else if(currentKey == SIGNAL_RESOURCES) {
                    inResourcesMap = true;
                    resourceBuilder = new(std::nothrow) ResourceBuilder;
                } else {
                    parsingSignal = true;
                    signalInfoBuilder = new(std::nothrow) SignalInfoBuilder;
                    if(signalInfoBuilder == nullptr) {
                        return RC_YAML_PARSING_ERROR;
                    }
                }

                break;

            case YAML_MAPPING_END_EVENT:
                if(inResourcesMap) {
                    if(RC_IS_OK(rc)) {
                        rc = signalInfoBuilder->addResource(resourceBuilder->build());
                    }
                    inResourcesMap = false;

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

            case YAML_SEQUENCE_START_EVENT:
                if(currentKey == SIGNAL_TARGETS_ENABLED) {
                    inTargetEnabledList = true;
                } else if(currentKey == SIGNAL_TARGETS_DISABLED) {
                    inTargetDisabledList = true;
                } else if(currentKey == SIGNAL_PERMISSIONS) {
                    inPermissionsList = true;
                } else if(currentKey == SIGNAL_DERIVATIVES) {
                    inDerivativesList = true;
                } else if(currentKey == SIGNAL_RESOURCES) {
                    if(!inResourcesList) {
                        inResourcesList = true;
                    } else if(inResourcesList) {
                        inResourceValuesList = true;
                    }
                } else if(resourceKey == SIGNAL_VALUES) {
                    inResourceValuesList = true;
                }

                break;

            case YAML_SEQUENCE_END_EVENT:
                if(inTargetEnabledList) {
                    inTargetEnabledList = false;
                } else if(inTargetDisabledList) {
                    inTargetDisabledList = false;
                } else if(inPermissionsList) {
                    inPermissionsList = false;
                } else if(inDerivativesList) {
                    inDerivativesList = false;
                } else if(inResourcesList) {
                    if(inResourceValuesList) {
                        inResourceValuesList = false;
                        if(RC_IS_OK(rc)) {
                            rc = resourceBuilder->setNumValues(resValues.size());
                        }

                        for(std::string resValue: resValues) {
                            if(RC_IS_OK(rc)) {
                                rc = resourceBuilder->addValue(resValue);
                            }

                            if(RC_IS_NOTOK(rc)) {
                                break;
                            }
                        }
                        resValues.clear();
                        break;

                    } else {
                        inResourcesList = false;
                    }
                } else if(inResourceValuesList) {
                    inResourceValuesList = false;
                }

                currentKey.clear();
                break;

            case YAML_SCALAR_EVENT:
                if(event.data.scalar.value != nullptr) {
                    value = reinterpret_cast<char*>(event.data.scalar.value);
                }

                if(value == SIGNAL_CONFIGS_ROOT) {
                    break;
                }

                if(currentKey.length() == 0) {
                    currentKey = value;
                } else {
                    if(inTargetEnabledList) {
                        if(RC_IS_OK(rc)) {
                            rc = signalInfoBuilder->addTarget(true, value);
                        }
                        break;
                    } else if(inTargetDisabledList) {
                        if(RC_IS_OK(rc)) {
                            rc = signalInfoBuilder->addTarget(false, value);
                        }
                        break;
                    } else if(inPermissionsList) {
                        if(RC_IS_OK(rc)) {
                            rc = signalInfoBuilder->addPermission(value);
                        }
                        break;
                    } else if(inDerivativesList) {
                        if(RC_IS_OK(rc)) {
                            rc = signalInfoBuilder->addDerivative(value);
                        }
                        break;
                    } else if(inResourcesList) {
                        if(value == SIGNAL_RESOURCE_CODE || value == SIGNAL_RESINFO || value == SIGNAL_VALUES) {
                            resourceKey = value;
                            break;
                        }

                        if(inResourceValuesList) {
                            resValues.push_back(value);
                        } else if(resourceKey == SIGNAL_RESOURCE_CODE) {
                            if(RC_IS_OK(rc)) {
                                rc = resourceBuilder->setResCode(value);
                            }
                        } else if(resourceKey == SIGNAL_RESINFO) {
                            if(RC_IS_OK(rc)) {
                                rc = resourceBuilder->setResInfo(value);
                            }
                        }

                        resourceKey.clear();
                        break;

                    } else if(currentKey == SIGNAL_SIGID) {
                        if(RC_IS_OK(rc)) {
                            rc = signalInfoBuilder->setSignalID(value);
                        }
                    } else if(currentKey == SIGNAL_CATEGORY) {
                        if(RC_IS_OK(rc)) {
                            rc = signalInfoBuilder->setSignalCategory(value);
                        }
                    } else if(currentKey == SIGNAL_NAME) {
                        if(RC_IS_OK(rc)) {
                            rc = signalInfoBuilder->setName(value);
                        }
                    } else if(currentKey == SIGNAL_TIMEOUT) {
                        if(RC_IS_OK(rc)) {
                            rc = signalInfoBuilder->setTimeout(value);
                        }
                    } else if(currentKey == SIGNAL_ENABLE) {
                        if(RC_IS_OK(rc)) {
                            rc = signalInfoBuilder->setIsEnabled(value);
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

    TEARDOWN_LIBYAML_PARSING
    return rc;
}

ErrCode SignalConfigProcessor::parseExtFeatureConfigYamlNode(const std::string& filePath) {
    SETUP_LIBYAML_PARSING(filePath);

    ErrCode rc = RC_SUCCESS;

    int8_t parsingDone = false;
    int8_t docMarker = false;
    int8_t parsingFeature = false;

    std::string currentKey;
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

                std::cout<<"scalar: "<<value<<std::endl;

                if(isKey(value)) {
                    keyTracker.push(value);
                    std::cout<<"key updated to: "<<value<<std::endl;
                    break;
                }

                if(keyTracker.empty()) {
                    return RC_YAML_INVALID_SYNTAX;
                }

                topKey = keyTracker.top();
                if(topKey != EXT_FEATURE_SUBSCRIBER_LIST) {
                    keyTracker.pop();
                }

                ADD_TO_EXT_FEATURE_BUILDER(EXT_FEATURE_ID, setId);
                ADD_TO_EXT_FEATURE_BUILDER(EXT_FEATURE_LIB, setLib);
                ADD_TO_EXT_FEATURE_BUILDER(EXT_FEATURE_NAME, setName);
                ADD_TO_EXT_FEATURE_BUILDER(EXT_FEATURE_SUBSCRIBER_LIST, addSignalSubscribedTo);

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
