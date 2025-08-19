// Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
// SPDX-License-Identifier: BSD-3-Clause-Clear

/*!
 * \file  Extensions.h
 */

#ifndef RESOURCE_TUNER_EXTENSION_H
#define RESOURCE_TUNER_EXTENSION_H

#include <iostream>
#include <vector>
#include <unordered_map>

#include "Utils.h"

/**
* @brief Extensions
* @details Provides an Interface for Customizing Resource Tuner Behaviour. Through the Extension Interface,
*          Custom Resource Callbacks / Appliers as well as Custom Config Files (for example: Resource
*          Configs or Signal Configs) can be specified.
*/
class Extensions {
private:
    static std::vector<std::string> mModifiedConfigFiles;
    static std::unordered_map<uint32_t, ResourceLifecycleCallback> mResourceApplierCallbacks;
    static std::unordered_map<uint32_t, ResourceLifecycleCallback> mResourceTearCallbacks;

public:
    Extensions(uint32_t resourceOpcode, int8_t callbackType, ResourceLifecycleCallback callback);
    Extensions(ConfigType configType, std::string yamlFile);

    static std::vector<std::pair<uint32_t, ResourceLifecycleCallback>> getResourceApplierCallbacks();
    static std::vector<std::pair<uint32_t, ResourceLifecycleCallback>> getResourceTearCallbacks();

    static std::string getResourceConfigFilePath();
    static std::string getPropertiesConfigFilePath();
    static std::string getSignalsConfigFilePath();
    static std::string getExtFeaturesConfigFilePath();
    static std::string getTargetConfigFilePath();
};

#define CONCAT(a, b) a ## b

/**
 * \def RESTUNE_REGISTER_APPLIER_CB(resourceOpcode, resourceApplierCallback)
 * \brief Register a Customer Resource Applier for a particular Opcode
 * \param optionalInfo An unsigned 32-bit integer representing the Resource Opcode.
 * \param resourceApplierCallback A function Pointer to the Custom Applier.
 *
 * \note This macro must be used in the Global Scope.
 */
#define RESTUNE_REGISTER_APPLIER_CB(resourceOpcode, resourceApplierCallback) \
        static Extensions CONCAT(_resourceApplier, resourceOpcode)(resourceOpcode, 0, resourceApplierCallback);

#define RESTUNE_REGISTER_TEAR_CB(resourceOpcode, resourceApplierCallback) \
        static Extensions CONCAT(_resourceTear, resourceOpcode)(resourceOpcode, 1, resourceApplierCallback);

/**
 * \def RESTUNE_REGISTER_CONFIG(configType, yamlFile)
 * \brief Register custom Config (YAML) file. This Macro can be used to register
 *        Resource Configs File, Signal Configs file and others with Resource Tuner.
 * \param configType The type of Config for which the Custom YAML file has to be specified.
 * \param yamlFile File Path of this Config YAML file.
 *
 * \note This macro must be used in the Global Scope.
 */
#define RESTUNE_REGISTER_CONFIG(configType, yamlFile) \
        static Extensions CONCAT(_regConfig, configType)(configType, yamlFile);

#define RTN_REGISTER_SIGNALS_CALLBACK(signalsInitCallback, signalsListenerCallback) \
        static Extensions _signalsConfigInit(signalsInitCallback, signalsListenerCallback);

#endif
