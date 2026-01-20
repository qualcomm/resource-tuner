// Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
// SPDX-License-Identifier: BSD-3-Clause-Clear

#include "TestUtils.h"
#include "ResourceRegistry.h"
#include "RestuneParser.h"
#include "Extensions.h"
#include "TestAggregator.h"

RESTUNE_REGISTER_CONFIG(RESOURCE_CONFIG, "/etc/urm/tests/configs/ResourcesConfig.yaml")
RESTUNE_REGISTER_CONFIG(PROPERTIES_CONFIG, "/etc/urm/tests/configs/PropertiesConfig.yaml")
RESTUNE_REGISTER_CONFIG(SIGNALS_CONFIG, "/etc/urm/tests/configs/SignalsConfig.yaml")
RESTUNE_REGISTER_CONFIG(TARGET_CONFIG, "/etc/urm/tests/configs/TargetConfig.yaml")
RESTUNE_REGISTER_CONFIG(INIT_CONFIG, "/etc/urm/tests/configs/InitConfig.yaml")

static int8_t firstTest = true;
static int8_t funcCalled = false;
static int32_t invokeCounter = 0;

static void customApplier1(void* context) {
    funcCalled = true;
}

static void customApplier2(void* context) {
    invokeCounter++;
}

static void customTear1(void* context) {
    funcCalled = true;
}

RESTUNE_REGISTER_APPLIER_CB(0x80ff0000, customApplier1)
RESTUNE_REGISTER_TEAR_CB(0x80ff0001, customTear1)
RESTUNE_REGISTER_APPLIER_CB(0x80ff0002, customApplier2)

static void Init() {
    RestuneParser configProcessor;

    configProcessor.parseResourceConfigs(Extensions::getResourceConfigFilePath(), true);
    ResourceRegistry::getInstance()->pluginModifications();
}

static void TestExtensionIntfModifiedResourceConfigPath() {
    C_ASSERT(Extensions::getResourceConfigFilePath() == "/etc/urm/tests/configs/ResourcesConfig.yaml");
}

static void TestExtensionIntfModifiedPropertiesConfigPath() {
    C_ASSERT(Extensions::getPropertiesConfigFilePath() == "/etc/urm/tests/configs/PropertiesConfig.yaml");
}

static void TestExtensionIntfModifiedSignalConfigPath() {
    C_ASSERT(Extensions::getSignalsConfigFilePath() == "/etc/urm/tests/configs/SignalsConfig.yaml");
}

static void TestExtensionIntfModifiedTargetConfigPath() {
    C_ASSERT(Extensions::getTargetConfigFilePath() == "/etc/urm/tests/configs/TargetConfig.yaml");
}

static void TestExtensionIntfModifiedInitConfigPath() {
    C_ASSERT(Extensions::getInitConfigFilePath() == "/etc/urm/tests/configs/InitConfig.yaml");
}

static void TestExtensionIntfCustomResourceApplier1() {
    ResConfInfo* info = ResourceRegistry::getInstance()->getResConf(0x80ff0000);
    C_ASSERT(info != nullptr);
    funcCalled = false;
    C_ASSERT(info->mResourceApplierCallback != nullptr);
    info->mResourceApplierCallback(nullptr);
    C_ASSERT(funcCalled == true);
}

static void TestExtensionIntfCustomResourceApplier2() {
    ResConfInfo* info = ResourceRegistry::getInstance()->getResConf(0x80ff0002);
    C_ASSERT(info != nullptr);
    C_ASSERT(info->mResourceApplierCallback != nullptr);
    info->mResourceApplierCallback(nullptr);
    C_ASSERT(invokeCounter == 1);
}

static void TestExtensionIntfCustomResourceTear() {
    ResConfInfo* info = ResourceRegistry::getInstance()->getResConf(0x80ff0001);
    C_ASSERT(info != nullptr);
    funcCalled = false;
    C_ASSERT(info->mResourceTearCallback != nullptr);
    info->mResourceTearCallback(nullptr);
    C_ASSERT(funcCalled == true);
}

static void RunTests() {
    std::cout<<"Running Test Suite: [ExtensionIntfTests]\n"<<std::endl;

    Init();
    RUN_TEST(TestExtensionIntfModifiedResourceConfigPath);
    RUN_TEST(TestExtensionIntfModifiedPropertiesConfigPath);
    RUN_TEST(TestExtensionIntfModifiedSignalConfigPath);
    RUN_TEST(TestExtensionIntfModifiedTargetConfigPath);
    RUN_TEST(TestExtensionIntfModifiedInitConfigPath);
    RUN_TEST(TestExtensionIntfCustomResourceApplier1);
    RUN_TEST(TestExtensionIntfCustomResourceApplier2);
    RUN_TEST(TestExtensionIntfCustomResourceTear);

    std::cout<<"\nAll Tests from the suite: [ExtensionIntfTests], executed successfully"<<std::endl;
}

REGISTER_TEST(RunTests);
