// Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
// SPDX-License-Identifier: BSD-3-Clause-Clear

#include <thread>
#include <cstdint>

#include "TestUtils.h"
#include "ConfigProcessor.h"
#include "ResourceRegistry.h"
#include "Extensions.h"
#include "Utils.h"

#define TOTAL_RESOURCE_CONFIGS_COUNT 12

#define GENERATE_RESOURCE_ID(restype, rescode) ({                              \
    uint32_t resourceBitmap = 0;                                               \
    resourceBitmap |= (1 << 31);                                               \
    resourceBitmap |= ((uint32_t)rescode);                                     \
    resourceBitmap |= ((uint32_t)restype << 16);                               \
    resourceBitmap;                                                            \
})

static void Init() {
    ConfigProcessor configProcessor;

    if(RC_IS_NOTOK(configProcessor.parseResourceConfigs("/etc/resource-tuner/tests/Configs/ResourcesConfig.yaml", true))) {
        return;
    }
}

static void TestResourceConfigProcessorYAMLDataIntegrity1() {
    C_ASSERT(ResourceRegistry::getInstance() != nullptr);
}

static void TestResourceConfigProcessorYAMLDataIntegrity2() {
    C_ASSERT(ResourceRegistry::getInstance()->getTotalResourcesCount() == TOTAL_RESOURCE_CONFIGS_COUNT);
}

static void TestResourceConfigProcessorYAMLDataIntegrity3_1() {
    ResourceConfigInfo* resourceConfigInfo = ResourceRegistry::getInstance()->getResourceById(GENERATE_RESOURCE_ID(1, 0));

    C_ASSERT(resourceConfigInfo != nullptr);
    C_ASSERT(resourceConfigInfo->mResourceResType == 1);
    C_ASSERT(resourceConfigInfo->mResourceResID == 0);
    C_ASSERT(strcmp((const char*)resourceConfigInfo->mResourceName.data(), "TEST_RESOURCE_1") == 0);
    C_ASSERT(strcmp((const char*)resourceConfigInfo->mResourcePath.data(), "/etc/resource-tuner/tests/Configs/ResourceSysFsNodes/sched_util_clamp_min") == 0);
    C_ASSERT(resourceConfigInfo->mHighThreshold == 1024);
    C_ASSERT(resourceConfigInfo->mLowThreshold == 0);
    C_ASSERT(resourceConfigInfo->mSupported == true);
    C_ASSERT(resourceConfigInfo->mPolicy == HIGHER_BETTER);
    C_ASSERT(resourceConfigInfo->mPermissions == PERMISSION_THIRD_PARTY);
    C_ASSERT(resourceConfigInfo->mModes == MODE_DISPLAY_ON | MODE_DOZE);
    C_ASSERT(resourceConfigInfo->mApplyType == ResourceApplyType::APPLY_GLOBAL);
}

static void TestResourceConfigProcessorYAMLDataIntegrity3_2() {
    ResourceConfigInfo* resourceConfigInfo = ResourceRegistry::getInstance()->getResourceById(GENERATE_RESOURCE_ID(1, 1));

    C_ASSERT(resourceConfigInfo != nullptr);
    C_ASSERT(resourceConfigInfo->mResourceResType == 1);
    C_ASSERT(resourceConfigInfo->mResourceResID == 1);
    C_ASSERT(strcmp((const char*)resourceConfigInfo->mResourceName.data(), "TEST_RESOURCE_2") == 0);
    C_ASSERT(strcmp((const char*)resourceConfigInfo->mResourcePath.data(), "/etc/resource-tuner/tests/Configs/ResourceSysFsNodes/sched_util_clamp_max") == 0);
    C_ASSERT(resourceConfigInfo->mHighThreshold == 1024);
    C_ASSERT(resourceConfigInfo->mLowThreshold == 512);
    C_ASSERT(resourceConfigInfo->mSupported == true);
    C_ASSERT(resourceConfigInfo->mPolicy == HIGHER_BETTER);
    C_ASSERT(resourceConfigInfo->mPermissions == PERMISSION_THIRD_PARTY);
    C_ASSERT(resourceConfigInfo->mModes == MODE_DISPLAY_ON | MODE_DOZE);
    C_ASSERT(resourceConfigInfo->mApplyType == ResourceApplyType::APPLY_GLOBAL);
}

static void TestResourceConfigProcessorYAMLDataIntegrity3_3() {
    ResourceConfigInfo* resourceConfigInfo = ResourceRegistry::getInstance()->getResourceById(GENERATE_RESOURCE_ID(1, 5));

    C_ASSERT(resourceConfigInfo != nullptr);
    C_ASSERT(resourceConfigInfo->mResourceResType == 1);
    C_ASSERT(resourceConfigInfo->mResourceResID == 5);
    C_ASSERT(strcmp((const char*)resourceConfigInfo->mResourceName.data(), "TEST_RESOURCE_6") == 0);
    C_ASSERT(strcmp((const char*)resourceConfigInfo->mResourcePath.data(), "/etc/resource-tuner/tests/Configs/ResourceSysFsNodes/target_test_resource2") == 0);
    C_ASSERT(resourceConfigInfo->mHighThreshold == 6500);
    C_ASSERT(resourceConfigInfo->mLowThreshold == 50);
    C_ASSERT(resourceConfigInfo->mSupported == true);
    C_ASSERT(resourceConfigInfo->mPolicy == HIGHER_BETTER);
    C_ASSERT(resourceConfigInfo->mPermissions == PERMISSION_THIRD_PARTY);
    C_ASSERT(resourceConfigInfo->mModes == MODE_DISPLAY_ON);
    C_ASSERT(resourceConfigInfo->mApplyType == ResourceApplyType::APPLY_CORE);
}

int main() {
    std::cout<<"Running Test Suite: [Resource Processor Tests]\n"<<std::endl;

    Init();
    RUN_TEST(TestResourceConfigProcessorYAMLDataIntegrity1);
    RUN_TEST(TestResourceConfigProcessorYAMLDataIntegrity2);
    RUN_TEST(TestResourceConfigProcessorYAMLDataIntegrity3_1);
    RUN_TEST(TestResourceConfigProcessorYAMLDataIntegrity3_2);
    RUN_TEST(TestResourceConfigProcessorYAMLDataIntegrity3_3);

    std::cout<<"\nAll Tests from the suite: [Resource Processor Tests], executed successfully"<<std::endl;
    return 0;
}
