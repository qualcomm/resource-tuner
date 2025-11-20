// Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
// SPDX-License-Identifier: BSD-3-Clause-Clear

#include "Common.h"
#include "TestUtils.h"
#include "MemoryPool.h"
#include "Request.h"
#include "Signal.h"

// Request Cleanup Tests
static void TestResourceStructCoreClusterSettingAndExtraction() {
    Resource* resource = new Resource;
    C_ASSERT(resource != nullptr);

    resource->setCoreValue(2);
    resource->setClusterValue(1);

    C_ASSERT(resource->getCoreValue() == 2);
    C_ASSERT(resource->getClusterValue() == 1);
    C_ASSERT(resource->getResCode() == (uint32_t)((1 << 16) | (1 << 0)));

    delete resource;
}

static void TestResourceStructOps1() {
    int32_t properties = -1;
    properties = SET_REQUEST_PRIORITY(properties, REQ_PRIORITY_HIGH);
    C_ASSERT(properties == -1);
}

static void TestResourceStructOps2() {
    int32_t properties = 0;
    properties = SET_REQUEST_PRIORITY(properties, 44);
    C_ASSERT(properties == -1);

    properties = 0;
    properties = SET_REQUEST_PRIORITY(properties, -3);
    C_ASSERT(properties == -1);
}

static void TestResourceStructOps3() {
    int32_t properties = 0;
    properties = SET_REQUEST_PRIORITY(properties, REQ_PRIORITY_HIGH);
    int8_t priority = EXTRACT_REQUEST_PRIORITY(properties);
    C_ASSERT(priority == REQ_PRIORITY_HIGH);

    properties = 0;
    properties = SET_REQUEST_PRIORITY(properties, REQ_PRIORITY_LOW);
    priority = EXTRACT_REQUEST_PRIORITY(properties);
    C_ASSERT(priority == REQ_PRIORITY_LOW);
}

static void TestResourceStructOps4() {
    int32_t properties = 0;
    properties = ADD_ALLOWED_MODE(properties, MODE_RESUME);
    int8_t allowedModes = EXTRACT_ALLOWED_MODES(properties);
    C_ASSERT(allowedModes == MODE_RESUME);

    properties = 0;
    properties = ADD_ALLOWED_MODE(properties, MODE_RESUME);
    properties = ADD_ALLOWED_MODE(properties, MODE_DOZE);
    allowedModes = EXTRACT_ALLOWED_MODES(properties);
    C_ASSERT(allowedModes == (MODE_RESUME | MODE_DOZE));
}

static void TestResourceStructOps5() {
    int32_t properties = 0;
    properties = ADD_ALLOWED_MODE(properties, 87);
    C_ASSERT(properties == -1);
}

static void TestResourceStructOps6() {
    int32_t properties = 0;
    properties = ADD_ALLOWED_MODE(properties, MODE_RESUME);
    properties = ADD_ALLOWED_MODE(properties, MODE_SUSPEND);
    int8_t allowedModes = EXTRACT_ALLOWED_MODES(properties);
    C_ASSERT(allowedModes == (MODE_RESUME | MODE_SUSPEND));
}

static void TestResourceStructOps7() {
    int32_t properties = 0;
    properties = ADD_ALLOWED_MODE(properties, MODE_RESUME);
    properties = ADD_ALLOWED_MODE(properties, -1);
    int8_t allowedModes = EXTRACT_ALLOWED_MODES(properties);
    C_ASSERT(allowedModes == -1);
}

static void TestResourceStructOps8() {
    int32_t properties = 0;
    properties = SET_REQUEST_PRIORITY(properties, REQ_PRIORITY_LOW);
    properties = ADD_ALLOWED_MODE(properties, MODE_RESUME);
    properties = ADD_ALLOWED_MODE(properties, MODE_SUSPEND);

    int8_t priority = EXTRACT_REQUEST_PRIORITY(properties);
    int8_t allowedModes = EXTRACT_ALLOWED_MODES(properties);

    C_ASSERT(priority == REQ_PRIORITY_LOW);
    C_ASSERT(allowedModes == (MODE_RESUME | MODE_SUSPEND));
}

static void TestResourceStructOps9() {
    int32_t resInfo = 0;
    resInfo = SET_RESOURCE_MPAM_VALUE(resInfo, 30);
    int8_t mpamValue = EXTRACT_RESOURCE_MPAM_VALUE(resInfo);
    C_ASSERT(mpamValue == 30);
}

static void TestHandleGeneration() {
    for(int32_t i = 1; i <= 2e7; i++) {
        int64_t handle = AuxRoutines::generateUniqueHandle();
        C_ASSERT(handle == i);
    }
}

static void TestAuxRoutineFileExists() {
    int8_t fileExists = AuxRoutines::fileExists("AuxParserTest.yaml");
    C_ASSERT(fileExists == false);

    fileExists = AuxRoutines::fileExists("/etc/resource-tuner/custom/NetworkConfig.yaml");
    C_ASSERT(fileExists == false);

    fileExists = AuxRoutines::fileExists(ResourceTunerSettings::mCommonResourceFilePath);
    C_ASSERT(fileExists == true);

    fileExists = AuxRoutines::fileExists(ResourceTunerSettings::mCommonPropertiesFilePath);
    C_ASSERT(fileExists == true);

    fileExists = AuxRoutines::fileExists("");
    C_ASSERT(fileExists == false);
}

static void TestRequestModeAddition() {
    Request request;
    request.setProperties(0);
    request.addProcessingMode(MODE_RESUME);
    C_ASSERT(request.getProcessingModes() == MODE_RESUME);

    request.setProperties(0);
    request.addProcessingMode(MODE_RESUME);
    request.addProcessingMode(MODE_SUSPEND);
    request.addProcessingMode(MODE_DOZE);
    C_ASSERT(request.getProcessingModes() == (MODE_RESUME | MODE_SUSPEND | MODE_DOZE));
}

static void RunTests()  {
    std::cout<<"Running Test Suite: [MiscTests]\n"<<std::endl;

    RUN_TEST(TestResourceStructCoreClusterSettingAndExtraction);
    RUN_TEST(TestResourceStructOps1);
    RUN_TEST(TestResourceStructOps2);
    RUN_TEST(TestResourceStructOps3);
    RUN_TEST(TestResourceStructOps4);
    RUN_TEST(TestResourceStructOps5);
    RUN_TEST(TestResourceStructOps6);
    RUN_TEST(TestResourceStructOps7);
    RUN_TEST(TestResourceStructOps8);
    RUN_TEST(TestHandleGeneration);
    RUN_TEST(TestAuxRoutineFileExists);
    RUN_TEST(TestRequestModeAddition);

    std::cout<<"\nAll Tests from the suite: [MiscTests], executed successfully"<<std::endl;
}

REGISTER_TEST(RunTests);
