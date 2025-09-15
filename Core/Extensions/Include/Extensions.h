// Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
// SPDX-License-Identifier: BSD-3-Clause-Clear

/*!
 * \file  Extensions.h
 */

#ifndef RESOURCE_TUNER_EXTENSIONS_H
#define RESOURCE_TUNER_EXTENSIONS_H

#include <iostream>
#include <vector>
#include <unordered_map>

typedef void (*ResourceLifecycleCallback)(void*);

/**
 * @enum ConfigType
 * @brief Different Config (via YAML) Types supported.
 * @details Note, the Config File corresponding to each config type
 *          can be altered via the Extensions interface.
 */
enum ConfigType {
    RESOURCE_CONFIG,
    PROPERTIES_CONFIG,
    SIGNALS_CONFIG,
    EXT_FEATURES_CONFIG,
    TARGET_CONFIG,
    INIT_CONFIG,
    TOTAL_CONFIGS_COUNT
};

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
    Extensions(uint32_t resCode, int8_t callbackType, ResourceLifecycleCallback callback);
    Extensions(ConfigType configType, std::string yamlFile);

    static std::vector<std::pair<uint32_t, ResourceLifecycleCallback>> getResourceApplierCallbacks();
    static std::vector<std::pair<uint32_t, ResourceLifecycleCallback>> getResourceTearCallbacks();

    static std::string getResourceConfigFilePath();
    static std::string getPropertiesConfigFilePath();
    static std::string getSignalsConfigFilePath();
    static std::string getExtFeaturesConfigFilePath();
    static std::string getTargetConfigFilePath();
    static std::string getInitConfigFilePath();
};

#define CONCAT(a, b) a ## b

/**
 * \def RESTUNE_REGISTER_APPLIER_CB(resCode, resourceApplierCallback)
 * \brief Register a Customer Resource Applier Callback for a particular ResCode
 * \param resCode An unsigned 32-bit integer representing the Resource ResCode.
 * \param resourceApplierCallback A function Pointer to the Custom Applier Callback.
 *
 * \note This macro must be used in the Global Scope.
 */
#define RESTUNE_REGISTER_APPLIER_CB(resCode, resourceApplierCallback) \
        static Extensions CONCAT(_resourceApplier, resCode)(resCode, 0, resourceApplierCallback);

/**
 * \def RESTUNE_REGISTER_TEAR_CB(resCode, resourceTearCallback)
 * \brief Register a Customer Resource Teardown Callback for a particular ResCode
 * \param resCode An unsigned 32-bit integer representing the Resource ResCode.
 * \param resourceTearCallback A function Pointer to the Custom Teardown Callback.
 *
 * \note This macro must be used in the Global Scope.
 */
#define RESTUNE_REGISTER_TEAR_CB(resCode, resourceTearCallback) \
        static Extensions CONCAT(_resourceTear, resCode)(resCode, 1, resourceTearCallback);

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

#endif
