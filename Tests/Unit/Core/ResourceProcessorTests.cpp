// Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
// SPDX-License-Identifier: BSD-3-Clause-Clear

#include <thread>
#include <cstdint>
#include <gtest/gtest.h>

#include "ConfigProcessor.h"
#include "ResourceRegistry.h"
#include "Extensions.h"
#include "Utils.h"

RTN_REGISTER_CONFIG(RESOURCE_CONFIG, "../Tests/Configs/testResourcesConfig.yaml")

#define TOTAL_RESOURCE_CONFIGS_COUNT 11

#define GENERATE_RESOURCE_ID(optype, opcode) ({                                \
    uint32_t resourceBitmap = 0;                                               \
    resourceBitmap |= (1 << 31);                                               \
    resourceBitmap |= ((uint32_t)opcode);                                      \
    resourceBitmap |= ((uint32_t)optype << 16);                                \
    resourceBitmap;                                                            \
})

class ResourceProcessorTests: public::testing::Test {
protected:
    void SetUp() override {
        static int8_t firstTest = true;
        if(firstTest) {
            firstTest = false;
            ConfigProcessor configProcessor;

            if(RC_IS_NOTOK(configProcessor.parseResourceConfigs(Extensions::getResourceConfigFilePath(), true))) {
                return;
            }
        }
    }
};

TEST_F(ResourceProcessorTests, TestResourceConfigProcessorYAMLDataIntegrity1) {
    ASSERT_NE(ResourceRegistry::getInstance(), nullptr);
}

TEST_F(ResourceProcessorTests, TestResourceConfigProcessorYAMLDataIntegrity2) {
    ASSERT_EQ(ResourceRegistry::getInstance()->getTotalResourcesCount(), TOTAL_RESOURCE_CONFIGS_COUNT);
}

TEST_F(ResourceProcessorTests, TestResourceConfigProcessorYAMLDataIntegrity3_1) {
    ResourceConfigInfo* resourceConfigInfo = ResourceRegistry::getInstance()->getResourceById(GENERATE_RESOURCE_ID(1, 0));

    ASSERT_NE(resourceConfigInfo, nullptr);
    ASSERT_EQ(resourceConfigInfo->mResourceOptype, 1);
    ASSERT_EQ(resourceConfigInfo->mResourceOpcode, 0);
    ASSERT_EQ(strcmp((const char*)resourceConfigInfo->mResourceName.data(), "TEST_RESOURCE_1"), 0);
    ASSERT_EQ(strcmp((const char*)resourceConfigInfo->mResourcePath.data(), "../Tests/Configs/ResourceSysFsNodes/sched_util_clamp_min"), 0);
    ASSERT_EQ(resourceConfigInfo->mHighThreshold, 1024);
    ASSERT_EQ(resourceConfigInfo->mLowThreshold, 0);
    ASSERT_EQ(resourceConfigInfo->mSupported, true);
    ASSERT_EQ(resourceConfigInfo->mPolicy, HIGHER_BETTER);
    ASSERT_EQ(resourceConfigInfo->mPermissions, PERMISSION_THIRD_PARTY);
    ASSERT_EQ(resourceConfigInfo->mModes, MODE_DISPLAY_ON | MODE_DOZE);
    ASSERT_EQ(resourceConfigInfo->mApplyType, ResourceApplyType::APPLY_GLOBAL);
}

TEST_F(ResourceProcessorTests, TestResourceConfigProcessorYAMLDataIntegrity3_2) {
    ResourceConfigInfo* resourceConfigInfo = ResourceRegistry::getInstance()->getResourceById(GENERATE_RESOURCE_ID(1, 1));

    ASSERT_NE(resourceConfigInfo, nullptr);
    ASSERT_EQ(resourceConfigInfo->mResourceOptype, 1);
    ASSERT_EQ(resourceConfigInfo->mResourceOpcode, 1);
    ASSERT_EQ(strcmp((const char*)resourceConfigInfo->mResourceName.data(), "TEST_RESOURCE_2"), 0);
    ASSERT_EQ(strcmp((const char*)resourceConfigInfo->mResourcePath.data(), "../Tests/Configs/ResourceSysFsNodes/sched_util_clamp_max"), 0);
    ASSERT_EQ(resourceConfigInfo->mHighThreshold, 1024);
    ASSERT_EQ(resourceConfigInfo->mLowThreshold, 512);
    ASSERT_EQ(resourceConfigInfo->mSupported, true);
    ASSERT_EQ(resourceConfigInfo->mPolicy, HIGHER_BETTER);
    ASSERT_EQ(resourceConfigInfo->mPermissions, PERMISSION_THIRD_PARTY);
    ASSERT_EQ(resourceConfigInfo->mModes, MODE_DISPLAY_ON | MODE_DOZE);
    ASSERT_EQ(resourceConfigInfo->mApplyType, ResourceApplyType::APPLY_GLOBAL);
}

TEST_F(ResourceProcessorTests, TestResourceConfigProcessorYAMLDataIntegrity3_3) {
    ResourceConfigInfo* resourceConfigInfo = ResourceRegistry::getInstance()->getResourceById(GENERATE_RESOURCE_ID(1, 5));

    ASSERT_NE(resourceConfigInfo, nullptr);
    ASSERT_EQ(resourceConfigInfo->mResourceOptype, 1);
    ASSERT_EQ(resourceConfigInfo->mResourceOpcode, 5);
    ASSERT_EQ(strcmp((const char*)resourceConfigInfo->mResourceName.data(), "TEST_RESOURCE_6"), 0);
    ASSERT_EQ(strcmp((const char*)resourceConfigInfo->mResourcePath.data(), "../Tests/Configs/ResourceSysFsNodes/target_test_resource2"), 0);
    ASSERT_EQ(resourceConfigInfo->mHighThreshold, 6500);
    ASSERT_EQ(resourceConfigInfo->mLowThreshold, 50);
    ASSERT_EQ(resourceConfigInfo->mSupported, true);
    ASSERT_EQ(resourceConfigInfo->mPolicy, HIGHER_BETTER);
    ASSERT_EQ(resourceConfigInfo->mPermissions, PERMISSION_THIRD_PARTY);
    ASSERT_EQ(resourceConfigInfo->mModes, MODE_DISPLAY_ON);
    ASSERT_EQ(resourceConfigInfo->mApplyType, ResourceApplyType::APPLY_GLOBAL);
}
