// Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
// SPDX-License-Identifier: BSD-3-Clause-Clear

#include <cstdint>
#include <string.h>
#include <thread>

#include "Utils.h"
#include "TestUtils.h"
#include "ServerInternal.h"
#include "PropertiesRegistry.h"
#include "ConfigProcessor.h"

#define TOTAL_SYS_CONFIGS_PROPS_COUNT 14

static void Init() {
    ConfigProcessor configProcessor;
    if(RC_IS_NOTOK(configProcessor.parsePropertiesConfigs("/etc/resource-tuner/custom/PropertiesConfig.yaml"))) {
        return;
    }
}

static void TestSysConfigProcessorYAMLDataIntegrity1() {
    C_ASSERT(PropertiesRegistry::getInstance() != nullptr);
}

static void TestSysConfigPropertiesParsing() {
    std::cout<<"Total Properties Parsed: "<<PropertiesRegistry::getInstance()->getPropertiesCount()<<std::endl;
    C_ASSERT(PropertiesRegistry::getInstance()->getPropertiesCount() == TOTAL_SYS_CONFIGS_PROPS_COUNT);
}

static void TestSysConfigGetPropSimpleRetrieval1() {
    std::string resultBuffer;

    int8_t propFound = submitPropGetRequest("test.debug.enabled", resultBuffer, "false");

    C_ASSERT(propFound == true);
    C_ASSERT(strcmp(resultBuffer.c_str(), "true") == 0);
}

static void TestSysConfigGetPropSimpleRetrieval2() {
    std::string resultBuffer;

    int8_t propFound = submitPropGetRequest("test.current.worker_thread.count", resultBuffer, "false");

    C_ASSERT(propFound == true);
    C_ASSERT(strcmp(resultBuffer.c_str(), "125") == 0);
}

static void TestSysConfigGetPropSimpleRetrievalInvalidProperty() {
    std::string resultBuffer;

    int8_t propFound = submitPropGetRequest("test.historic.worker_thread.count", resultBuffer, "5");

    C_ASSERT(propFound == false);
    C_ASSERT(strcmp(resultBuffer.c_str(), "5") == 0);
}

static void TestSysConfigGetPropConcurrentRetrieval() {
    std::thread th1([&]{
        std::string resultBuffer;
        int8_t propFound = submitPropGetRequest("test.current.worker_thread.count", resultBuffer, "false");

        C_ASSERT(propFound == true);
        C_ASSERT(strcmp(resultBuffer.c_str(), "125") == 0);
    });

    std::thread th2([&]{
        std::string resultBuffer;
        int8_t propFound = submitPropGetRequest("test.debug.enabled", resultBuffer, "false");

        C_ASSERT(propFound == true);
        C_ASSERT(strcmp(resultBuffer.c_str(), "true") == 0);
    });

    std::thread th3([&]{
        std::string resultBuffer;
        int8_t propFound = submitPropGetRequest("test.doc.build.mode.enabled", resultBuffer, "false");

        C_ASSERT(propFound == true);
        C_ASSERT(strcmp(resultBuffer.c_str(), "false") == 0);
    });

    th1.join();
    th2.join();
    th3.join();
}

int32_t main() {
    std::cout<<"Running Test Suite: [SysConfigAPITests]\n"<<std::endl;

    Init();
    RUN_TEST(TestSysConfigPropertiesParsing);
    RUN_TEST(TestSysConfigProcessorYAMLDataIntegrity1);
    RUN_TEST(TestSysConfigGetPropSimpleRetrieval1);
    RUN_TEST(TestSysConfigGetPropSimpleRetrieval2);
    RUN_TEST(TestSysConfigGetPropSimpleRetrievalInvalidProperty);
    RUN_TEST(TestSysConfigGetPropConcurrentRetrieval);

    std::cout<<"\nAll Tests from the suite [SysConfigAPITests], executed successfully"<<std::endl;
}
