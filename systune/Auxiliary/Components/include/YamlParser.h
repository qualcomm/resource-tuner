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
inline T safeExtract(const YAML::Node& item) {
    try {
        if(item.IsDefined() && item.IsScalar()) {
            return item.as<T>();
        }
    } catch (YAML::TypedBadConversion<T>& e) {}

    throw std::invalid_argument("Could not parse Yaml Node");
}

#endif
