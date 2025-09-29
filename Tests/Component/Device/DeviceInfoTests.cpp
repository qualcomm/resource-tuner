// Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
// SPDX-License-Identifier: BSD-3-Clause-Clear

#include <cstdint>

#include "TestUtils.h"
#include "TestBaseline.h"
#include "TargetRegistry.h"
#include "Extensions.h"
#include "Utils.h"

static TestBaseline baseline;

static void Init() {
    baseline.fetchBaseline();
    TargetRegistry::getInstance()->readTargetInfo();
}

static void TestDeviceClusterCount() {
    int32_t clusterCount = ResourceTunerSettings::targetConfigs.mTotalClusterCount;
    int32_t expectedClusterCount = baseline.getExpectedClusterCount();

    std::cout<<"Determined Cluster Count: "<<clusterCount<<std::endl;
    std::cout<<"Expected Cluster Count: "<<expectedClusterCount<<std::endl;

    if(expectedClusterCount == -1) {
        LOG_SKIP("Baseline Could not be fetched for the Target, skipping");
        return;
    }

    C_ASSERT(clusterCount == expectedClusterCount);
}

static void TestDeviceCoreCount() {
    int32_t coreCount = ResourceTunerSettings::targetConfigs.mTotalCoreCount;
    int32_t expectedCoreCount = baseline.getExpectedCoreCount();

    std::cout<<"Determined Core Count: "<<coreCount<<std::endl;
    std::cout<<"Expected Core Count: "<<expectedCoreCount<<std::endl;

    if(expectedCoreCount == -1) {
        LOG_SKIP("Baseline Could not be fetched for the Target, skipping");
        return;
    }

    C_ASSERT(coreCount == expectedCoreCount);
}

static void TestDeviceClusterMappingInfo() {
    int32_t phy_for_lgc_0 = baseline.getExpectedPhysicalCluster(0);
    if(phy_for_lgc_0 != -1) {
        // Lgc0 does exist
        int32_t phyID = TargetRegistry::getInstance()->getPhysicalClusterId(0);
        std::cout<<"Lgc 0 maps to physical ID: "<<phyID<<", Expected: ["<<phy_for_lgc_0<<"]"<<std::endl;
        C_ASSERT(phy_for_lgc_0 == phyID);
    }

    int32_t phy_for_lgc_1 = baseline.getExpectedPhysicalCluster(1);
    if(phy_for_lgc_1 != -1) {
        // Lgc1 does exist
        int32_t phyID = TargetRegistry::getInstance()->getPhysicalClusterId(1);
        std::cout<<"Lgc 1 maps to physical ID: "<<phyID<<", Expected: ["<<phy_for_lgc_1<<"]"<<std::endl;
        C_ASSERT(phy_for_lgc_1 == phyID);
    }

    int32_t phy_for_lgc_2 = baseline.getExpectedPhysicalCluster(2);
    if(phy_for_lgc_2 != -1) {
        // Lgc2 does exist
        int32_t phyID = TargetRegistry::getInstance()->getPhysicalClusterId(2);
        std::cout<<"Lgc 2 maps to physical ID: "<<phyID<<", Expected: ["<<phy_for_lgc_2<<"]"<<std::endl;
        C_ASSERT(phy_for_lgc_2 == phyID);
    }

    int32_t phy_for_lgc_3 = baseline.getExpectedPhysicalCluster(3);
    if(phy_for_lgc_3 != -1) {
        // Lgc3 does exist
        int32_t phyID = TargetRegistry::getInstance()->getPhysicalClusterId(3);
        std::cout<<"Lgc 3 maps to physical ID: "<<phyID<<", Expected: ["<<phy_for_lgc_3<<"]"<<std::endl;
        C_ASSERT(phy_for_lgc_3 == phyID);
    }
}

int32_t main() {
    std::cout<<"Running Test Suite: [DeviceInfoTests]\n"<<std::endl;

    Init();
    RUN_TEST(TestDeviceClusterCount);
    RUN_TEST(TestDeviceCoreCount);
    RUN_TEST(TestDeviceClusterMappingInfo);

    std::cout<<"\nAll Tests from the suite: [DeviceInfoTests], executed successfully"<<std::endl;
    return 0;
}
