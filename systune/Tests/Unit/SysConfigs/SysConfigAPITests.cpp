// Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
// SPDX-License-Identifier: BSD-3-Clause-Clear

#include <thread>
#include <cstdint>
#include <gtest/gtest.h>
#include <string.h>

#include "Extensions.h"
#include "SysConfigInternal.h"
#include "SysConfigPropRegistry.h"
#include "SysConfigProcessor.h"
#include "Logger.h"

URM_REGISTER_CONFIG(PROPERTIES_CONFIG, "../Tests/Configs/testPropertiesConfig.json")

class SysConfigAPITests: public::testing::Test {
protected:
    void SetUp() override {
        static int8_t firstTest = true;
        if(firstTest) {
            firstTest = false;

            std::shared_ptr<SysConfigProcessor> sysConfigProcessor =
                SysConfigProcessor::getInstance(Extensions::getPropertiesConfigFilePath());

            if(RC_IS_NOTOK(sysConfigProcessor->parseSysConfigs())) {
                LOGE("URM_TEST_SYSCONFIG_PARSER", "SysConfig Config Parsing Failed");
                return;
            }
        }
    }
};

TEST_F(SysConfigAPITests, TestSysConfigPropertiesParsing) {
    ASSERT_EQ(SysConfigPropRegistry::getInstance()->getPropertiesCount(), 14);
}

TEST_F(SysConfigAPITests, TestSysConfigGetPropSimpleRetrieval1) {
    std::string resultBuffer;

    int8_t propFound = sysConfigGetProp("test.debug.enabled", resultBuffer, sizeof(resultBuffer), "false");

    ASSERT_EQ(propFound, true);
    ASSERT_EQ(strcmp(resultBuffer.c_str(), "true"), 0);
}

TEST_F(SysConfigAPITests, TestSysConfigGetPropSimpleRetrieval2) {
    std::string resultBuffer;

    int8_t propFound = sysConfigGetProp("test.current.worker_thread.count", resultBuffer, sizeof(resultBuffer), "false");

    ASSERT_EQ(propFound, true);
    ASSERT_EQ(strcmp(resultBuffer.c_str(), "125"), 0);
}

TEST_F(SysConfigAPITests, TestSysConfigGetPropSimpleRetrievalInvalidProperty) {
    std::string resultBuffer;

    int8_t propFound = sysConfigGetProp("test.historic.worker_thread.count", resultBuffer, sizeof(resultBuffer), "5");

    ASSERT_EQ(propFound, false);
    ASSERT_EQ(strcmp(resultBuffer.c_str(), "5"), 0);
}

TEST_F(SysConfigAPITests, TestSysConfigGetPropConcurrentRetrieval) {
    std::thread th1([&]{
        std::string resultBuffer;
        int8_t propFound = sysConfigGetProp("test.current.worker_thread.count", resultBuffer, sizeof(resultBuffer), "false");

        ASSERT_EQ(propFound, true);
        ASSERT_EQ(strcmp(resultBuffer.c_str(), "125"), 0);
    });

    std::thread th2([&]{
        std::string resultBuffer;
        int8_t propFound = sysConfigGetProp("test.debug.enabled", resultBuffer, sizeof(resultBuffer), "false");

        ASSERT_EQ(propFound, true);
        ASSERT_EQ(strcmp(resultBuffer.c_str(), "true"), 0);
    });

    std::thread th3([&]{
        std::string resultBuffer;
        int8_t propFound = sysConfigGetProp("test.doc.build.mode.enabled", resultBuffer, sizeof(resultBuffer), "false");

        ASSERT_EQ(propFound, true);
        ASSERT_EQ(strcmp(resultBuffer.c_str(), "false"), 0);
    });

    th1.join();
    th2.join();
    th3.join();
}

TEST_F(SysConfigAPITests, TestSysConfigSetPropSimpleModification) {
    std::string resultBuffer;

    int8_t propFound = sysConfigGetProp("test.current.worker_thread.count", resultBuffer, sizeof(resultBuffer), "0");

    ASSERT_EQ(propFound, true);
    ASSERT_EQ(strcmp(resultBuffer.c_str(), "125"), 0);

    int8_t modificationStatus = sysConfigSetProp("test.current.worker_thread.count", "144");

    ASSERT_EQ(modificationStatus, true);

    propFound = sysConfigGetProp("test.current.worker_thread.count", resultBuffer, sizeof(resultBuffer), "0");

    ASSERT_EQ(propFound, true);
    ASSERT_EQ(strcmp(resultBuffer.c_str(), "144"), 0);
}
