// Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
// SPDX-License-Identifier: BSD-3-Clause-Clear

#include <thread>
#include <cstdint>
#include <cstring>

#include "TestUtils.h"
#include "SignalConfigProcessor.h"
#include "ExtFeaturesRegistry.h"
#include "Extensions.h"
#include "Utils.h"

#define TOTAL_EXT_FEAT_CONFIGS_COUNT 2

static void Init() {
    SignalConfigProcessor configProcessor;

    if(RC_IS_NOTOK(configProcessor.parseExtFeaturesConfigs("/etc/resource-tuner/custom/ExtFeaturesConfig.yaml"))) {
        return;
    }
}

static void TestExtFearConfigProcessorYAMLDataIntegrity1() {
    C_ASSERT(ExtFeaturesRegistry::getInstance() != nullptr);
}

static void TestExtFearConfigProcessorYAMLDataIntegrity2() {
    std::cout<<"Count of Features Parsed: "<<ExtFeaturesRegistry::getInstance()->getExtFeaturesConfigCount()<<std::endl;
    C_ASSERT(ExtFeaturesRegistry::getInstance()->getExtFeaturesConfigCount() == TOTAL_EXT_FEAT_CONFIGS_COUNT);
}

static void TestExtFearConfigProcessorYAMLDataIntegrity3() {
    ExtFeatureInfo* feature =
        ExtFeaturesRegistry::getInstance()->getExtFeatureConfigById(0x00000001);

    C_ASSERT(feature != nullptr);
    C_ASSERT(feature->mFeatureId == 0x00000001);
    C_ASSERT(feature->mFeatureName == "FEAT-1");
    C_ASSERT(feature->mFeatureLib == "/usr/lib/libtesttuner.so");

    C_ASSERT(feature->mSignalsSubscribedTo != nullptr);
    C_ASSERT(feature->mSignalsSubscribedTo->size() == 2);
    C_ASSERT((*feature->mSignalsSubscribedTo)[0] == 0x000dbbca);
    C_ASSERT((*feature->mSignalsSubscribedTo)[1] == 0x000a00ff);
}

static void TestExtFearConfigProcessorYAMLDataIntegrity4() {
    ExtFeatureInfo* feature =
        ExtFeaturesRegistry::getInstance()->getExtFeatureConfigById(0x00000002);

    C_ASSERT(feature != nullptr);
    C_ASSERT(feature->mFeatureId == 0x00000002);
    C_ASSERT(feature->mFeatureName == "FEAT-2");
    C_ASSERT(feature->mFeatureLib == "/usr/lib/libpropagate.so");

    C_ASSERT(feature->mSignalsSubscribedTo != nullptr);
    C_ASSERT(feature->mSignalsSubscribedTo->size() == 2);
    C_ASSERT((*feature->mSignalsSubscribedTo)[0] == 0x80a105ea);
    C_ASSERT((*feature->mSignalsSubscribedTo)[1] == 0x800ccca5);
}

int32_t main() {
    std::cout<<"Running Test Suite: [ExtFeaturesParsingTests]\n"<<std::endl;

    Init();
    RUN_TEST(TestExtFearConfigProcessorYAMLDataIntegrity1);
    RUN_TEST(TestExtFearConfigProcessorYAMLDataIntegrity2);
    RUN_TEST(TestExtFearConfigProcessorYAMLDataIntegrity3);
    RUN_TEST(TestExtFearConfigProcessorYAMLDataIntegrity4);

    std::cout<<"\nAll Tests from the suite: [ExtFeaturesParsingTests], executed successfully"<<std::endl;
    return 0;
}
