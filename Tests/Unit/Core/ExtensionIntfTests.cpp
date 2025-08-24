// Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
// SPDX-License-Identifier: BSD-3-Clause-Clear

#include <gtest/gtest.h>

#include "ResourceRegistry.h"
#include "ConfigProcessor.h"
#include "Extensions.h"

RESTUNE_REGISTER_CONFIG(RESOURCE_CONFIG, "/etc/resource-tuner/tests/Configs/ResourcesConfig.yaml")
RESTUNE_REGISTER_CONFIG(PROPERTIES_CONFIG, "/etc/resource-tuner/tests/Configs/PropertiesConfig.yaml")
RESTUNE_REGISTER_CONFIG(SIGNALS_CONFIG, "/etc/resource-tuner/tests/Configs/SignalsConfig.yaml")
RESTUNE_REGISTER_CONFIG(TARGET_CONFIG, "/etc/resource-tuner/tests/Configs/TargetConfig.yaml")

static int8_t firstTest = true;
static int8_t funcCalled = false;

void customApplier(void* context) {
    funcCalled = true;
}

void customTear(void* context) {
    funcCalled = true;
}

RESTUNE_REGISTER_APPLIER_CB(0x80010000, customApplier)
RESTUNE_REGISTER_TEAR_CB(0x80010001, customTear)

class ExtensionIntfTests: public::testing::Test {
protected:
    void SetUp() override {
        if(firstTest) {
            firstTest = false;
            ConfigProcessor configProcessor;

            configProcessor.parseResourceConfigs(Extensions::getResourceConfigFilePath(), true);
            ResourceRegistry::getInstance()->pluginModifications();
        }
    }
};


TEST_F(ExtensionIntfTests, TestExtensionIntfModifiedResourceConfigPath) {
    ASSERT_EQ(
        Extensions::getResourceConfigFilePath(),
        "/etc/resource-tuner/tests/Configs/ResourcesConfig.yaml"
    );
}

TEST_F(ExtensionIntfTests, TestExtensionIntfModifiedPropertiesConfigPath) {
    ASSERT_EQ(
        Extensions::getPropertiesConfigFilePath(),
        "/etc/resource-tuner/tests/Configs/PropertiesConfig.yaml"
    );
}

TEST_F(ExtensionIntfTests, TestExtensionIntfModifiedSignalConfigPath) {
    ASSERT_EQ(
        Extensions::getSignalsConfigFilePath(),
        "/etc/resource-tuner/tests/Configs/SignalsConfig.yaml"
    );
}

TEST_F(ExtensionIntfTests, TestExtensionIntfModifiedTargetConfigPath) {
    ASSERT_EQ(
        Extensions::getTargetConfigFilePath(),
        "/etc/resource-tuner/tests/Configs/TargetConfig.yaml"
    );
}

TEST_F(ExtensionIntfTests, TestExtensionIntfCustomResourceApplier) {
    ResourceConfigInfo* info = ResourceRegistry::getInstance()->getResourceById(0x80010000);
    ASSERT_NE(info, nullptr);
    funcCalled = false;
    ASSERT_NE(info->mResourceApplierCallback, nullptr);
    info->mResourceApplierCallback(nullptr);
    ASSERT_EQ(funcCalled, true);
}

TEST_F(ExtensionIntfTests, TestExtensionIntfCustomResourceTear) {
    ResourceConfigInfo* info = ResourceRegistry::getInstance()->getResourceById(0x80010001);
    ASSERT_NE(info, nullptr);
    funcCalled = false;
    ASSERT_NE(info->mResourceTearCallback, nullptr);
    info->mResourceTearCallback(nullptr);
    ASSERT_EQ(funcCalled, true);
}
