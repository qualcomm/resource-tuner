// Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
// SPDX-License-Identifier: BSD-3-Clause-Clear

#include <thread>
#include <cstdint>

#include "ConfigProcessor.h"
#include "ResourceRegistry.h"
#include "Extensions.h"
#include "Utils.h"

#define RUN_TEST(test)                                              \
do {                                                                \
    std::cout<<"Running Test: "<<#test<<std::endl;                  \
    test();                                                         \
    std::cout<<#test<<": Run Successful"<<std::endl;                \
    std::cout<<"-------------------------------------"<<std::endl;  \
} while(false);                                                     \

#define C_ASSERT(cond)                                                               \
    if(cond == false) {                                                              \
        std::cerr<<"Condition Check on line:["<<__LINE__<<"]  failed"<<std::endl;    \
        std::cerr<<"Test: ["<<__func__<<"] Failed, Terminating Suite\n"<<std::endl;  \
        exit(EXIT_FAILURE);                                                          \
    }                                                                                \

#define TOTAL_RESOURCE_CONFIGS_COUNT 38

static void Init() {
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

static void TestResourceParsingSanity() {
    C_ASSERT(ResourceRegistry::getInstance() != nullptr);
}

static void TestResourceParsingResourcesParsed() {
    C_ASSERT(ResourceRegistry::getInstance()->getTotalResourcesCount() == TOTAL_RESOURCE_CONFIGS_COUNT);
}

// static void TestResourceParsingResourcesMerged1() {
//     ResourceConfigInfo* resourceConfigInfo = ResourceRegistry::getInstance()->getResourceById(0x00030000);

//     C_ASSERT(resourceConfigInfo != nullptr);
//     C_ASSERT(resourceConfigInfo->mResourceResType == 3);
//     C_ASSERT(resourceConfigInfo->mResourceResID == 0);
//     C_ASSERT(strcmp((const char*)resourceConfigInfo->mResourceName.data(), "OVERWRITE_RESOURCE_1") == 0);
//     C_ASSERT(strcmp((const char*)resourceConfigInfo->mResourcePath.data(), "/proc/sys/kernel/test_node1") == 0);
//     C_ASSERT(resourceConfigInfo->mHighThreshold == 1024);
//     C_ASSERT(resourceConfigInfo->mLowThreshold == 0);
//     C_ASSERT(resourceConfigInfo->mSupported == true);
//     C_ASSERT(resourceConfigInfo->mPolicy == LOWER_BETTER);
//     C_ASSERT(resourceConfigInfo->mPermissions == PERMISSION_SYSTEM);
//     C_ASSERT(resourceConfigInfo->mModes == MODE_DISPLAY_ON | MODE_DOZE);
//     C_ASSERT(resourceConfigInfo->mApplyType == ResourceApplyType::APPLY_GLOBAL);
// }

// static void TestResourceParsingResourcesMerged2() {
//     ResourceConfigInfo* resourceConfigInfo = ResourceRegistry::getInstance()->getResourceById(0x00040003);

//     C_ASSERT(resourceConfigInfo != nullptr);
//     C_ASSERT(resourceConfigInfo->mResourceResType == 4);
//     C_ASSERT(resourceConfigInfo->mResourceResID == 3);
//     C_ASSERT(strcmp((const char*)resourceConfigInfo->mResourceName.data(), "OVERWRITE_RESOURCE_2") == 0);
//     C_ASSERT(strcmp((const char*)resourceConfigInfo->mResourcePath.data(), "/proc/sys/kernel/test_node2") == 0);
//     C_ASSERT(resourceConfigInfo->mHighThreshold == 1024);
//     C_ASSERT(resourceConfigInfo->mLowThreshold == 890);
//     C_ASSERT(resourceConfigInfo->mSupported == true);
//     C_ASSERT(resourceConfigInfo->mPolicy == INSTANT_APPLY);
//     C_ASSERT(resourceConfigInfo->mPermissions == PERMISSION_SYSTEM);
//     C_ASSERT(resourceConfigInfo->mModes == MODE_DISPLAY_ON | MODE_DISPLAY_OFF);
//     C_ASSERT(resourceConfigInfo->mApplyType == ResourceApplyType::APPLY_CLUSTER);
// }

static void TestResourceParsingResourcesMerged3() {
    ResourceConfigInfo* resourceConfigInfo = ResourceRegistry::getInstance()->getResourceById(0x00040102);

    C_ASSERT(resourceConfigInfo == nullptr);
}

static void TestResourceParsingResourcesMerged4() {
    ResourceConfigInfo* resourceConfigInfo = ResourceRegistry::getInstance()->getResourceById(0x80040102);

    C_ASSERT(resourceConfigInfo != nullptr);
    C_ASSERT(resourceConfigInfo->mResourceResType == 4);
    C_ASSERT(resourceConfigInfo->mResourceResID == 258);
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

static void TestResourceParsingResourcesMerged5() {
    ResourceConfigInfo* resourceConfigInfo = ResourceRegistry::getInstance()->getResourceById(0x800a00aa);

    C_ASSERT(resourceConfigInfo != nullptr);
    C_ASSERT(resourceConfigInfo->mResourceResType == 0x0a);
    C_ASSERT(resourceConfigInfo->mResourceResID == 0x00aa);
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

static void TestResourceParsingResourcesDefaultValuesCheck() {
    ResourceConfigInfo* resourceConfigInfo = ResourceRegistry::getInstance()->getResourceById(0x00efffff);

    C_ASSERT(resourceConfigInfo != nullptr);
    C_ASSERT(resourceConfigInfo->mResourceResType == 0xef);
    C_ASSERT(resourceConfigInfo->mResourceResID == 0xffff);
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

int main() {
    std::cout<<"Running Test Suite: [Resource Parsing Tests]\n"<<std::endl;

    RUN_TEST(TestResourceParsingSanity);
    RUN_TEST(TestResourceParsingResourcesParsed);
    RUN_TEST(TestResourceParsingResourcesMerged3);
    RUN_TEST(TestResourceParsingResourcesMerged4);
    RUN_TEST(TestResourceParsingResourcesMerged5);
    RUN_TEST(TestResourceParsingResourcesDefaultValuesCheck);

    std::cout<<"\nAll Tests from the suite: [Resource Parsing Test], executed successfully"<<std::endl;
    return 0;
}
