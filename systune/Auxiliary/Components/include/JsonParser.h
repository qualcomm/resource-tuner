// Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
// SPDX-License-Identifier: BSD-3-Clause-Clear

#ifndef JSON_PARSER_H
#define JSON_PARSER_H

#include <fstream>
#include <string>
#include <stdint.h>
#include <jsoncpp/json/json.h>
#include <functional>

#include "ErrCodes.h"

/**
 * @brief JsonParser
 * @details Utility for Reading and Parsing JSON files (Note, Systune Configs are based in JSON).
 *          Note, it internally uses the external json_cpp lib.
 */
class JsonParser {
private:
    Json::Value mRoot;
    std::string mJsonRootName;

public:
    JsonParser();
    ~JsonParser();

    /**
    * @brief Verifies the format of the json file with the expected format
    *       and scans file for total members
    *
    * @param fJsonFileName Json file path.
    * @param fJsonRootName The name of the root member of the file.
    * @return int32_t returns the number of members of the root array.
    *               -1 is returned for invalid json.
    */
    ErrCode verifyAndGetSize(const std::string& fJsonFileName, const std::string& fJsonRootName, int32_t& size);

    /**
    * @brief For each member of the root array, it calls the callback function for processing
    *       and storing data in appropriate data structures.
    * @param callback The callback function that does the processing of the json member.
    */
    int32_t parse(std::function<void(const Json::Value&)> callback);
};

#endif
