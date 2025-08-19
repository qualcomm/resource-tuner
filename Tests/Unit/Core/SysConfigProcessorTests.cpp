// Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
// SPDX-License-Identifier: BSD-3-Clause-Clear

#include <thread>
#include <cstdint>
#include <gtest/gtest.h>

#include "SysConfigProcessor.h"
#include "SysConfigPropRegistry.h"
#include "Extensions.h"
#include "Utils.h"
#include "Logger.h"

RTN_REGISTER_CONFIG(PROPERTIES_CONFIG, "../Tests/Configs/testPropertiesConfig.yaml")

#define TOTAL_SYS_CONFIGS_PROPS_COUNT 14

class SysConfigProcessorTests: public::testing::Test {
protected:
    void SetUp() override {
        static int8_t firstTest = true;
        if(firstTest) {
            firstTest = false;
            SysConfigProcessor sysConfigProcessor;

            if(RC_IS_NOTOK(sysConfigProcessor.parseSysConfigs(Extensions::getPropertiesConfigFilePath()))) {
                return;
            }
        }
    }
};

TEST_F(SysConfigProcessorTests, TestSysConfigProcessorYAMLDataIntegrity1) {
    ASSERT_NE(SysConfigPropRegistry::getInstance(), nullptr);
}

TEST_F(SysConfigProcessorTests, TestSignalConfigProcessorYAMLDataIntegrity2) {
    ASSERT_EQ(SysConfigPropRegistry::getInstance()->getPropertiesCount(), TOTAL_SYS_CONFIGS_PROPS_COUNT);
}
