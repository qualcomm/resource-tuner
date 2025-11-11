// Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
// SPDX-License-Identifier: BSD-3-Clause-Clear

#include <cstring>

#include "ErrCodes.h"
#include "TestUtils.h"
#include "SignalConfigProcessor.h"
#include "SignalRegistry.h"
#include "Extensions.h"
#include "Utils.h"

#define TOTAL_SIGNAL_CONFIGS_COUNT 8

static ErrCode parsingStatus = RC_SUCCESS;

static void Init() {
    SignalConfigProcessor configProcessor;
    parsingStatus = configProcessor.parseSignalConfigs("/etc/resource-tuner/custom/SignalsConfig.yaml");
}

static void TestSignalConfigProcessorYAMLDataIntegrity1() {
    C_ASSERT(SignalRegistry::getInstance() != nullptr);
    C_ASSERT(parsingStatus == RC_SUCCESS);
}

static void TestSignalConfigProcessorYAMLDataIntegrity2() {
    std::cout<<"Count of Signals Parsed = "<<SignalRegistry::getInstance()->getSignalsConfigCount()<<std::endl;
    C_ASSERT(SignalRegistry::getInstance()->getSignalsConfigCount() == TOTAL_SIGNAL_CONFIGS_COUNT);
}

static void TestSignalConfigProcessorYAMLDataIntegrity3_1() {
    SignalInfo* signalInfo = SignalRegistry::getInstance()->getSignalConfigById(0x000d0000);

    C_ASSERT(signalInfo != nullptr);
    C_ASSERT(signalInfo->mSignalID == 0);
    C_ASSERT(signalInfo->mSignalCategory == 0x0d);
    C_ASSERT(strcmp((const char*)signalInfo->mSignalName.data(), "TEST_SIGNAL_1") == 0);
    C_ASSERT(signalInfo->mTimeout == 4000);

    C_ASSERT(signalInfo->mPermissions != nullptr);
    C_ASSERT(signalInfo->mDerivatives != nullptr);
    C_ASSERT(signalInfo->mSignalResources != nullptr);

    C_ASSERT(signalInfo->mPermissions->size() == 1);
    C_ASSERT(signalInfo->mDerivatives->size() == 1);
    C_ASSERT(signalInfo->mSignalResources->size() == 1);

    C_ASSERT(signalInfo->mPermissions->at(0) == PERMISSION_THIRD_PARTY);

    C_ASSERT(strcmp((const char*)signalInfo->mDerivatives->at(0).data(), "solar") == 0);

    Resource* resource1 = signalInfo->mSignalResources->at(0);
    C_ASSERT(resource1 != nullptr);
    C_ASSERT(resource1->getResCode() == 2147549184);
    C_ASSERT(resource1->getValuesCount() == 1);
    C_ASSERT(resource1->mResValue.value == 700);
    C_ASSERT(resource1->getResInfo() == 0);
}

static void TestSignalConfigProcessorYAMLDataIntegrity3_2() {
    SignalInfo* signalInfo = SignalRegistry::getInstance()->getSignalConfigById(0x000d0001);

    C_ASSERT(signalInfo != nullptr);
    C_ASSERT(signalInfo->mSignalID == 1);
    C_ASSERT(signalInfo->mSignalCategory == 0x0d);
    C_ASSERT(strcmp((const char*)signalInfo->mSignalName.data(), "TEST_SIGNAL_2") == 0);
    C_ASSERT(signalInfo->mTimeout == 5000);

    C_ASSERT(signalInfo->mPermissions != nullptr);
    C_ASSERT(signalInfo->mDerivatives != nullptr);
    C_ASSERT(signalInfo->mSignalResources != nullptr);

    C_ASSERT(signalInfo->mPermissions->size() == 1);
    C_ASSERT(signalInfo->mDerivatives->size() == 1);
    C_ASSERT(signalInfo->mSignalResources->size() == 2);

    C_ASSERT(signalInfo->mPermissions->at(0) == PERMISSION_SYSTEM);

    C_ASSERT(strcmp((const char*)signalInfo->mDerivatives->at(0).data(), "derivative_v2") == 0);

    Resource* resource1 = signalInfo->mSignalResources->at(0);
    C_ASSERT(resource1->getResCode() == 8);
    C_ASSERT(resource1->getValuesCount() == 1);
    C_ASSERT(resource1->mResValue.value == 814);
    C_ASSERT(resource1->getResInfo() == 0);

    Resource* resource2 = signalInfo->mSignalResources->at(1);
    C_ASSERT(resource2->getResCode() == 15);
    C_ASSERT(resource2->getValuesCount() == 2);
    C_ASSERT(((IntIterable*)(resource2->mResValue.values->getNth(0)))->mData == 23);
    C_ASSERT(((IntIterable*)(resource2->mResValue.values->getNth(1)))->mData == 90);
    C_ASSERT(resource2->getResInfo() == 256);
}

static void TestSignalConfigProcessorYAMLDataIntegrity3_3() {
    SignalInfo* signalInfo = SignalRegistry::getInstance()->getSignalConfigById(0x000d0003);

    C_ASSERT(signalInfo != nullptr);
    C_ASSERT(signalInfo->mSignalID == 3);
    C_ASSERT(signalInfo->mSignalCategory == 0x0d);
    C_ASSERT(strcmp((const char*)signalInfo->mSignalName.data(), "TEST_SIGNAL_4") == 0);
    C_ASSERT(signalInfo->mTimeout == 4000);

    C_ASSERT(signalInfo->mPermissions != nullptr);
    C_ASSERT(signalInfo->mDerivatives != nullptr);
    C_ASSERT(signalInfo->mSignalResources != nullptr);

    C_ASSERT(signalInfo->mPermissions->size() == 1);
    C_ASSERT(signalInfo->mDerivatives->size() == 1);
    C_ASSERT(signalInfo->mSignalResources->size() == 4);

    C_ASSERT(signalInfo->mPermissions->at(0) == PERMISSION_THIRD_PARTY);

    C_ASSERT(strcmp((const char*)signalInfo->mDerivatives->at(0).data(), "solar") == 0);

    Resource* resource1 = signalInfo->mSignalResources->at(0);
    C_ASSERT(resource1->getResCode() == 8);
    C_ASSERT(resource1->getValuesCount() == 2);
    C_ASSERT(((IntIterable*)(resource1->mResValue.values->getNth(0)))->mData == 300);
    C_ASSERT(((IntIterable*)(resource1->mResValue.values->getNth(1)))->mData == 400);
    C_ASSERT(resource1->getResInfo() == 0);

    Resource* resource2 = signalInfo->mSignalResources->at(1);
    C_ASSERT(resource2->getResCode() == 241);
    C_ASSERT(resource2->getValuesCount() == 3);
    C_ASSERT(((IntIterable*)(resource2->mResValue.values->getNth(0)))->mData == 12);
    C_ASSERT(((IntIterable*)(resource2->mResValue.values->getNth(1)))->mData == 45);
    C_ASSERT(((IntIterable*)(resource2->mResValue.values->getNth(2)))->mData == 67);
    C_ASSERT(resource2->getResInfo() == 1024);

    Resource* resource3 = signalInfo->mSignalResources->at(2);
    C_ASSERT(resource3->getResCode() == 43981);
    C_ASSERT(resource3->getValuesCount() == 1);
    C_ASSERT(resource3->mResValue.value == 5);
    C_ASSERT(resource3->getResInfo() == 32);

    Resource* resource4 = signalInfo->mSignalResources->at(3);
    C_ASSERT(resource4->getResCode() == 59917);
    C_ASSERT(resource4->getValuesCount() == 1);
    C_ASSERT(resource4->mResValue.value == 87);
    C_ASSERT(resource4->getResInfo() == 512);
}

static void TestSignalConfigProcessorYAMLDataIntegrity3_4() {
    SignalInfo* signalInfo = SignalRegistry::getInstance()->getSignalConfigById(0x000d0007);

    C_ASSERT(signalInfo != nullptr);
    C_ASSERT(signalInfo->mSignalID == 0x0007);
    C_ASSERT(signalInfo->mSignalCategory == 0x0d);
    C_ASSERT(strcmp((const char*)signalInfo->mSignalName.data(), "TEST_SIGNAL_8") == 0);
    C_ASSERT(signalInfo->mTimeout == 5500);

    C_ASSERT(signalInfo->mPermissions != nullptr);
    C_ASSERT(signalInfo->mDerivatives == nullptr);
    C_ASSERT(signalInfo->mSignalResources != nullptr);

    C_ASSERT(signalInfo->mPermissions->size() == 1);
    C_ASSERT(signalInfo->mSignalResources->size() == 2);

    C_ASSERT(signalInfo->mPermissions->at(0) == PERMISSION_THIRD_PARTY);

    Resource* resource1 = signalInfo->mSignalResources->at(0);
    C_ASSERT(resource1->getResCode() == 0x000900aa);
    C_ASSERT(resource1->getValuesCount() == 3);
    C_ASSERT(((IntIterable*)(resource1->mResValue.values->getNth(0)))->mData == -1);
    C_ASSERT(((IntIterable*)(resource1->mResValue.values->getNth(1)))->mData == -1);
    C_ASSERT(((IntIterable*)(resource1->mResValue.values->getNth(2)))->mData == 68);
    C_ASSERT(resource1->getResInfo() == 0);

    Resource* resource2 = signalInfo->mSignalResources->at(1);
    C_ASSERT(resource2->getResCode() == 0x000900dc);
    C_ASSERT(resource2->getValuesCount() == 4);
    C_ASSERT(((IntIterable*)(resource2->mResValue.values->getNth(0)))->mData == -1);
    C_ASSERT(((IntIterable*)(resource2->mResValue.values->getNth(1)))->mData == -1);
    C_ASSERT(((IntIterable*)(resource2->mResValue.values->getNth(2)))->mData == 50);
    C_ASSERT(((IntIterable*)(resource2->mResValue.values->getNth(3)))->mData == 512);
    C_ASSERT(resource2->getResInfo() == 0);
}

static void TestSignalConfigProcessorYAMLDataIntegrity4() {
    std::vector<SignalInfo*> signalConfigs = SignalRegistry::getInstance()->getSignalConfigs();
    C_ASSERT(signalConfigs.size() == TOTAL_SIGNAL_CONFIGS_COUNT);

    std::vector<std::string> signalNames;

    for(int32_t i = 1; i <= TOTAL_SIGNAL_CONFIGS_COUNT; i++) {
        char buf[32];
        snprintf(buf, sizeof(buf), "TEST_SIGNAL_%d", i);
        signalNames.push_back(std::string(buf));
    }

    C_ASSERT(signalNames.size() == signalConfigs.size());

    for(int32_t i = 0; i < signalConfigs.size(); i++) {
        C_ASSERT(signalConfigs[i]->mSignalName == signalNames[i]);
    }
}

int32_t main() {
    std::cout<<"Running Test Suite: [SignalConfigProcessorTests]\n"<<std::endl;

    Init();
    RUN_TEST(TestSignalConfigProcessorYAMLDataIntegrity1);
    RUN_TEST(TestSignalConfigProcessorYAMLDataIntegrity2);
    RUN_TEST(TestSignalConfigProcessorYAMLDataIntegrity3_1);
    RUN_TEST(TestSignalConfigProcessorYAMLDataIntegrity3_2);
    RUN_TEST(TestSignalConfigProcessorYAMLDataIntegrity3_3);
    RUN_TEST(TestSignalConfigProcessorYAMLDataIntegrity3_4);
    RUN_TEST(TestSignalConfigProcessorYAMLDataIntegrity4);

    std::cout<<"\nAll Tests from the suite: [SignalConfigProcessorTests], executed successfully"<<std::endl;
    return 0;
}
