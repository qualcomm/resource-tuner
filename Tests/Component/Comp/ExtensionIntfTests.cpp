// Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
// SPDX-License-Identifier: BSD-3-Clause-Clear

#include "TestUtils.h"
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

RESTUNE_REGISTER_APPLIER_CB(0x80ff0000, customApplier)
RESTUNE_REGISTER_TEAR_CB(0x80ff0001, customTear)

static void Init() {
    ConfigProcessor configProcessor;

    configProcessor.parseResourceConfigs(Extensions::getResourceConfigFilePath(), true);
    ResourceRegistry::getInstance()->pluginModifications();
}

static void TestExtensionIntfModifiedResourceConfigPath() {
    C_ASSERT(Extensions::getResourceConfigFilePath() == "/etc/resource-tuner/tests/Configs/ResourcesConfig.yaml");
}

static void TestExtensionIntfModifiedPropertiesConfigPath() {
    C_ASSERT(Extensions::getPropertiesConfigFilePath() == "/etc/resource-tuner/tests/Configs/PropertiesConfig.yaml");
}

static void TestExtensionIntfModifiedSignalConfigPath() {
    C_ASSERT(Extensions::getSignalsConfigFilePath() == "/etc/resource-tuner/tests/Configs/SignalsConfig.yaml");
}

static void TestExtensionIntfModifiedTargetConfigPath() {
    C_ASSERT(Extensions::getTargetConfigFilePath() == "/etc/resource-tuner/tests/Configs/TargetConfig.yaml");
}

static void TestExtensionIntfCustomResourceApplier() {
    ResourceConfigInfo* info = ResourceRegistry::getInstance()->getResourceById(0x80ff0000);
    C_ASSERT(info != nullptr);
    funcCalled = false;
    C_ASSERT(info->mResourceApplierCallback != nullptr);
    info->mResourceApplierCallback(nullptr);
    C_ASSERT(funcCalled == true);
}

static void TestExtensionIntfCustomResourceTear() {
    ResourceConfigInfo* info = ResourceRegistry::getInstance()->getResourceById(0x80ff0001);
    C_ASSERT(info != nullptr);
    funcCalled = false;
    C_ASSERT(info->mResourceTearCallback != nullptr);
    info->mResourceTearCallback(nullptr);
    C_ASSERT(funcCalled == true);
}

int32_t main() {
     std::cout<<"Running Test Suite: [ExtensionIntfTests]\n"<<std::endl;

    Init();
    RUN_TEST(TestExtensionIntfModifiedResourceConfigPath);
    RUN_TEST(TestExtensionIntfModifiedPropertiesConfigPath);
    RUN_TEST(TestExtensionIntfModifiedSignalConfigPath);
    RUN_TEST(TestExtensionIntfModifiedTargetConfigPath);
    RUN_TEST(TestExtensionIntfCustomResourceApplier);
    RUN_TEST(TestExtensionIntfCustomResourceTear);

    std::cout<<"\nAll Tests from the suite: [ExtensionIntfTests], executed successfully"<<std::endl;
}
