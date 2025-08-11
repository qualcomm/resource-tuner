// Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
// SPDX-License-Identifier: BSD-3-Clause-Clear

#include <gtest/gtest.h>
#include "YamlParser.h"

TEST(YamlParserTests, TestYamlParserFileNotFound) {
    // A Non-Existent file
    const std::string fileName = "AuxParserTest.yaml";
    YAML::Node result;
    ErrCode rc = YamlParser::parse(fileName, result);
    ASSERT_EQ(rc, RC_FILE_NOT_FOUND);
}

TEST(YamlParserTests, TestYamlParserInvalidSyntax) {
    // A YAML with Malformed or Invalid Syntax
    const std::string fileName = "testYamlInvalidSyntax.yaml";
    YAML::Node result;
    ErrCode rc = YamlParser::parse(fileName, result);
    ASSERT_EQ(rc, RC_YAML_INVALID_SYNTAX);
}
