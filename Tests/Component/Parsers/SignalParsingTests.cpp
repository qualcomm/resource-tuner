// Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
// SPDX-License-Identifier: BSD-3-Clause-Clear

#include <thread>
#include <cstdint>
#include <cstring>

#include "SignalConfigProcessor.h"
#include "SignalRegistry.h"
#include "Extensions.h"
#include "Utils.h"

#define TOTAL_SIGNAL_CONFIGS_COUNT 10

#define RUN_TEST(test)                                              \
do {                                                                \
    std::cout<<"Running Test: "<<#test<<std::endl;                  \
    test();                                                         \
    std::cout<<#test<<": Run Successful"<<std::endl;                \
    std::cout<<"-------------------------------------"<<std::endl;  \
} while(false);                                                     \

#define C_ASSERT(cond)                                                                          \
    if(cond == false) {                                                                         \
        std::cerr<<"Assertion failed at line [" << __LINE__ << "]: "<<#cond<<std::endl;         \
        std::cerr<<"Test: ["<<__func__<<"] Failed, Terminating Suite\n"<<std::endl;             \
        exit(EXIT_FAILURE);                                                                     \
    }                                                                                           \

static void Init() {
    SignalConfigProcessor configProcessor;

    std::string signalsClassA = "/etc/resource-tuner/tests/Configs/SignalsConfigA.yaml";
    std::string signalsClassB = "/etc/resource-tuner/tests/Configs/SignalsConfigB.yaml";

    if(RC_IS_NOTOK(configProcessor.parseSignalConfigs(signalsClassA))) {
        return;
    }

    if(RC_IS_NOTOK(configProcessor.parseSignalConfigs(signalsClassB, true))) {
        return;
    }
}

static void TestSignalParsingSanity() {
    C_ASSERT(SignalRegistry::getInstance() != nullptr);
}

static void TestSignalParsingSignalsParsed() {
    C_ASSERT(SignalRegistry::getInstance()->getSignalsConfigCount() == TOTAL_SIGNAL_CONFIGS_COUNT);
}

static void TestSignalParsingSignalsMerged1() {
    SignalInfo* signalInfo = SignalRegistry::getInstance()->getSignalConfigById(0x00010000);

    C_ASSERT(signalInfo != nullptr);
    C_ASSERT(signalInfo->mSignalID == 0);
    C_ASSERT(signalInfo->mSignalCategory == 1);
    C_ASSERT(strcmp((const char*)signalInfo->mSignalName.data(), "OVERRIDE_SIGNAL_1") == 0);
    C_ASSERT(signalInfo->mIsEnabled == true);
    C_ASSERT(signalInfo->mTimeout == 14500);

    C_ASSERT(signalInfo->mTargetsDisabled == nullptr);
    C_ASSERT(signalInfo->mTargetsEnabled != nullptr);
    C_ASSERT(signalInfo->mPermissions != nullptr);
    C_ASSERT(signalInfo->mDerivatives != nullptr);
    C_ASSERT(signalInfo->mSignalResources != nullptr);

    C_ASSERT(signalInfo->mTargetsEnabled->size() == 1);
    C_ASSERT(signalInfo->mPermissions->size() == 1);
    C_ASSERT(signalInfo->mDerivatives->size() == 1);
    C_ASSERT(signalInfo->mSignalResources->size() == 1);

    // Note the target values are converted to LowerCase before inserting into the map
    C_ASSERT(signalInfo->mTargetsEnabled->count("qti-cli-test") == 1);

    C_ASSERT(signalInfo->mPermissions->at(0) == PERMISSION_SYSTEM);

    C_ASSERT(strcmp((const char*)signalInfo->mDerivatives->at(0).data(), "test-derivative") == 0);

    Resource* resource1 = signalInfo->mSignalResources->at(0);
    C_ASSERT(resource1->getResCode() == 0x80dbaaa0);
    C_ASSERT(resource1->getValuesCount() == 1);
    C_ASSERT(resource1->mResValue.value == 887);
    C_ASSERT(resource1->getResInfo() == 0x000776aa);
}

static void TestSignalParsingSignalsMerged2() {
    SignalInfo* signalInfo = SignalRegistry::getInstance()->getSignalConfigById(0x00080002);

    C_ASSERT(signalInfo != nullptr);
    C_ASSERT(signalInfo->mSignalID == 2);
    C_ASSERT(signalInfo->mSignalCategory == 0x08);
    C_ASSERT(strcmp((const char*)signalInfo->mSignalName.data(), "MOVE_TID_CUSTOMIZABLE") == 0);
    C_ASSERT(signalInfo->mIsEnabled == true);
    C_ASSERT(signalInfo->mTimeout == 5500);

    C_ASSERT(signalInfo->mTargetsDisabled == nullptr);
    C_ASSERT(signalInfo->mTargetsEnabled == nullptr);
    C_ASSERT(signalInfo->mPermissions != nullptr);
    C_ASSERT(signalInfo->mDerivatives == nullptr);
    C_ASSERT(signalInfo->mSignalResources != nullptr);

    C_ASSERT(signalInfo->mPermissions->size() == 1);
    C_ASSERT(signalInfo->mSignalResources->size() == 2);

    C_ASSERT(signalInfo->mPermissions->at(0) == PERMISSION_THIRD_PARTY);

    Resource* resource1 = signalInfo->mSignalResources->at(0);
    C_ASSERT(resource1->getResCode() == 0x000900aa);
    C_ASSERT(resource1->getValuesCount() == 3);
    C_ASSERT((*resource1->mResValue.values)[0] == 1);
    C_ASSERT((*resource1->mResValue.values)[1] == -1);
    C_ASSERT((*resource1->mResValue.values)[2] == 68);
    C_ASSERT(resource1->getResInfo() == 0);

    Resource* resource2 = signalInfo->mSignalResources->at(1);
    C_ASSERT(resource2->getResCode() == 0x000900dc);
    C_ASSERT(resource2->getValuesCount() == 4);
    C_ASSERT((*resource2->mResValue.values)[0] == 1);
    C_ASSERT((*resource2->mResValue.values)[1] == -1);
    C_ASSERT((*resource2->mResValue.values)[2] == 50);
    C_ASSERT((*resource2->mResValue.values)[3] == 512);
    C_ASSERT(resource2->getResInfo() == 0);
}

static void TestSignalParsingSignalsMerged3() {
    SignalInfo* signalInfo = SignalRegistry::getInstance()->getSignalConfigById(0x801e00ab);

    C_ASSERT(signalInfo != nullptr);
    C_ASSERT(signalInfo->mSignalID == 0x00ab);
    C_ASSERT(signalInfo->mSignalCategory == 0x1e);
    C_ASSERT(strcmp((const char*)signalInfo->mSignalName.data(), "CUSTOM_SIGNAL_1") == 0);
    C_ASSERT(signalInfo->mIsEnabled == true);
    C_ASSERT(signalInfo->mTimeout == 6700);

    C_ASSERT(signalInfo->mTargetsDisabled == nullptr);
    C_ASSERT(signalInfo->mTargetsEnabled != nullptr);
    C_ASSERT(signalInfo->mPermissions != nullptr);
    C_ASSERT(signalInfo->mDerivatives != nullptr);
    C_ASSERT(signalInfo->mSignalResources != nullptr);

    C_ASSERT(signalInfo->mTargetsEnabled->size() == 2);
    C_ASSERT(signalInfo->mPermissions->size() == 1);
    C_ASSERT(signalInfo->mDerivatives->size() == 1);
    C_ASSERT(signalInfo->mSignalResources->size() == 2);

    // Note the target values are converted to LowerCase before inserting into the map
    C_ASSERT(signalInfo->mTargetsEnabled->count("target1") == 1);
    C_ASSERT(signalInfo->mTargetsEnabled->count("target2") == 1);

    C_ASSERT(signalInfo->mPermissions->at(0) == PERMISSION_THIRD_PARTY);

    C_ASSERT(strcmp((const char*)signalInfo->mDerivatives->at(0).data(), "derivative-device1") == 0);

    Resource* resource1 = signalInfo->mSignalResources->at(0);
    C_ASSERT(resource1->getResCode() == 0x80f10000);
    C_ASSERT(resource1->getValuesCount() == 1);
    C_ASSERT(resource1->mResValue.value == 665);
    C_ASSERT(resource1->getResInfo() == 0x0a00f000);

    Resource* resource2 = signalInfo->mSignalResources->at(1);
    C_ASSERT(resource2->getResCode() == 0x800100d0);
    C_ASSERT(resource2->getValuesCount() == 2);
    C_ASSERT((*resource2->mResValue.values)[0] == 679);
    C_ASSERT((*resource2->mResValue.values)[1] == 812);
    C_ASSERT(resource2->getResInfo() == 0x00100112);
}

static void TestSignalParsingSignalsMerged4() {
    SignalInfo* signalInfo = SignalRegistry::getInstance()->getSignalConfigById(0x00080000);

    C_ASSERT(signalInfo == nullptr);
}

static void TestSignalParsingSignalsMerged5() {
    SignalInfo* signalInfo = SignalRegistry::getInstance()->getSignalConfigById(0x80080000);

    C_ASSERT(signalInfo != nullptr);
    C_ASSERT(signalInfo->mSignalID == 0x0000);
    C_ASSERT(signalInfo->mSignalCategory == 0x08);
    C_ASSERT(strcmp((const char*)signalInfo->mSignalName.data(), "CAMERA_OPEN_CUSTOM") == 0);
    C_ASSERT(signalInfo->mIsEnabled == true);
    C_ASSERT(signalInfo->mTimeout == 1);

    C_ASSERT(signalInfo->mTargetsDisabled != nullptr);
    C_ASSERT(signalInfo->mTargetsEnabled == nullptr);
    C_ASSERT(signalInfo->mPermissions != nullptr);
    C_ASSERT(signalInfo->mDerivatives == nullptr);
    C_ASSERT(signalInfo->mSignalResources != nullptr);

    C_ASSERT(signalInfo->mTargetsDisabled->size() == 2);
    C_ASSERT(signalInfo->mPermissions->size() == 1);
    C_ASSERT(signalInfo->mSignalResources->size() == 2);

    // Note the target values are converted to LowerCase before inserting into the map
    C_ASSERT(signalInfo->mTargetsDisabled->count("target1") == 1);
    C_ASSERT(signalInfo->mTargetsDisabled->count("target2") == 1);

    C_ASSERT(signalInfo->mPermissions->at(0) == PERMISSION_SYSTEM);

    Resource* resource1 = signalInfo->mSignalResources->at(0);
    C_ASSERT(resource1->getResCode() == 0x80d9aa00);
    C_ASSERT(resource1->getValuesCount() == 2);
    C_ASSERT((*resource1->mResValue.values)[0] == 1);
    C_ASSERT((*resource1->mResValue.values)[1] == 556);
    C_ASSERT(resource1->getResInfo() == 0);

    Resource* resource2 = signalInfo->mSignalResources->at(1);
    C_ASSERT(resource2->getResCode() == 0x80c6500f);
    C_ASSERT(resource2->getValuesCount() == 3);
    C_ASSERT((*resource2->mResValue.values)[0] == 1);
    C_ASSERT((*resource2->mResValue.values)[1] == 900);
    C_ASSERT((*resource2->mResValue.values)[2] == 965);
    C_ASSERT(resource2->getResInfo() == 0);
}

int32_t main() {
    std::cout<<"Running Test Suite: [Signal Parsing Tests]\n"<<std::endl;

    Init();
    RUN_TEST(TestSignalParsingSanity)
    RUN_TEST(TestSignalParsingSignalsParsed)
    RUN_TEST(TestSignalParsingSignalsMerged1)
    RUN_TEST(TestSignalParsingSignalsMerged2)
    RUN_TEST(TestSignalParsingSignalsMerged3)
    RUN_TEST(TestSignalParsingSignalsMerged4)
    RUN_TEST(TestSignalParsingSignalsMerged5)

    std::cout<<"\nAll Tests from the suite: [Signal Parsing Test], executed successfully"<<std::endl;
    return 0;
}
