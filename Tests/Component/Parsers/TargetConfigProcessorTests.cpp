// Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
// SPDX-License-Identifier: BSD-3-Clause-Clear

#include "ErrCodes.h"
#include "TestUtils.h"
#include "ConfigProcessor.h"
#include "ResourceRegistry.h"
#include "TargetRegistry.h"
#include "Extensions.h"
#include "Utils.h"

static ErrCode parsingStatus = RC_SUCCESS;

static void Init() {
    ResourceTunerSettings::targetConfigs.targetName = "TestDevice";
    ConfigProcessor configProcessor;
    parsingStatus = configProcessor.parseTargetConfigs("/etc/resource-tuner/custom/TargetConfigDup.yaml");
}

static void TestTargetConfigProcessorYAMLDataIntegrity1() {
    C_ASSERT(TargetRegistry::getInstance() != nullptr);
    C_ASSERT(parsingStatus == RC_SUCCESS);
}

static void TestTargetConfigProcessorYAMLDataIntegrity2() {
    std::cout<<"Determined Cluster Count = "<<ResourceTunerSettings::targetConfigs.mTotalClusterCount<<std::endl;
    C_ASSERT(ResourceTunerSettings::targetConfigs.mTotalClusterCount == 4);
}

static void TestTargetConfigProcessorYAMLDataIntegrity3() {
    C_ASSERT(TargetRegistry::getInstance()->getPhysicalClusterId(0) == 4);
    C_ASSERT(TargetRegistry::getInstance()->getPhysicalClusterId(1) == 0);
    C_ASSERT(TargetRegistry::getInstance()->getPhysicalClusterId(2) == 9);
    C_ASSERT(TargetRegistry::getInstance()->getPhysicalClusterId(3) == 7);
}

static void TestTargetConfigProcessorYAMLDataIntegrity4() {
    // Distribution of physical clusters
    // 1:0 => 0, 1, 2, 3
    // 0:4 => 4, 5, 6
    // 3:7 => 7, 8
    // 2:9 => 9
    C_ASSERT(TargetRegistry::getInstance()->getPhysicalCoreId(1, 3) == 2);

    C_ASSERT(TargetRegistry::getInstance()->getPhysicalCoreId(0, 2) == 5);

    C_ASSERT(TargetRegistry::getInstance()->getPhysicalCoreId(3, 1) == 7);

    C_ASSERT(TargetRegistry::getInstance()->getPhysicalCoreId(2, 1) == 9);
}

int32_t main() {
    std::cout<<"Running Test Suite: [TargetConfigProcessorTests]\n"<<std::endl;

    Init();
    RUN_TEST(TestTargetConfigProcessorYAMLDataIntegrity1);
    RUN_TEST(TestTargetConfigProcessorYAMLDataIntegrity2);
    RUN_TEST(TestTargetConfigProcessorYAMLDataIntegrity3);
    RUN_TEST(TestTargetConfigProcessorYAMLDataIntegrity4);

    std::cout<<"\nAll Tests from the suite: [TargetConfigProcessorTests], executed successfully"<<std::endl;
}
