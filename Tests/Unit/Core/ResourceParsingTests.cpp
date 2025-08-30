// Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
// SPDX-License-Identifier: BSD-3-Clause-Clear

#include <thread>
#include <cstdint>

#include "TestUtils.h"
#include "ConfigProcessor.h"
#include "ResourceRegistry.h"
#include "Extensions.h"
#include "Utils.h"

#define TOTAL_RESOURCE_CONFIGS_COUNT 15

static void Init() {
    ConfigProcessor configProcessor;

    std::string resourcesClassA = "/etc/resource-tuner/tests/Configs/ResourcesConfigA.yaml";
    std::string resourcesClassB = "/etc/resource-tuner/tests/Configs/ResourcesConfigB.yaml";

    if(RC_IS_NOTOK(configProcessor.parseResourceConfigs(resourcesClassA))) {
        return;
    }

    if(RC_IS_NOTOK(configProcessor.parseResourceConfigs(resourcesClassB, true))) {
        return;
    }
}

static void TestResourceParsingSanity() {
    C_ASSERT(ResourceRegistry::getInstance() != nullptr);
}

static void TestResourceParsingResourcesParsed() {
    C_ASSERT(ResourceRegistry::getInstance()->getTotalResourcesCount() == TOTAL_RESOURCE_CONFIGS_COUNT);
}

static void TestResourceParsingResourcesMerged1() {
    ResourceConfigInfo* resourceConfigInfo = ResourceRegistry::getInstance()->getResourceById(0x80ff000b);

    C_ASSERT(resourceConfigInfo != nullptr);
    C_ASSERT(resourceConfigInfo->mResourceResType == 0xff);
    C_ASSERT(resourceConfigInfo->mResourceResID == 0x000b);
    C_ASSERT(strcmp((const char*)resourceConfigInfo->mResourceName.data(), "OVERRIDE_RESOURCE_1") == 0);
    C_ASSERT(strcmp((const char*)resourceConfigInfo->mResourcePath.data(), "/etc/resouce-tuner/tests/Configs/pathB/overwrite") == 0);
    C_ASSERT(resourceConfigInfo->mHighThreshold == 220);
    C_ASSERT(resourceConfigInfo->mLowThreshold == 150);
    C_ASSERT(resourceConfigInfo->mSupported == true);
    C_ASSERT(resourceConfigInfo->mPolicy == LOWER_BETTER);
    C_ASSERT(resourceConfigInfo->mPermissions == PERMISSION_SYSTEM);
    C_ASSERT(resourceConfigInfo->mModes == MODE_DISPLAY_ON | MODE_DOZE);
    C_ASSERT(resourceConfigInfo->mApplyType == ResourceApplyType::APPLY_CORE);
}

static void TestResourceParsingResourcesMerged2() {
    ResourceConfigInfo* resourceConfigInfo = ResourceRegistry::getInstance()->getResourceById(0x80ff1000);

    C_ASSERT(resourceConfigInfo != nullptr);
    C_ASSERT(resourceConfigInfo->mResourceResType == 0xff);
    C_ASSERT(resourceConfigInfo->mResourceResID == 0x1000);
    C_ASSERT(strcmp((const char*)resourceConfigInfo->mResourceName.data(), "CUSTOM_SCALING_FREQ") == 0);
    C_ASSERT(strcmp((const char*)resourceConfigInfo->mResourcePath.data(), "/usr/local/customfreq/node") == 0);
    C_ASSERT(resourceConfigInfo->mHighThreshold == 90);
    C_ASSERT(resourceConfigInfo->mLowThreshold == 80);
    C_ASSERT(resourceConfigInfo->mSupported == true);
    C_ASSERT(resourceConfigInfo->mPolicy == LAZY_APPLY);
    C_ASSERT(resourceConfigInfo->mPermissions == PERMISSION_THIRD_PARTY);
    C_ASSERT(resourceConfigInfo->mModes == MODE_DOZE);
    C_ASSERT(resourceConfigInfo->mApplyType == ResourceApplyType::APPLY_CORE);
}

static void TestResourceParsingResourcesMerged3() {
    ResourceConfigInfo* resourceConfigInfo = ResourceRegistry::getInstance()->getResourceById(0x80ff1001);

    C_ASSERT(resourceConfigInfo != nullptr);
    C_ASSERT(resourceConfigInfo->mResourceResType == 0xff);
    C_ASSERT(resourceConfigInfo->mResourceResID == 0x1001);
    C_ASSERT(strcmp((const char*)resourceConfigInfo->mResourceName.data(), "CUSTOM_RESOURCE_ADDED_BY_BU") == 0);
    C_ASSERT(strcmp((const char*)resourceConfigInfo->mResourcePath.data(), "/some/bu/specific/node/path/customized_to_usecase") == 0);
    C_ASSERT(resourceConfigInfo->mHighThreshold == 512);
    C_ASSERT(resourceConfigInfo->mLowThreshold == 128);
    C_ASSERT(resourceConfigInfo->mSupported == true);
    C_ASSERT(resourceConfigInfo->mPolicy == LOWER_BETTER);
    C_ASSERT(resourceConfigInfo->mPermissions == PERMISSION_SYSTEM);
    C_ASSERT(resourceConfigInfo->mModes == MODE_DISPLAY_ON);
    C_ASSERT(resourceConfigInfo->mApplyType == ResourceApplyType::APPLY_GLOBAL);
}

static void TestResourceParsingResourcesMerged4() {
    ResourceConfigInfo* resourceConfigInfo = ResourceRegistry::getInstance()->getResourceById(0x80ff000c);

    C_ASSERT(resourceConfigInfo != nullptr);
    C_ASSERT(resourceConfigInfo->mResourceResType == 0xff);
    C_ASSERT(resourceConfigInfo->mResourceResID == 0x000c);
    C_ASSERT(strcmp((const char*)resourceConfigInfo->mResourceName.data(), "OVERRIDE_RESOURCE_2") == 0);
    C_ASSERT(strcmp((const char*)resourceConfigInfo->mResourcePath.data(), "/proc/kernel/tid/kernel/uclamp.tid.sched/rt") == 0);
    C_ASSERT(resourceConfigInfo->mHighThreshold == 100022);
    C_ASSERT(resourceConfigInfo->mLowThreshold == 87755);
    C_ASSERT(resourceConfigInfo->mSupported == true);
    C_ASSERT(resourceConfigInfo->mPolicy == INSTANT_APPLY);
    C_ASSERT(resourceConfigInfo->mPermissions == PERMISSION_THIRD_PARTY);
    C_ASSERT(resourceConfigInfo->mModes == MODE_DISPLAY_ON | MODE_DISPLAY_OFF);
    C_ASSERT(resourceConfigInfo->mApplyType == ResourceApplyType::APPLY_GLOBAL);
}

static void TestResourceParsingResourcesDefaultValuesCheck() {
    ResourceConfigInfo* resourceConfigInfo = ResourceRegistry::getInstance()->getResourceById(0x00ff0009);

    C_ASSERT(resourceConfigInfo != nullptr);
    C_ASSERT(resourceConfigInfo->mResourceResType == 0xff);
    C_ASSERT(resourceConfigInfo->mResourceResID == 0x0009);
    C_ASSERT(strcmp((const char*)resourceConfigInfo->mResourceName.data(), "DEFAULT_VALUES_TEST") == 0);
    C_ASSERT(strcmp((const char*)resourceConfigInfo->mResourcePath.data(), "") == 0);
    C_ASSERT(resourceConfigInfo->mHighThreshold == -1);
    C_ASSERT(resourceConfigInfo->mLowThreshold == -1);
    C_ASSERT(resourceConfigInfo->mSupported == false);
    C_ASSERT(resourceConfigInfo->mPolicy == LAZY_APPLY);
    C_ASSERT(resourceConfigInfo->mPermissions == PERMISSION_THIRD_PARTY);
    C_ASSERT(resourceConfigInfo->mModes == MODE_DISPLAY_ON);
    C_ASSERT(resourceConfigInfo->mApplyType == ResourceApplyType::APPLY_GLOBAL);
}

int32_t main() {
    std::cout<<"Running Test Suite: [Resource Parsing Tests]\n"<<std::endl;

    Init();
    RUN_TEST(TestResourceParsingSanity);
    RUN_TEST(TestResourceParsingResourcesParsed);
    RUN_TEST(TestResourceParsingResourcesMerged1);
    RUN_TEST(TestResourceParsingResourcesMerged2);
    RUN_TEST(TestResourceParsingResourcesMerged3);
    RUN_TEST(TestResourceParsingResourcesMerged4);
    RUN_TEST(TestResourceParsingResourcesDefaultValuesCheck);

    std::cout<<"\nAll Tests from the suite: [Resource Parsing Test], executed successfully"<<std::endl;
    return 0;
}
