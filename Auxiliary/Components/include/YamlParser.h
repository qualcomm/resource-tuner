// Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
// SPDX-License-Identifier: BSD-3-Clause-Clear

#ifndef YAML_PARSER_H
#define YAML_PARSER_H

#include <yaml-cpp/yaml.h>

#include "ErrCodes.h"
#include "Logger.h"

/**
 * @brief YamlParser
 * @details Utility for Reading and Parsing Yaml files (Note, Systune Configs are based in Yaml).
 *          Note, it internally uses the external yaml-cpp lib.
 */

class YamlParser {
public:
    static ErrCode parse(const std::string& fileName, YAML::Node& result);
};

template <typename T>
inline T safeExtract(const YAML::Node& node) {
    try {
        if(node.IsDefined() && node.IsScalar()) {
            return node.as<T>();
        }
    } catch(const YAML::TypedBadConversion<T>& e) {
        throw std::invalid_argument("Could not parse Yaml Node, Error: " + std::string(e.what()));
    }

    throw std::invalid_argument("Could not parse Yaml Node as it is Null or Not a Scalar");
}

template <typename T>
inline T safeExtract(const YAML::Node& node, T defaultValue) {
    try {
        if(node.IsDefined() && node.IsScalar()) {
            return node.as<T>();
        }
    } catch(const YAML::TypedBadConversion<T>& e) {
        LOGE("URM_YAML_PARSER",
             "Failed to parse Node to Yaml, Error: " + std::string(e.what()) + " " +
             "returning specified default Value");
        return defaultValue;
    }

    LOGE("URM_YAML_PARSER",
         "Could not parse Yaml Node as it is Null or Not a Scalar " \
         "returning specified default Value");
    return defaultValue;
}

inline int8_t isList(const YAML::Node& node) {
    return node.IsDefined() && node.IsSequence();
}

#endif
