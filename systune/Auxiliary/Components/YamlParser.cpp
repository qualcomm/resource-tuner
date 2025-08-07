// Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
// SPDX-License-Identifier: BSD-3-Clause-Clear

#include "YamlParser.h"
#include <iostream>

ErrCode YamlParser::parse(const std::string& fileName, YAML::Node& result) {
    try {
        result = YAML::LoadFile(fileName);
        return RC_SUCCESS;

    } catch(const YAML::BadFile& e) {
        TYPELOGV(YAML_PARSE_ERROR, fileName, e.what());
        return RC_FILE_NOT_FOUND;

    } catch(const YAML::ParserException& e) {
        TYPELOGV(YAML_PARSE_ERROR, fileName, e.what());
        return RC_YAML_INVALID_SYNTAX;

    } catch(const YAML::Exception& e) {
        TYPELOGV(YAML_PARSE_ERROR, fileName, e.what());
        return RC_YAML_PARSING_ERROR;
    }

    return RC_YAML_PARSING_ERROR;
}
