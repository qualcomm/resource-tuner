// Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
// SPDX-License-Identifier: BSD-3-Clause-Clear

#include <thread>
#include <cstdint>
#include <gtest/gtest.h>

#include "ConfigProcessor.h"
#include "SignalRegistry.h"
#include "Extensions.h"
#include "Utils.h"

#define TOTAL_SIGNAL_CONFIGS_COUNT 8

class SignalConfigProcessorTests: public::testing::Test {
protected:
    void SetUp() override {
        static int8_t firstTest = true;
        if(firstTest) {
            firstTest = false;
            ConfigProcessor configProcessor;

            if(RC_IS_NOTOK(configProcessor.parseSignalConfigs("../Tests/Configs/testSignalsTargetSpecificConfig.yaml"))) {
                return;
            }
        }
    }
};

TEST_F(SignalConfigProcessorTests, TestSignalConfigProcessorYAMLDataIntegrity1) {
    ASSERT_NE(SignalRegistry::getInstance(), nullptr);
}

TEST_F(SignalConfigProcessorTests, TestSignalConfigProcessorYAMLDataIntegrity2) {
    ASSERT_EQ(SignalRegistry::getInstance()->getSignalsConfigCount(), TOTAL_SIGNAL_CONFIGS_COUNT);
}

TEST_F(SignalConfigProcessorTests, TestSignalConfigProcessorYAMLDataIntegrity3_1) {
    SignalInfo* signalInfo = SignalRegistry::getInstance()->getSignalConfigById(0x000d0000);

    ASSERT_NE(signalInfo, nullptr);
    ASSERT_EQ(signalInfo->mSignalID, 0);
    ASSERT_EQ(signalInfo->mSignalCategory, 0x0d);
    ASSERT_EQ(strcmp((const char*)signalInfo->mSignalName.data(), "INSTALL"), 0);
    ASSERT_EQ(signalInfo->mIsEnabled, true);
    ASSERT_EQ(signalInfo->mTimeout, 4000);

    ASSERT_NE(signalInfo->mTargetsEnabled, nullptr);
    ASSERT_EQ(signalInfo->mTargetsDisabled, nullptr);
    ASSERT_NE(signalInfo->mPermissions, nullptr);
    ASSERT_NE(signalInfo->mDerivatives, nullptr);
    ASSERT_NE(signalInfo->mSignalResources, nullptr);

    ASSERT_EQ(signalInfo->mTargetsEnabled->size(), 2);
    ASSERT_EQ(signalInfo->mPermissions->size(), 1);
    ASSERT_EQ(signalInfo->mDerivatives->size(), 1);
    ASSERT_EQ(signalInfo->mSignalResources->size(), 1);

    // Note the target values are converted to LowerCase before inserting into the map
    ASSERT_EQ(signalInfo->mTargetsEnabled->count("sun"), 1);
    ASSERT_EQ(signalInfo->mTargetsEnabled->count("moon"), 1);

    ASSERT_EQ(signalInfo->mPermissions->at(0), PERMISSION_THIRD_PARTY);

    ASSERT_EQ(strcmp((const char*)signalInfo->mDerivatives->at(0).data(), "solar"), 0);

    Resource* resource1 = signalInfo->mSignalResources->at(0);
    ASSERT_NE(resource1, nullptr);
    ASSERT_EQ(resource1->getResCode(), 2147549184);
    ASSERT_EQ(resource1->getValuesCount(), 1);
    ASSERT_EQ(resource1->mResValue.value, 700);
    ASSERT_EQ(resource1->getResInfo(), 0);
}

TEST_F(SignalConfigProcessorTests, TestSignalConfigProcessorYAMLDataIntegrity3_2) {
    SignalInfo* signalInfo = SignalRegistry::getInstance()->getSignalConfigById(0x000d0001);

    ASSERT_NE(signalInfo, nullptr);
    ASSERT_EQ(signalInfo->mSignalID, 1);
    ASSERT_EQ(signalInfo->mSignalCategory, 0x0d);
    ASSERT_EQ(strcmp((const char*)signalInfo->mSignalName.data(), "EARLY_WAKEUP"), 0);
    ASSERT_EQ(signalInfo->mIsEnabled, true);
    ASSERT_EQ(signalInfo->mTimeout, 5000);

    ASSERT_NE(signalInfo->mTargetsDisabled, nullptr);
    ASSERT_EQ(signalInfo->mTargetsEnabled, nullptr);
    ASSERT_NE(signalInfo->mPermissions, nullptr);
    ASSERT_NE(signalInfo->mDerivatives, nullptr);
    ASSERT_NE(signalInfo->mSignalResources, nullptr);

    ASSERT_EQ(signalInfo->mTargetsDisabled->size(), 1);
    ASSERT_EQ(signalInfo->mPermissions->size(), 1);
    ASSERT_EQ(signalInfo->mDerivatives->size(), 1);
    ASSERT_EQ(signalInfo->mSignalResources->size(), 2);

    // Note the target values are converted to LowerCase before inserting into the map
    ASSERT_EQ(signalInfo->mTargetsDisabled->count("sun"), 1);

    ASSERT_EQ(signalInfo->mPermissions->at(0), PERMISSION_SYSTEM);

    ASSERT_EQ(strcmp((const char*)signalInfo->mDerivatives->at(0).data(), "derivative_v2"), 0);

    Resource* resource1 = signalInfo->mSignalResources->at(0);
    ASSERT_EQ(resource1->getResCode(), 8);
    ASSERT_EQ(resource1->getValuesCount(), 1);
    ASSERT_EQ(resource1->mResValue.value, 814);
    ASSERT_EQ(resource1->getResInfo(), 0);

    Resource* resource2 = signalInfo->mSignalResources->at(1);
    ASSERT_EQ(resource2->getResCode(), 15);
    ASSERT_EQ(resource2->getValuesCount(), 2);
    ASSERT_EQ((*resource2->mResValue.values)[0], 23);
    ASSERT_EQ((*resource2->mResValue.values)[1], 90);
    ASSERT_EQ(resource2->getResInfo(), 256);
}

TEST_F(SignalConfigProcessorTests, TestSignalConfigProcessorYAMLDataIntegrity3_3) {
    SignalInfo* signalInfo = SignalRegistry::getInstance()->getSignalConfigById(0x000d0003);

    ASSERT_NE(signalInfo, nullptr);
    ASSERT_EQ(signalInfo->mSignalID, 3);
    ASSERT_EQ(signalInfo->mSignalCategory, 0x0d);
    ASSERT_EQ(strcmp((const char*)signalInfo->mSignalName.data(), "SMOOTH_SCROLL"), 0);
    ASSERT_EQ(signalInfo->mIsEnabled, false);
    ASSERT_EQ(signalInfo->mTimeout, 4000);

    ASSERT_NE(signalInfo->mTargetsEnabled, nullptr);
    ASSERT_EQ(signalInfo->mTargetsDisabled, nullptr);
    ASSERT_NE(signalInfo->mPermissions, nullptr);
    ASSERT_NE(signalInfo->mDerivatives, nullptr);
    ASSERT_NE(signalInfo->mSignalResources, nullptr);

    ASSERT_EQ(signalInfo->mTargetsEnabled->size(), 2);
    ASSERT_EQ(signalInfo->mPermissions->size(), 1);
    ASSERT_EQ(signalInfo->mDerivatives->size(), 1);
    ASSERT_EQ(signalInfo->mSignalResources->size(), 4);

    // Note the target values are converted to LowerCase before inserting into the map
    // Verify that the Lower case translation Correctly Happens
    ASSERT_EQ(signalInfo->mTargetsEnabled->count("sun"), 1);
    ASSERT_EQ(signalInfo->mTargetsEnabled->count("qli"), 1);

    ASSERT_EQ(signalInfo->mTargetsEnabled->count("SUN"), 0);
    ASSERT_EQ(signalInfo->mTargetsEnabled->count("QLI"), 0);

    ASSERT_EQ(signalInfo->mPermissions->at(0), PERMISSION_THIRD_PARTY);

    ASSERT_EQ(strcmp((const char*)signalInfo->mDerivatives->at(0).data(), "solar"), 0);

    Resource* resource1 = signalInfo->mSignalResources->at(0);
    ASSERT_EQ(resource1->getResCode(), 8);
    ASSERT_EQ(resource1->getValuesCount(), 2);
    ASSERT_EQ((*resource1->mResValue.values)[0], 300);
    ASSERT_EQ((*resource1->mResValue.values)[1], 400);
    ASSERT_EQ(resource1->getResInfo(), 0);

    Resource* resource2 = signalInfo->mSignalResources->at(1);
    ASSERT_EQ(resource2->getResCode(), 241);
    ASSERT_EQ(resource2->getValuesCount(), 3);
    ASSERT_EQ((*resource2->mResValue.values)[0], 12);
    ASSERT_EQ((*resource2->mResValue.values)[1], 45);
    ASSERT_EQ((*resource2->mResValue.values)[2], 67);
    ASSERT_EQ(resource2->getResInfo(), 1024);

    Resource* resource3 = signalInfo->mSignalResources->at(2);
    ASSERT_EQ(resource3->getResCode(), 43981);
    ASSERT_EQ(resource3->getValuesCount(), 1);
    ASSERT_EQ(resource3->mResValue.value, 5);
    ASSERT_EQ(resource3->getResInfo(), 32);

    Resource* resource4 = signalInfo->mSignalResources->at(3);
    ASSERT_EQ(resource4->getResCode(), 59917);
    ASSERT_EQ(resource4->getValuesCount(), 1);
    ASSERT_EQ(resource4->mResValue.value, 87);
    ASSERT_EQ(resource4->getResInfo(), 512);
}

TEST_F(SignalConfigProcessorTests, TestSignalConfigProcessorYAMLDataIntegrity4) {
    std::vector<SignalInfo*> signalConfigs = SignalRegistry::getInstance()->getSignalConfigs();
    ASSERT_EQ(signalConfigs.size(), TOTAL_SIGNAL_CONFIGS_COUNT);

    std::vector<std::string> signalNames {"INSTALL", "EARLY_WAKEUP", "LIGHTNING_LAUNCHES",
                                          "SMOOTH_SCROLL", "TEST_SIGNAL-1", "TEST_SIGNAL-2",
                                          "OVERRIDE_SIGNAL_1", "MOVE_TID_CUSTOMIZABLE"};

    ASSERT_EQ(signalNames.size(), signalConfigs.size());

    for(int32_t i = 0; i < signalConfigs.size(); i++) {
        ASSERT_EQ(strcmp((const char*)signalConfigs[i]->mSignalName.data(), (const char*)signalNames[i].data()), 0);
    }
}
