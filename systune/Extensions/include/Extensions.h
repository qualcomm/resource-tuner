// Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
// SPDX-License-Identifier: BSD-3-Clause-Clear

/*!
 * \file  Extensions.h
 */

#ifndef SYSTUNE_EXTENSION_H
#define SYSTUNE_EXTENSION_H

#include <iostream>
#include <vector>
#include <unordered_map>

#include "Utils.h"
#include "Types.h"

/**
* @brief Extensions
* @details Provides an Interface for Customizing Systune Behaviour. Through the Extension Interface,
*          Custom Resource Callbacks / Appliers as well as Custom Config Files (for example: Resource
*          Configs or Signal Configs) can be specified.
*/
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

/**
 * \def URM_REGISTER_RESOURCE(resourceOpcode, resourceApplierCallback)
 * \brief Register a Customer Resource Applier for a particular Opcode
 * \param optionalInfo An unsigned 32-bit integer representing the Resource Opcode.
 * \param resourceApplierCallback A function Pointer to the Custom Applier.
 *
 * \note This macro must be used in the Global Scope.
 */
#define URM_REGISTER_RESOURCE(resourceOpcode, resourceApplierCallback) \
        static Extensions CONCAT(_resource, resourceOpcode)(resourceOpcode, resourceApplierCallback);

/**
 * \def URM_REGISTER_CONFIG(configType, jsonFile)
 * \brief Register custom Config (JSON) file. This Macro can be used to register
 *        Resource Configs File, Signal Configs file and others with Systune.
 * \param configType The type of Config for which the Custom JSON file has to be specified.
 * \param jsonFile File Path of this Config JSON file.
 *
 * \note This macro must be used in the Global Scope.
 */
#define URM_REGISTER_CONFIG(configType, jsonFile) \
        static Extensions CONCAT(_regConfig, configType)(configType, jsonFile);

#define URM_REGISTER_SIGNALS_CALLBACK(signalsInitCallback, signalsListenerCallback) \
        static Extensions _signalsConfigInit(signalsInitCallback, signalsListenerCallback);

#endif
