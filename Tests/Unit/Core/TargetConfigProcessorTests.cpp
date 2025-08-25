// Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
// SPDX-License-Identifier: BSD-3-Clause-Clear

#include <thread>
#include <cstdint>

#include "ConfigProcessor.h"
#include "ResourceRegistry.h"
#include "TargetRegistry.h"
#include "Extensions.h"
#include "Utils.h"

#define RUN_TEST(test)                                              \
do {                                                                \
    std::cout<<"Running Test: "<<#test<<std::endl;                  \
    test();                                                         \
    std::cout<<#test<<": Run Successful"<<std::endl;                \
    std::cout<<"-------------------------------------"<<std::endl;  \
} while(false);                                                     \

#define C_ASSERT(cond)                                                              \
    if(cond == false) {                                                             \
        std::cerr<<"Condition Check on line:["<<__LINE__<<"]  failed"<<std::endl; \
        std::cerr<<"Test: ["<<__func__<<"] Failed, Terminating Suite\n"<<std::endl;  \
        exit(EXIT_FAILURE);                                                         \
    }                                                                               \

static void Init() {
    ConfigProcessor configProcessor;

    if(RC_IS_NOTOK(configProcessor.parseTargetConfigs("/etc/resource-tuner/tests/Configs/TargetConfig.yaml"))) {
        return;
    }

    if(RC_IS_NOTOK(TargetRegistry::getInstance()->readPhysicalCoreClusterInfo())) {
        return;
    }
}

static void TestTargetConfigProcessorYAMLDataIntegrity1() {
    C_ASSERT(TargetRegistry::getInstance() != nullptr);
}

static void TestResourceConfigProcessorYAMLDataIntegrity2() {
    C_ASSERT(ResourceTunerSettings::targetConfigs.totalCoreCount == 16);
}

static void TestResourceConfigProcessorYAMLDataIntegrity3() {
    C_ASSERT(strcmp(ResourceTunerSettings::targetConfigs.targetName.c_str(), "qli-test") == 0);
}

static void TestResourceConfigProcessorYAMLDataIntegrity4() {
    C_ASSERT(TargetRegistry::getInstance()->getPhysicalClusterId(BIG) == 1);
    C_ASSERT(TargetRegistry::getInstance()->getPhysicalClusterId(LITTLE) == 3);
    C_ASSERT(TargetRegistry::getInstance()->getPhysicalClusterId(PRIME) == 0);
    C_ASSERT(TargetRegistry::getInstance()->getPhysicalClusterId(TITANIUM) == 2);
}

static void TestResourceConfigProcessorYAMLDataIntegrity5() {
    // Distribution
    // 0: PRIME => 0, 1, 2, 3
    // 1: BIG => 4, 5, 6, 7
    // 2: TITANIUM => 8, 9, 10, 11
    // 3: LITTLE => 12, 13, 14, 15

    // Get the first Core in the Big Cluster
    C_ASSERT(TargetRegistry::getInstance()->getPhysicalCoreId(BIG, 1) == 4);

    // Get the third Core in the Titanium Cluster
    C_ASSERT(TargetRegistry::getInstance()->getPhysicalCoreId(TITANIUM, 3) == 10);

    // Get the second Core in the Little Cluster
    C_ASSERT(TargetRegistry::getInstance()->getPhysicalCoreId(LITTLE, 2) == 13);

    // Get the fourth Core in the Prime Cluster
    C_ASSERT(TargetRegistry::getInstance()->getPhysicalCoreId(PRIME, 4) == 3);
}

int32_t main() {
    std::cout<<"Running Target Config Processing Test Suite\n"<<std::endl;

    Init();
    RUN_TEST(TestTargetConfigProcessorYAMLDataIntegrity1);
    RUN_TEST(TestResourceConfigProcessorYAMLDataIntegrity2);
    RUN_TEST(TestResourceConfigProcessorYAMLDataIntegrity3);
    RUN_TEST(TestResourceConfigProcessorYAMLDataIntegrity4);
    RUN_TEST(TestResourceConfigProcessorYAMLDataIntegrity5);

    std::cout<<"\nAll Tests from the suite: [Target Config Processing], executed successfully"<<std::endl;
}
