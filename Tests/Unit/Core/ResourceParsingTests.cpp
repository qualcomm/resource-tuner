// Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
// SPDX-License-Identifier: BSD-3-Clause-Clear

#include <thread>
#include <cstdint>
#include <gtest/gtest.h>

#include "ConfigProcessor.h"
#include "ResourceRegistry.h"
#include "Extensions.h"
#include "Utils.h"

#define TOTAL_RESOURCE_CONFIGS_COUNT 38

static int8_t firstTest = true;

class ResourceParsingTests: public::testing::Test {
protected:
    void SetUp() override {
        if(firstTest) {
            firstTest = false;
            ConfigProcessor configProcessor;

            std::string commonResources = "/etc/resource-tuner/tests/Configs/ResourcesConfigA.yaml";
            std::string additionalResourcesCatA = "/etc/resource-tuner/tests/Configs/ResourcesConfig.yaml";
            std::string additionalResourcesCatB =  "/etc/resource-tuner/tests/Configs/ResourcesConfigB.yaml";

            if(RC_IS_NOTOK(configProcessor.parseResourceConfigs(commonResources))) {
                return;
            }

            if(RC_IS_NOTOK(configProcessor.parseResourceConfigs(additionalResourcesCatA))) {
                return;
            }

            if(RC_IS_NOTOK(configProcessor.parseResourceConfigs(additionalResourcesCatB, true))) {
                return;
            }
        }
    }
};

TEST_F(ResourceParsingTests, TestResourceParsingSanity) {
    ASSERT_NE(ResourceRegistry::getInstance(), nullptr);
}

TEST_F(ResourceParsingTests, TestResourceParsingResourcesParsed) {
    ASSERT_EQ(ResourceRegistry::getInstance()->getTotalResourcesCount(), TOTAL_RESOURCE_CONFIGS_COUNT);
}

TEST_F(ResourceParsingTests, TestResourceParsingResourcesMerged1) {
    ResourceConfigInfo* resourceConfigInfo = ResourceRegistry::getInstance()->getResourceById(0x00030000);

    ASSERT_NE(resourceConfigInfo, nullptr);
    ASSERT_EQ(resourceConfigInfo->mResourceResType, 3);
    ASSERT_EQ(resourceConfigInfo->mResourceResID, 0);
    ASSERT_EQ(strcmp((const char*)resourceConfigInfo->mResourceName.data(), "OVERWRITE_RESOURCE_1"), 0);
    ASSERT_EQ(strcmp((const char*)resourceConfigInfo->mResourcePath.data(), "/proc/sys/kernel/test_node1"), 0);
    ASSERT_EQ(resourceConfigInfo->mHighThreshold, 1024);
    ASSERT_EQ(resourceConfigInfo->mLowThreshold, 0);
    ASSERT_EQ(resourceConfigInfo->mSupported, true);
    ASSERT_EQ(resourceConfigInfo->mPolicy, LOWER_BETTER);
    ASSERT_EQ(resourceConfigInfo->mPermissions, PERMISSION_SYSTEM);
    ASSERT_EQ(resourceConfigInfo->mModes, MODE_DISPLAY_ON | MODE_DOZE);
    ASSERT_EQ(resourceConfigInfo->mApplyType, ResourceApplyType::APPLY_GLOBAL);
}

TEST_F(ResourceParsingTests, TestResourceParsingResourcesMerged2) {
    ResourceConfigInfo* resourceConfigInfo = ResourceRegistry::getInstance()->getResourceById(0x00040003);

    ASSERT_NE(resourceConfigInfo, nullptr);
    ASSERT_EQ(resourceConfigInfo->mResourceResType, 4);
    ASSERT_EQ(resourceConfigInfo->mResourceResID, 3);
    ASSERT_EQ(strcmp((const char*)resourceConfigInfo->mResourceName.data(), "OVERWRITE_RESOURCE_2"), 0);
    ASSERT_EQ(strcmp((const char*)resourceConfigInfo->mResourcePath.data(), "/proc/sys/kernel/test_node2"), 0);
    ASSERT_EQ(resourceConfigInfo->mHighThreshold, 1024);
    ASSERT_EQ(resourceConfigInfo->mLowThreshold, 890);
    ASSERT_EQ(resourceConfigInfo->mSupported, true);
    ASSERT_EQ(resourceConfigInfo->mPolicy, INSTANT_APPLY);
    ASSERT_EQ(resourceConfigInfo->mPermissions, PERMISSION_SYSTEM);
    ASSERT_EQ(resourceConfigInfo->mModes, MODE_DISPLAY_ON | MODE_DISPLAY_OFF);
    ASSERT_EQ(resourceConfigInfo->mApplyType, ResourceApplyType::APPLY_CLUSTER);
}

TEST_F(ResourceParsingTests, TestResourceParsingResourcesMerged3) {
    ResourceConfigInfo* resourceConfigInfo = ResourceRegistry::getInstance()->getResourceById(0x00040102);

    ASSERT_EQ(resourceConfigInfo, nullptr);
}

TEST_F(ResourceParsingTests, TestResourceParsingResourcesMerged4) {
    ResourceConfigInfo* resourceConfigInfo = ResourceRegistry::getInstance()->getResourceById(0x80040102);

    ASSERT_NE(resourceConfigInfo, nullptr);
    ASSERT_EQ(resourceConfigInfo->mResourceResType, 4);
    ASSERT_EQ(resourceConfigInfo->mResourceResID, 258);
    ASSERT_EQ(strcmp((const char*)resourceConfigInfo->mResourceName.data(), "CUSTOM_SCALING_FREQ"), 0);
    ASSERT_EQ(strcmp((const char*)resourceConfigInfo->mResourcePath.data(), "/usr/local/customfreq/node"), 0);
    ASSERT_EQ(resourceConfigInfo->mHighThreshold, 90);
    ASSERT_EQ(resourceConfigInfo->mLowThreshold, 80);
    ASSERT_EQ(resourceConfigInfo->mSupported, true);
    ASSERT_EQ(resourceConfigInfo->mPolicy, LAZY_APPLY);
    ASSERT_EQ(resourceConfigInfo->mPermissions, PERMISSION_THIRD_PARTY);
    ASSERT_EQ(resourceConfigInfo->mModes, MODE_DOZE);
    ASSERT_EQ(resourceConfigInfo->mApplyType, ResourceApplyType::APPLY_CORE);
}

TEST_F(ResourceParsingTests, TestResourceParsingResourcesMerged5) {
    ResourceConfigInfo* resourceConfigInfo = ResourceRegistry::getInstance()->getResourceById(0x800a00aa);

    ASSERT_NE(resourceConfigInfo, nullptr);
    ASSERT_EQ(resourceConfigInfo->mResourceResType, 0x0a);
    ASSERT_EQ(resourceConfigInfo->mResourceResID, 0x00aa);
    ASSERT_EQ(strcmp((const char*)resourceConfigInfo->mResourceName.data(), "CUSTOM_RESOURCE_ADDED_BY_BU"), 0);
    ASSERT_EQ(strcmp((const char*)resourceConfigInfo->mResourcePath.data(), "/some/bu/specific/node/path/customized_to_usecase"), 0);
    ASSERT_EQ(resourceConfigInfo->mHighThreshold, 512);
    ASSERT_EQ(resourceConfigInfo->mLowThreshold, 128);
    ASSERT_EQ(resourceConfigInfo->mSupported, true);
    ASSERT_EQ(resourceConfigInfo->mPolicy, LOWER_BETTER);
    ASSERT_EQ(resourceConfigInfo->mPermissions, PERMISSION_SYSTEM);
    ASSERT_EQ(resourceConfigInfo->mModes, MODE_DISPLAY_ON);
    ASSERT_EQ(resourceConfigInfo->mApplyType, ResourceApplyType::APPLY_GLOBAL);
}

TEST_F(ResourceParsingTests, TestResourceParsingResourcesDefaultValuesCheck) {
    ResourceConfigInfo* resourceConfigInfo = ResourceRegistry::getInstance()->getResourceById(0x00efffff);

    ASSERT_NE(resourceConfigInfo, nullptr);
    ASSERT_EQ(resourceConfigInfo->mResourceResType, 0xef);
    ASSERT_EQ(resourceConfigInfo->mResourceResID, 0xffff);
    ASSERT_EQ(strcmp((const char*)resourceConfigInfo->mResourceName.data(), "DEFAULT_VALUES_TEST"), 0);
    ASSERT_EQ(strcmp((const char*)resourceConfigInfo->mResourcePath.data(), ""), 0);
    ASSERT_EQ(resourceConfigInfo->mHighThreshold, -1);
    ASSERT_EQ(resourceConfigInfo->mLowThreshold, -1);
    ASSERT_EQ(resourceConfigInfo->mSupported, false);
    ASSERT_EQ(resourceConfigInfo->mPolicy, LAZY_APPLY);
    ASSERT_EQ(resourceConfigInfo->mPermissions, PERMISSION_THIRD_PARTY);
    ASSERT_EQ(resourceConfigInfo->mModes, MODE_DISPLAY_ON);
    ASSERT_EQ(resourceConfigInfo->mApplyType, ResourceApplyType::APPLY_GLOBAL);
}
