// Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
// SPDX-License-Identifier: BSD-3-Clause-Clear

#include <thread>
#include <cstdint>
#include <gtest/gtest.h>

#include "ConfigProcessor.h"
#include "ResourceRegistry.h"
#include "TargetRegistry.h"
#include "Extensions.h"
#include "Utils.h"

RESTUNE_REGISTER_CONFIG(TARGET_CONFIG, "../Tests/Configs/testTargetConfig.yaml")

class TargetConfigProcessorTests: public::testing::Test {
protected:
    void SetUp() override {
        static int8_t firstTest = true;
        if(firstTest) {
            firstTest = false;
            ConfigProcessor configProcessor;

            if(RC_IS_NOTOK(configProcessor.parseTargetConfigs(Extensions::getTargetConfigFilePath()))) {
                return;
            }

            if(RC_IS_NOTOK(TargetRegistry::getInstance()->readPhysicalCoreClusterInfo())) {
                return;
            }
        }
    }
};

TEST_F(TargetConfigProcessorTests, TestTargetConfigProcessorYAMLDataIntegrity1) {
    ASSERT_NE(TargetRegistry::getInstance(), nullptr);
}

TEST_F(TargetConfigProcessorTests, TestResourceConfigProcessorYAMLDataIntegrity2) {
    ASSERT_EQ(ResourceTunerSettings::targetConfigs.totalCoreCount, 16);
}

TEST_F(TargetConfigProcessorTests, TestResourceConfigProcessorYAMLDataIntegrity3) {
    ASSERT_EQ(strcmp(ResourceTunerSettings::targetConfigs.targetName.c_str(), "qli-test"), 0);
}

TEST_F(TargetConfigProcessorTests, TestResourceConfigProcessorYAMLDataIntegrity4) {
    ASSERT_EQ(TargetRegistry::getInstance()->getPhysicalClusterId(BIG), 1);
    ASSERT_EQ(TargetRegistry::getInstance()->getPhysicalClusterId(LITTLE), 3);
    ASSERT_EQ(TargetRegistry::getInstance()->getPhysicalClusterId(PRIME), 0);
    ASSERT_EQ(TargetRegistry::getInstance()->getPhysicalClusterId(TITANIUM), 2);
}

TEST_F(TargetConfigProcessorTests, TestResourceConfigProcessorYAMLDataIntegrity5) {
    // Distribution
    // 0: PRIME => 0, 1, 2, 3
    // 1: BIG => 4, 5, 6, 7
    // 2: TITANIUM => 8, 9, 10, 11
    // 3: LITTLE => 12, 13, 14, 15

    // Get the first Core in the Big Cluster
    ASSERT_EQ(TargetRegistry::getInstance()->getPhysicalCoreId(BIG, 1), 4);

    // Get the third Core in the Titanium Cluster
    ASSERT_EQ(TargetRegistry::getInstance()->getPhysicalCoreId(TITANIUM, 3), 10);

    // Get the second Core in the Little Cluster
    ASSERT_EQ(TargetRegistry::getInstance()->getPhysicalCoreId(LITTLE, 2), 13);

    // Get the fourth Core in the Prime Cluster
    ASSERT_EQ(TargetRegistry::getInstance()->getPhysicalCoreId(PRIME, 4), 3);
}
