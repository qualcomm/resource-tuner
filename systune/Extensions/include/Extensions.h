// Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
// SPDX-License-Identifier: BSD-3-Clause-Clear

#ifndef EXTENSION_H
#define EXTENSION_H

#include <iostream>
#include <vector>
#include <unordered_map>

#include "Utils.h"
#include "Types.h"

class Extensions {
private:
    static std::vector<std::string> mModifiedConfigFiles;
    static std::unordered_map<int32_t, ResourceApplierCallback> mModifiedResourceConfigs;

public:
    Extensions(int32_t resourceOpcode, void (*resourceApplierCallback)(void*));
    Extensions(ConfigType configType, std::string jsonFile);

    static std::vector<std::pair<int32_t, ResourceApplierCallback>> getModifiedResources();

    static std::string getResourceConfigFilePath();

    static std::string getPropertiesConfigFilePath();

    static std::string getSignalsConfigFilePath();

    static std::string getExtFeaturesConfigFilePath();

    static std::string getTargetConfigFilePath();
};

#define CONCAT(a, b) a ## b

#define URM_REGISTER_RESOURCE(resourceOpcode, resourceApplierCallback) \
        static Extensions CONCAT(_resource, resourceOpcode)(resourceOpcode, resourceApplierCallback);

#define URM_REGISTER_CONFIG(configType, jsonFile) \
        static Extensions CONCAT(_regConfig, configType)(configType, jsonFile);

#define URM_REGISTER_SIGNALS_CALLBACK(signalsInitCallback, signalsListenerCallback) \
        static Extensions _signalsConfigInit(signalsInitCallback, signalsListenerCallback);

#endif
