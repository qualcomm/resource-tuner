// Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
// SPDX-License-Identifier: BSD-3-Clause-Clear

#include <iostream>

#include "TestUtils.h"
#include "YamlParser.h"

static void TestYamlParserFileNotFound() {
    // A Non-Existent file
    const std::string fileName = "AuxParserTest.yaml";
    YAML::Node result;
    ErrCode rc = YamlParser::parse(fileName, result);
    C_ASSERT(rc == RC_FILE_NOT_FOUND);
}

static void TestYamlParserInvalidSyntax() {
    // A YAML with Malformed or Invalid Syntax
    const std::string fileName = "/etc/resource-tuner/custom/InvalidSyntax.yaml";
    YAML::Node result;
    ErrCode rc = YamlParser::parse(fileName, result);
    C_ASSERT(rc == RC_YAML_INVALID_SYNTAX);
}

int32_t main() {
    std::cout<<"Running [YamlParser] Test Suite\n"<<std::endl;

    RUN_TEST(TestYamlParserFileNotFound);
    RUN_TEST(TestYamlParserInvalidSyntax);

    std::cout<<"\nAll Tests from the suite: [YamlParser], executed successfully"<<std::endl;
}
