// Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
// SPDX-License-Identifier: BSD-3-Clause-Clear

#include <thread>
#include <cstdint>
#include <gtest/gtest.h>

#include "TargetConfigProcessor.h"
#include "ResourceRegistry.h"
#include "TargetRegistry.h"
#include "Extensions.h"
#include "Utils.h"

URM_REGISTER_CONFIG(TARGET_CONFIG, "../Tests/Configs/testTargetConfigs.json")

class TargetConfigProcessorTests: public::testing::Test {
protected:
    void SetUp() override {
        static int8_t firstTest = true;
        if(firstTest) {
            firstTest = false;
            TargetConfigProcessor targetConfigProcessor(Extensions::getTargetConfigFilePath());

            if(RC_IS_NOTOK(targetConfigProcessor.parseTargetConfigs())) {
                return;
            }

            if(RC_IS_NOTOK(TargetRegistry::getInstance()->readPhysicalCoreClusterInfo())) {
                return;
            }
        }
    }
};

TEST_F(TargetConfigProcessorTests, TestTargetConfigProcessorJSONDataIntegrity1) {
    ASSERT_NE(TargetRegistry::getInstance(), nullptr);
}

TEST_F(TargetConfigProcessorTests, TestResourceConfigProcessorJSONDataIntegrity2) {
    ASSERT_EQ(SystuneSettings::targetConfigs.totalCoreCount, 16);
}

TEST_F(TargetConfigProcessorTests, TestResourceConfigProcessorJSONDataIntegrity3) {
    ASSERT_EQ(strcmp(SystuneSettings::targetConfigs.targetName.c_str(), "qli-test"), 0);
}

TEST_F(TargetConfigProcessorTests, TestResourceConfigProcessorJSONDataIntegrity4) {
    ASSERT_EQ(TargetRegistry::getInstance()->getPhysicalClusterId(BIG), 1);
    ASSERT_EQ(TargetRegistry::getInstance()->getPhysicalClusterId(LITTLE), 3);
    ASSERT_EQ(TargetRegistry::getInstance()->getPhysicalClusterId(PRIME), 0);
    ASSERT_EQ(TargetRegistry::getInstance()->getPhysicalClusterId(TITANIUM), 2);
}

TEST_F(TargetConfigProcessorTests, TestResourceConfigProcessorJSONDataIntegrity5) {
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
