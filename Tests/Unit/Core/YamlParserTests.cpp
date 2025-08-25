// Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
// SPDX-License-Identifier: BSD-3-Clause-Clear

#include <iostream>

#include "YamlParser.h"

#define RUN_TEST(test)                                              \
do {                                                                \
    std::cout<<"Running Test: "<<#test<<std::endl;                  \
    test();                                                         \
    std::cout<<#test<<": Run Successful"<<std::endl;                \
    std::cout<<"-------------------------------------"<<std::endl;  \
} while(false);                                                     \

#define C_ASSERT(cond)                                                              \
    if(cond == false) {                                                             \
        std::cerr<<"Condition Check on line:["<<__LINE__<<"]  failed"<<std::endl; \
        std::cerr<<"Test: ["<<__func__<<"] Failed, Terminating Suite\n"<<std::endl;  \
        exit(EXIT_FAILURE);                                                         \
    }                                                                               \

static void TestYamlParserFileNotFound() {
    // A Non-Existent file
    const std::string fileName = "AuxParserTest.yaml";
    YAML::Node result;
    ErrCode rc = YamlParser::parse(fileName, result);
    C_ASSERT(rc == RC_FILE_NOT_FOUND);
}

static void TestYamlParserInvalidSyntax() {
    // A YAML with Malformed or Invalid Syntax
    const std::string fileName = "/etc/resource-tuner/tests/Configs/InvalidSyntax.yaml";
    YAML::Node result;
    ErrCode rc = YamlParser::parse(fileName, result);
    C_ASSERT(rc == RC_YAML_INVALID_SYNTAX);
}

int32_t main() {
    std::cout<<"Running YamlParser Test Suite\n"<<std::endl;

    RUN_TEST(TestYamlParserFileNotFound);
    RUN_TEST(TestYamlParserInvalidSyntax);

    std::cout<<"\nAll Tests from the suite: [YamlParser], executed successfully"<<std::endl;
}
