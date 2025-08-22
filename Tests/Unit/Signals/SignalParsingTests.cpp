// Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
// SPDX-License-Identifier: BSD-3-Clause-Clear

#include <thread>
#include <cstdint>
#include <gtest/gtest.h>

#include "ConfigProcessor.h"
#include "SignalRegistry.h"
#include "Extensions.h"
#include "Utils.h"

#define TOTAL_SIGNAL_CONFIGS_COUNT 13

static int8_t firstTest = true;

class SignalParsingTests: public::testing::Test {
protected:
    void SetUp() override {
        if(firstTest) {
            firstTest = false;
            ConfigProcessor configProcessor;

            std::string commonSignals = "../Tests/Configs/testSignalsCommonConfig.yaml";
            std::string targetSpecificSignals = "../Tests/Configs/testSignalsTargetSpecificConfig.yaml";
            std::string customSignals = "../Tests/Configs/testSignalsCustomConfig.yaml";

            if(RC_IS_NOTOK(configProcessor.parseSignalConfigs(commonSignals))) {
                return;
            }

            if(RC_IS_NOTOK(configProcessor.parseSignalConfigs(targetSpecificSignals))) {
                return;
            }

            if(RC_IS_NOTOK(configProcessor.parseSignalConfigs(customSignals, true))) {
                return;
            }
        }
    }
};

TEST_F(SignalParsingTests, TestSignalParsingSanity) {
    ASSERT_NE(SignalRegistry::getInstance(), nullptr);
}

TEST_F(SignalParsingTests, TestSignalParsingSignalsParsed) {
    ASSERT_EQ(SignalRegistry::getInstance()->getSignalsConfigCount(), TOTAL_SIGNAL_CONFIGS_COUNT);
}

TEST_F(SignalParsingTests, TestSignalParsingSignalsMerged1) {
    SignalInfo* signalInfo = SignalRegistry::getInstance()->getSignalConfigById(0x00010000);

    ASSERT_NE(signalInfo, nullptr);
    ASSERT_EQ(signalInfo->mSignalID, 0);
    ASSERT_EQ(signalInfo->mSignalCategory, 1);
    ASSERT_EQ(strcmp((const char*)signalInfo->mSignalName.data(), "OVERRIDE_SIGNAL_1"), 0);
    ASSERT_EQ(signalInfo->mIsEnabled, true);
    ASSERT_EQ(signalInfo->mTimeout, 14500);

    ASSERT_EQ(signalInfo->mTargetsDisabled, nullptr);
    ASSERT_NE(signalInfo->mTargetsEnabled, nullptr);
    ASSERT_NE(signalInfo->mPermissions, nullptr);
    ASSERT_NE(signalInfo->mDerivatives, nullptr);
    ASSERT_NE(signalInfo->mSignalResources, nullptr);

    ASSERT_EQ(signalInfo->mTargetsEnabled->size(), 1);
    ASSERT_EQ(signalInfo->mPermissions->size(), 1);
    ASSERT_EQ(signalInfo->mDerivatives->size(), 1);
    ASSERT_EQ(signalInfo->mSignalResources->size(), 1);

    // Note the target values are converted to LowerCase before inserting into the map
    ASSERT_EQ(signalInfo->mTargetsEnabled->count("qti-cli-test"), 1);

    ASSERT_EQ(signalInfo->mPermissions->at(0), PERMISSION_SYSTEM);

    ASSERT_EQ(strcmp((const char*)signalInfo->mDerivatives->at(0).data(), "test-derivative"), 0);

    Resource* resource1 = signalInfo->mSignalResources->at(0);
    ASSERT_EQ(resource1->getOpCode(), 0x80dbaaa0);
    ASSERT_EQ(resource1->getValuesCount(), 1);
    ASSERT_EQ(resource1->mConfigValue.singleValue, 887);
    ASSERT_EQ(resource1->getOperationalInfo(), 0x000776aa);
}

TEST_F(SignalParsingTests, TestSignalParsingSignalsMerged2) {
    SignalInfo* signalInfo = SignalRegistry::getInstance()->getSignalConfigById(0x00080002);

    ASSERT_NE(signalInfo, nullptr);
    ASSERT_EQ(signalInfo->mSignalID, 2);
    ASSERT_EQ(signalInfo->mSignalCategory, 0x08);
    ASSERT_EQ(strcmp((const char*)signalInfo->mSignalName.data(), "MOVE_TID_CUSTOMIZABLE"), 0);
    ASSERT_EQ(signalInfo->mIsEnabled, true);
    ASSERT_EQ(signalInfo->mTimeout, 5500);

    ASSERT_EQ(signalInfo->mTargetsDisabled, nullptr);
    ASSERT_EQ(signalInfo->mTargetsEnabled, nullptr);
    ASSERT_NE(signalInfo->mPermissions, nullptr);
    ASSERT_EQ(signalInfo->mDerivatives, nullptr);
    ASSERT_NE(signalInfo->mSignalResources, nullptr);

    ASSERT_EQ(signalInfo->mPermissions->size(), 1);
    ASSERT_EQ(signalInfo->mSignalResources->size(), 2);

    ASSERT_EQ(signalInfo->mPermissions->at(0), PERMISSION_THIRD_PARTY);

    Resource* resource1 = signalInfo->mSignalResources->at(0);
    ASSERT_EQ(resource1->getOpCode(), 0x000900aa);
    ASSERT_EQ(resource1->getValuesCount(), 3);
    ASSERT_EQ((*resource1->mConfigValue.valueArray)[0], 1);
    ASSERT_EQ((*resource1->mConfigValue.valueArray)[1], -1);
    ASSERT_EQ((*resource1->mConfigValue.valueArray)[2], 68);
    ASSERT_EQ(resource1->getOperationalInfo(), 0);

    Resource* resource2 = signalInfo->mSignalResources->at(1);
    ASSERT_EQ(resource2->getOpCode(), 0x000900dc);
    ASSERT_EQ(resource2->getValuesCount(), 4);
    ASSERT_EQ((*resource2->mConfigValue.valueArray)[0], 1);
    ASSERT_EQ((*resource2->mConfigValue.valueArray)[1], -1);
    ASSERT_EQ((*resource2->mConfigValue.valueArray)[2], 50);
    ASSERT_EQ((*resource2->mConfigValue.valueArray)[3], 512);
    ASSERT_EQ(resource2->getOperationalInfo(), 0);
}

TEST_F(SignalParsingTests, TestSignalParsingSignalsMerged3) {
    SignalInfo* signalInfo = SignalRegistry::getInstance()->getSignalConfigById(0x801e00ab);

    ASSERT_NE(signalInfo, nullptr);
    ASSERT_EQ(signalInfo->mSignalID, 0x00ab);
    ASSERT_EQ(signalInfo->mSignalCategory, 0x1e);
    ASSERT_EQ(strcmp((const char*)signalInfo->mSignalName.data(), "CUSTOM_SIGNAL_1"), 0);
    ASSERT_EQ(signalInfo->mIsEnabled, true);
    ASSERT_EQ(signalInfo->mTimeout, 6700);

    ASSERT_EQ(signalInfo->mTargetsDisabled, nullptr);
    ASSERT_NE(signalInfo->mTargetsEnabled, nullptr);
    ASSERT_NE(signalInfo->mPermissions, nullptr);
    ASSERT_NE(signalInfo->mDerivatives, nullptr);
    ASSERT_NE(signalInfo->mSignalResources, nullptr);

    ASSERT_EQ(signalInfo->mTargetsEnabled->size(), 2);
    ASSERT_EQ(signalInfo->mPermissions->size(), 1);
    ASSERT_EQ(signalInfo->mDerivatives->size(), 1);
    ASSERT_EQ(signalInfo->mSignalResources->size(), 2);

    // Note the target values are converted to LowerCase before inserting into the map
    ASSERT_EQ(signalInfo->mTargetsEnabled->count("target1"), 1);
    ASSERT_EQ(signalInfo->mTargetsEnabled->count("target2"), 1);

    ASSERT_EQ(signalInfo->mPermissions->at(0), PERMISSION_THIRD_PARTY);

    ASSERT_EQ(strcmp((const char*)signalInfo->mDerivatives->at(0).data(), "derivative-device1"), 0);

    Resource* resource1 = signalInfo->mSignalResources->at(0);
    ASSERT_EQ(resource1->getOpCode(), 0x80f10000);
    ASSERT_EQ(resource1->getValuesCount(), 1);
    ASSERT_EQ(resource1->mConfigValue.singleValue, 665);
    ASSERT_EQ(resource1->getOperationalInfo(), 0x0a00f000);

    Resource* resource2 = signalInfo->mSignalResources->at(1);
    ASSERT_EQ(resource2->getOpCode(), 0x800100d0);
    ASSERT_EQ(resource2->getValuesCount(), 2);
    ASSERT_EQ((*resource2->mConfigValue.valueArray)[0], 679);
    ASSERT_EQ((*resource2->mConfigValue.valueArray)[1], 812);
    ASSERT_EQ(resource2->getOperationalInfo(), 0x00100112);
}

TEST_F(SignalParsingTests, TestSignalParsingSignalsMerged4) {
    SignalInfo* signalInfo = SignalRegistry::getInstance()->getSignalConfigById(0x00080000);

    ASSERT_EQ(signalInfo, nullptr);
}

TEST_F(SignalParsingTests, TestSignalParsingSignalsMerged5) {
    SignalInfo* signalInfo = SignalRegistry::getInstance()->getSignalConfigById(0x80080000);

    ASSERT_NE(signalInfo, nullptr);
    ASSERT_EQ(signalInfo->mSignalID, 0x0000);
    ASSERT_EQ(signalInfo->mSignalCategory, 0x08);
    ASSERT_EQ(strcmp((const char*)signalInfo->mSignalName.data(), "CAMERA_OPEN_CUSTOM"), 0);
    ASSERT_EQ(signalInfo->mIsEnabled, true);
    ASSERT_EQ(signalInfo->mTimeout, 1);

    ASSERT_NE(signalInfo->mTargetsDisabled, nullptr);
    ASSERT_EQ(signalInfo->mTargetsEnabled, nullptr);
    ASSERT_NE(signalInfo->mPermissions, nullptr);
    ASSERT_EQ(signalInfo->mDerivatives, nullptr);
    ASSERT_NE(signalInfo->mSignalResources, nullptr);

    ASSERT_EQ(signalInfo->mTargetsDisabled->size(), 2);
    ASSERT_EQ(signalInfo->mPermissions->size(), 1);
    ASSERT_EQ(signalInfo->mSignalResources->size(), 2);

    // Note the target values are converted to LowerCase before inserting into the map
    ASSERT_EQ(signalInfo->mTargetsDisabled->count("target1"), 1);
    ASSERT_EQ(signalInfo->mTargetsDisabled->count("target2"), 1);

    ASSERT_EQ(signalInfo->mPermissions->at(0), PERMISSION_SYSTEM);

    Resource* resource1 = signalInfo->mSignalResources->at(0);
    ASSERT_EQ(resource1->getOpCode(), 0x80d9aa00);
    ASSERT_EQ(resource1->getValuesCount(), 2);
    ASSERT_EQ((*resource1->mConfigValue.valueArray)[0], 1);
    ASSERT_EQ((*resource1->mConfigValue.valueArray)[1], 556);
    ASSERT_EQ(resource1->getOperationalInfo(), 0);

    Resource* resource2 = signalInfo->mSignalResources->at(1);
    ASSERT_EQ(resource2->getOpCode(), 0x80c6500f);
    ASSERT_EQ(resource2->getValuesCount(), 3);
    ASSERT_EQ((*resource2->mConfigValue.valueArray)[0], 1);
    ASSERT_EQ((*resource2->mConfigValue.valueArray)[1], 900);
    ASSERT_EQ((*resource2->mConfigValue.valueArray)[2], 965);
    ASSERT_EQ(resource2->getOperationalInfo(), 0);
}
