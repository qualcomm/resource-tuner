// Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
// SPDX-License-Identifier: BSD-3-Clause-Clear


// misc_tests.cpp

#include "AuxRoutines.h"
#include "UrmSettings.h"
#include "TestUtils.h"
#include "Common.h"
#include "MemoryPool.h"
#include "Request.h"
#include "Signal.h"
#include "TestAggregator.h"
#define MTEST_NO_MAIN
#include "../framework/mini.hpp"



using namespace mtest;

// Suite: MiscTests

MT_TEST(MiscTests, ResourceCoreClusterSettingAndExtraction, "component-serial") {
    Resource resource;

    resource.setCoreValue(2);
    resource.setClusterValue(1);

    MT_REQUIRE_EQ(ctx, resource.getCoreValue(), 2);
    MT_REQUIRE_EQ(ctx, resource.getClusterValue(), 1);
}

MT_TEST(MiscTests, ResourceStructOps1, "unit") {
    int32_t properties = -1;
    properties = SET_REQUEST_PRIORITY(properties, REQ_PRIORITY_HIGH);
    MT_REQUIRE_EQ(ctx, properties, -1);
}

MT_TEST(MiscTests, ResourceStructOps2, "unit") {
    int32_t properties = 0;
    properties = SET_REQUEST_PRIORITY(properties, 44);
    MT_REQUIRE_EQ(ctx, properties, -1);

    properties = 0;
    properties = SET_REQUEST_PRIORITY(properties, -3);
    MT_REQUIRE_EQ(ctx, properties, -1);
}

MT_TEST(MiscTests, ResourceStructOps3, "unit") {
    int32_t properties = 0;
    properties = SET_REQUEST_PRIORITY(properties, REQ_PRIORITY_HIGH);
    int8_t priority = EXTRACT_REQUEST_PRIORITY(properties);
    MT_REQUIRE_EQ(ctx, priority, REQ_PRIORITY_HIGH);

    properties = 0;
    properties = SET_REQUEST_PRIORITY(properties, REQ_PRIORITY_LOW);
    priority = EXTRACT_REQUEST_PRIORITY(properties);
    MT_REQUIRE_EQ(ctx, priority, REQ_PRIORITY_LOW);
}

MT_TEST(MiscTests, ResourceStructOps4, "component-serial") {
    int32_t properties = 0;
    properties = ADD_ALLOWED_MODE(properties, MODE_RESUME);
    int8_t allowedModes = EXTRACT_ALLOWED_MODES(properties);
    MT_REQUIRE_EQ(ctx, allowedModes, MODE_RESUME);

    properties = 0;
    properties = ADD_ALLOWED_MODE(properties, MODE_RESUME);
    properties = ADD_ALLOWED_MODE(properties, MODE_DOZE);
    allowedModes = EXTRACT_ALLOWED_MODES(properties);
    MT_REQUIRE_EQ(ctx, allowedModes, (MODE_RESUME | MODE_DOZE));
}

MT_TEST(MiscTests, ResourceStructOps5, "component-serial") {
    int32_t properties = 0;
    properties = ADD_ALLOWED_MODE(properties, 87);
    MT_REQUIRE_EQ(ctx, properties, -1);
}

MT_TEST(MiscTests, ResourceStructOps6, "component-serial") {
    int32_t properties = 0;
    properties = ADD_ALLOWED_MODE(properties, MODE_RESUME);
    properties = ADD_ALLOWED_MODE(properties, MODE_SUSPEND);
    int8_t allowedModes = EXTRACT_ALLOWED_MODES(properties);
    MT_REQUIRE_EQ(ctx, allowedModes, (MODE_RESUME | MODE_SUSPEND));
}

MT_TEST(MiscTests, ResourceStructOps7, "component-serial") {
    int32_t properties = 0;
    properties = ADD_ALLOWED_MODE(properties, MODE_RESUME);
    properties = ADD_ALLOWED_MODE(properties, -1);
    int8_t allowedModes = EXTRACT_ALLOWED_MODES(properties);
    MT_REQUIRE_EQ(ctx, allowedModes, -1);
}

MT_TEST(MiscTests, ResourceStructOps8, "component-serial") {
    int32_t properties = 0;
    properties = SET_REQUEST_PRIORITY(properties, REQ_PRIORITY_LOW);
    properties = ADD_ALLOWED_MODE(properties, MODE_RESUME);
    properties = ADD_ALLOWED_MODE(properties, MODE_SUSPEND);

    int8_t priority = EXTRACT_REQUEST_PRIORITY(properties);
    int8_t allowedModes = EXTRACT_ALLOWED_MODES(properties);

    MT_REQUIRE_EQ(ctx, priority, REQ_PRIORITY_LOW);
    MT_REQUIRE_EQ(ctx, allowedModes, (MODE_RESUME | MODE_SUSPEND));
}

MT_TEST(MiscTests, ResourceMpamOps, "component-serial") {
    int32_t resInfo = 0;
    resInfo = SET_RESOURCE_MPAM_VALUE(resInfo, 30);
    int8_t mpamValue = EXTRACT_RESOURCE_MPAM_VALUE(resInfo);
    MT_REQUIRE_EQ(ctx, mpamValue, 30);
}

// NOTE: This test can be very slow (2e7 iterations). Consider running with --threads
// or gate it under a tag filter or environment if needed.
MT_TEST(MiscTests, HandleGeneration, "component-serial") {
    for (int32_t i = 1; i <= static_cast<int32_t>(2e7); ++i) {
        int64_t handle = AuxRoutines::generateUniqueHandle();
        MT_REQUIRE_EQ(ctx, handle, static_cast<int64_t>(i));
    }
}

MT_TEST(MiscTests, AuxRoutineFileExists, "component-serial") {
    int8_t fileExists = AuxRoutines::fileExists("AuxParserTest.yaml");
    MT_REQUIRE_EQ(ctx, fileExists, false);

    fileExists = AuxRoutines::fileExists("/etc/urm/tests/configs/NetworkConfig.yaml");
    MT_REQUIRE_EQ(ctx, fileExists, false);

    fileExists = AuxRoutines::fileExists(UrmSettings::mCommonResourceFilePath.c_str());
    MT_REQUIRE_EQ(ctx, fileExists, true);

    fileExists = AuxRoutines::fileExists(UrmSettings::mCommonPropertiesFilePath.c_str());
    MT_REQUIRE_EQ(ctx, fileExists, true);

    fileExists = AuxRoutines::fileExists("");
    MT_REQUIRE_EQ(ctx, fileExists, false);
}


// MiscTests.cpp
#include "TestUtils.h" // where MakeAlloc<T>() lives

MT_TEST(MiscTests, RequestModeAddition, "component-serial") {
    // Initialize the pool used by Request::Request() before creating any Request
    MakeAlloc<DLManager>(/*capacity*/ 32);   // pick a sensible capacity for your suite

    // Case 1
    Request request1;
    request1.setProperties(0);
    request1.addProcessingMode(MODE_RESUME);
    MT_REQUIRE_EQ(ctx,
        static_cast<uint32_t>(request1.getProcessingModes()),
        static_cast<uint32_t>(MODE_RESUME));

    // Case 2 – use a fresh Request since there is no clearProcessingModes()
    Request request2;
    request2.setProperties(0);
    request2.addProcessingMode(MODE_RESUME);
    request2.addProcessingMode(MODE_SUSPEND);
    request2.addProcessingMode(MODE_DOZE);
    MT_REQUIRE_EQ(ctx,
        static_cast<uint32_t>(request2.getProcessingModes()),
        static_cast<uint32_t>(MODE_RESUME) |
        static_cast<uint32_t>(MODE_SUSPEND) |
        static_cast<uint32_t>(MODE_DOZE));
}

