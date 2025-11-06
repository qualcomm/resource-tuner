// Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
// SPDX-License-Identifier: BSD-3-Clause-Clear

#include <unordered_set>

#include "ErrCodes.h"
#include "TestUtils.h"
#include "ConfigProcessor.h"
#include "ResourceRegistry.h"
#include "TargetRegistry.h"
#include "Extensions.h"
#include "Utils.h"

static ErrCode parsingStatus = RC_SUCCESS;

static void Init() {
    ConfigProcessor configProcessor;
    parsingStatus = configProcessor.parseInitConfigs("/etc/resource-tuner/custom/InitConfig.yaml");
}

static void TestInitConfigProcessorYAMLDataIntegrity1() {
    C_ASSERT(TargetRegistry::getInstance() != nullptr);\
    C_ASSERT(parsingStatus == RC_SUCCESS);
}

static void TestInitConfigProcessorYAMLDataIntegrity2() {
    std::cout<<"Count of Cgroups created: "<<TargetRegistry::getInstance()->getCreatedCGroupsCount()<<std::endl;
    C_ASSERT(TargetRegistry::getInstance()->getCreatedCGroupsCount() == 3);
}

// Note don't rely on order here, since internally CGroup mapping data is stored
// as an unordered_map.
static void TestInitConfigProcessorYAMLDataIntegrity3() {
    std::vector<std::string> cGroupNames;
    TargetRegistry::getInstance()->getCGroupNames(cGroupNames);
    std::vector<std::string> expectedNames = {"camera-cgroup", "audio-cgroup", "video-cgroup"};

    C_ASSERT(cGroupNames.size() == 3);

    std::unordered_set<std::string> expectedNamesSet;
    for(int32_t i = 0; i < cGroupNames.size(); i++) {
        expectedNamesSet.insert(cGroupNames[i]);
    }

    for(int32_t i = 0; i < expectedNames.size(); i++) {
        C_ASSERT(expectedNamesSet.find(expectedNames[i]) != expectedNamesSet.end());
    }
}

static void TestInitConfigProcessorYAMLDataIntegrity4() {
    CGroupConfigInfo* cameraConfig = TargetRegistry::getInstance()->getCGroupConfig(0);
    C_ASSERT(cameraConfig != nullptr);
    C_ASSERT(cameraConfig->mCgroupName == "camera-cgroup");
    C_ASSERT(cameraConfig->mIsThreaded == false);

    CGroupConfigInfo* videoConfig = TargetRegistry::getInstance()->getCGroupConfig(2);
    C_ASSERT(videoConfig != nullptr);
    C_ASSERT(videoConfig->mCgroupName == "video-cgroup");
    C_ASSERT(videoConfig->mIsThreaded == true);
}


static void TestInitConfigProcessorYAMLDataIntegrity5() {
    C_ASSERT(TargetRegistry::getInstance()->getCreatedMpamGroupsCount() == 3);
}

// Note don't rely on order here, since internally CGroup mapping data is stored
// as an unordered_map.
static void TestInitConfigProcessorYAMLDataIntegrity6() {
    std::vector<std::string> mpamGroupNames;
    TargetRegistry::getInstance()->getMpamGroupNames(mpamGroupNames);
    std::vector<std::string> expectedNames = {"camera-mpam-group", "audio-mpam-group", "video-mpam-group"};

    C_ASSERT(mpamGroupNames.size() == 3);

    std::unordered_set<std::string> expectedNamesSet;
    for(int32_t i = 0; i < mpamGroupNames.size(); i++) {
        expectedNamesSet.insert(mpamGroupNames[i]);
    }

    for(int32_t i = 0; i < expectedNames.size(); i++) {
        C_ASSERT(expectedNamesSet.find(expectedNames[i]) != expectedNamesSet.end());
    }
}

static void TestInitConfigProcessorYAMLDataIntegrity7() {
    MpamGroupConfigInfo* cameraConfig = TargetRegistry::getInstance()->getMpamGroupConfig(0);
    C_ASSERT(cameraConfig != nullptr);
    C_ASSERT(cameraConfig->mMpamGroupName == "camera-mpam-group");
    C_ASSERT(cameraConfig->mMpamGroupInfoID == 0);
    C_ASSERT(cameraConfig->mPriority == 0);

    MpamGroupConfigInfo* videoConfig = TargetRegistry::getInstance()->getMpamGroupConfig(2);
    C_ASSERT(videoConfig != nullptr);
    C_ASSERT(videoConfig->mMpamGroupName == "video-mpam-group");
    C_ASSERT(videoConfig->mMpamGroupInfoID == 2);
    C_ASSERT(videoConfig->mPriority == 2);
}

int32_t main() {
    std::cout<<"Running Test Suite: [InitConfigParsingTests]\n"<<std::endl;

    Init();
    RUN_TEST(TestInitConfigProcessorYAMLDataIntegrity1);
    RUN_TEST(TestInitConfigProcessorYAMLDataIntegrity2);
    RUN_TEST(TestInitConfigProcessorYAMLDataIntegrity3);
    RUN_TEST(TestInitConfigProcessorYAMLDataIntegrity4);
    RUN_TEST(TestInitConfigProcessorYAMLDataIntegrity5);
    RUN_TEST(TestInitConfigProcessorYAMLDataIntegrity6);
    RUN_TEST(TestInitConfigProcessorYAMLDataIntegrity7);

    std::cout<<"\nAll Tests from the suite: [InitConfigParsingTests], executed successfully"<<std::endl;
}
