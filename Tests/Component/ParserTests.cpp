// Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
// SPDX-License-Identifier: BSD-3-Clause-Clear

#include "ErrCodes.h"
#include "TestUtils.h"
#include "ConfigProcessor.h"
#include "ResourceRegistry.h"
#include "SignalRegistry.h"
#include "Extensions.h"
#include "SignalConfigProcessor.h"
#include "Utils.h"
#include "ServerInternal.h"
#include "PropertiesRegistry.h"

namespace ResourceParsingTests {
    std::string __testGroupName = "ResourceParsingTests";

    static ErrCode parsingStatus = RC_SUCCESS;
    static void Init() {
        ConfigProcessor configProcessor;
        parsingStatus = configProcessor.parseResourceConfigs("/etc/resource-tuner/custom/ResourcesConfig.yaml", true);
    }

    static void TestResourceConfigProcessorYAMLDataIntegrity1() {
        C_ASSERT(ResourceRegistry::getInstance() != nullptr);
        C_ASSERT(parsingStatus == RC_SUCCESS);
    }

    static void TestResourceConfigProcessorYAMLDataIntegrity3_1() {
        ResConfInfo* resourceConfigInfo = ResourceRegistry::getInstance()->getResConf(0x80ff0000);

        C_ASSERT(resourceConfigInfo != nullptr);
        C_ASSERT(resourceConfigInfo->mResourceResType == 0xff);
        C_ASSERT(resourceConfigInfo->mResourceResID == 0);
        C_ASSERT(strcmp((const char*)resourceConfigInfo->mResourceName.data(), "TEST_RESOURCE_1") == 0);
        C_ASSERT(strcmp((const char*)resourceConfigInfo->mResourcePath.data(), "/etc/resource-tuner/tests/Configs/ResourceSysFsNodes/sched_util_clamp_min.txt") == 0);
        C_ASSERT(resourceConfigInfo->mHighThreshold == 1024);
        C_ASSERT(resourceConfigInfo->mLowThreshold == 0);
        C_ASSERT(resourceConfigInfo->mPolicy == HIGHER_BETTER);
        C_ASSERT(resourceConfigInfo->mPermissions == PERMISSION_THIRD_PARTY);
        C_ASSERT(resourceConfigInfo->mModes == (MODE_RESUME | MODE_DOZE));
        C_ASSERT(resourceConfigInfo->mApplyType == ResourceApplyType::APPLY_GLOBAL);
    }

    static void TestResourceConfigProcessorYAMLDataIntegrity3_2() {
        ResConfInfo* resourceConfigInfo = ResourceRegistry::getInstance()->getResConf(0x80ff0001);

        C_ASSERT(resourceConfigInfo != nullptr);
        C_ASSERT(resourceConfigInfo->mResourceResType == 0xff);
        C_ASSERT(resourceConfigInfo->mResourceResID == 1);
        C_ASSERT(strcmp((const char*)resourceConfigInfo->mResourceName.data(), "TEST_RESOURCE_2") == 0);
        C_ASSERT(strcmp((const char*)resourceConfigInfo->mResourcePath.data(), "/etc/resource-tuner/tests/Configs/ResourceSysFsNodes/sched_util_clamp_max.txt") == 0);
        C_ASSERT(resourceConfigInfo->mHighThreshold == 1024);
        C_ASSERT(resourceConfigInfo->mLowThreshold == 512);
        C_ASSERT(resourceConfigInfo->mPolicy == HIGHER_BETTER);
        C_ASSERT(resourceConfigInfo->mPermissions == PERMISSION_THIRD_PARTY);
        C_ASSERT(resourceConfigInfo->mModes == (MODE_RESUME | MODE_DOZE));
        C_ASSERT(resourceConfigInfo->mApplyType == ResourceApplyType::APPLY_GLOBAL);
    }

    static void TestResourceConfigProcessorYAMLDataIntegrity3_3() {
        ResConfInfo* resourceConfigInfo = ResourceRegistry::getInstance()->getResConf(0x80ff0005);

        C_ASSERT(resourceConfigInfo != nullptr);
        C_ASSERT(resourceConfigInfo->mResourceResType == 0xff);
        C_ASSERT(resourceConfigInfo->mResourceResID == 5);
        C_ASSERT(strcmp((const char*)resourceConfigInfo->mResourceName.data(), "TEST_RESOURCE_6") == 0);
        C_ASSERT(strcmp((const char*)resourceConfigInfo->mResourcePath.data(), "/etc/resource-tuner/tests/Configs/ResourceSysFsNodes/target_test_resource2.txt") == 0);
        C_ASSERT(resourceConfigInfo->mHighThreshold == 6500);
        C_ASSERT(resourceConfigInfo->mLowThreshold == 50);
        C_ASSERT(resourceConfigInfo->mPolicy == HIGHER_BETTER);
        C_ASSERT(resourceConfigInfo->mPermissions == PERMISSION_THIRD_PARTY);
        C_ASSERT(resourceConfigInfo->mModes == MODE_RESUME);
        C_ASSERT(resourceConfigInfo->mApplyType == ResourceApplyType::APPLY_CORE);
    }

    static void RunTestGroup() {
        std::cout<<"\nRunning tests from the Group: "<<__testGroupName<<std::endl;

        Init();
        RUN_TEST(TestResourceConfigProcessorYAMLDataIntegrity1);
        RUN_TEST(TestResourceConfigProcessorYAMLDataIntegrity3_1);
        RUN_TEST(TestResourceConfigProcessorYAMLDataIntegrity3_2);
        RUN_TEST(TestResourceConfigProcessorYAMLDataIntegrity3_3);

        std::cout<<"\n\nAll tests from the Group: "<<__testGroupName<<", Ran Successfully"<<std::endl;
    }
}

namespace SignalParsingTests {
    std::string __testGroupName = "Signal Application Checks";

    static ErrCode parsingStatus = RC_SUCCESS;
    static void Init() {
        SignalConfigProcessor configProcessor;
        parsingStatus = configProcessor.parseSignalConfigs("/etc/resource-tuner/custom/SignalsConfig.yaml");
    }

    static void TestSignalConfigProcessorYAMLDataIntegrity1() {
        C_ASSERT(SignalRegistry::getInstance() != nullptr);
        C_ASSERT(parsingStatus == RC_SUCCESS);
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
        C_ASSERT(resource1->getValueAt(0) == 700);
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
        C_ASSERT(resource1->getValueAt(0) == 814);
        C_ASSERT(resource1->getResInfo() == 0);

        Resource* resource2 = signalInfo->mSignalResources->at(1);
        C_ASSERT(resource2->getResCode() == 15);
        C_ASSERT(resource2->getValuesCount() == 2);
        C_ASSERT(resource2->getValueAt(0) == 23);
        C_ASSERT(resource2->getValueAt(1) == 90);
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
        C_ASSERT(resource1->getValueAt(0) == 300);
        C_ASSERT(resource1->getValueAt(1) == 400);
        C_ASSERT(resource1->getResInfo() == 0);

        Resource* resource2 = signalInfo->mSignalResources->at(1);
        C_ASSERT(resource2->getResCode() == 241);
        C_ASSERT(resource2->getValuesCount() == 3);
        C_ASSERT(resource2->getValueAt(0) == 12);
        C_ASSERT(resource2->getValueAt(1) == 45);
        C_ASSERT(resource2->getValueAt(2) == 67);
        C_ASSERT(resource2->getResInfo() == 1024);

        Resource* resource3 = signalInfo->mSignalResources->at(2);
        C_ASSERT(resource3->getResCode() == 43981);
        C_ASSERT(resource3->getValuesCount() == 1);
        C_ASSERT(resource3->getValueAt(0) == 5);
        C_ASSERT(resource3->getResInfo() == 32);

        Resource* resource4 = signalInfo->mSignalResources->at(3);
        C_ASSERT(resource4->getResCode() == 59917);
        C_ASSERT(resource4->getValuesCount() == 1);
        C_ASSERT(resource4->getValueAt(0) == 87);
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
        C_ASSERT(resource1->getValueAt(0) == -1);
        C_ASSERT(resource1->getValueAt(1) == -1);
        C_ASSERT(resource1->getValueAt(2) == 68);
        C_ASSERT(resource1->getResInfo() == 0);

        Resource* resource2 = signalInfo->mSignalResources->at(1);
        C_ASSERT(resource2->getResCode() == 0x000900dc);
        C_ASSERT(resource2->getValuesCount() == 4);
        C_ASSERT(resource2->getValueAt(0) == -1);
        C_ASSERT(resource2->getValueAt(1) == -1);
        C_ASSERT(resource2->getValueAt(2) == 50);
        C_ASSERT(resource2->getValueAt(3) == 512);
        C_ASSERT(resource2->getResInfo() == 0);
    }

    static void RunTestGroup() {
        std::cout<<"\nRunning tests from the Group: "<<__testGroupName<<std::endl;

        Init();
        RUN_TEST(TestSignalConfigProcessorYAMLDataIntegrity1);
        RUN_TEST(TestSignalConfigProcessorYAMLDataIntegrity3_1);
        RUN_TEST(TestSignalConfigProcessorYAMLDataIntegrity3_2);
        RUN_TEST(TestSignalConfigProcessorYAMLDataIntegrity3_3);
        RUN_TEST(TestSignalConfigProcessorYAMLDataIntegrity3_4);

        std::cout<<"\n\nAll tests from the Group: "<<__testGroupName<<", Ran Successfully"<<std::endl;
    }
}

namespace InitConfigParsingTests {
    std::string __testGroupName = "InitConfigParsingTests";

    static ErrCode parsingStatus = RC_SUCCESS;
    static void Init() {
        ConfigProcessor configProcessor;
        parsingStatus = configProcessor.parseInitConfigs("/etc/resource-tuner/custom/InitConfig.yaml");
    }

    static void TestInitConfigProcessorYAMLDataIntegrity1() {
        C_ASSERT(TargetRegistry::getInstance() != nullptr);\
        C_ASSERT(parsingStatus == RC_SUCCESS);
    }

    static void TestInitConfigProcessorYAMLDataIntegrity2() {
        std::cout<<"Count of Cgroups created: "<<TargetRegistry::getInstance()->getCreatedCGroupsCount()<<std::endl;
        C_ASSERT(TargetRegistry::getInstance()->getCreatedCGroupsCount() == 3);
    }

    // Note don't rely on order here, since internally CGroup mapping data is stored
    // as an unordered_map.
    static void TestInitConfigProcessorYAMLDataIntegrity3() {
        std::vector<std::string> cGroupNames;
        TargetRegistry::getInstance()->getCGroupNames(cGroupNames);
        std::vector<std::string> expectedNames = {"camera-cgroup", "audio-cgroup", "video-cgroup"};

        C_ASSERT(cGroupNames.size() == 3);

        std::unordered_set<std::string> expectedNamesSet;
        for(int32_t i = 0; i < cGroupNames.size(); i++) {
            expectedNamesSet.insert(cGroupNames[i]);
        }

        for(int32_t i = 0; i < expectedNames.size(); i++) {
            C_ASSERT(expectedNamesSet.find(expectedNames[i]) != expectedNamesSet.end());
        }
    }

    static void TestInitConfigProcessorYAMLDataIntegrity4() {
        CGroupConfigInfo* cameraConfig = TargetRegistry::getInstance()->getCGroupConfig(0);
        C_ASSERT(cameraConfig != nullptr);
        C_ASSERT(cameraConfig->mCgroupName == "camera-cgroup");
        C_ASSERT(cameraConfig->mIsThreaded == false);

        CGroupConfigInfo* videoConfig = TargetRegistry::getInstance()->getCGroupConfig(2);
        C_ASSERT(videoConfig != nullptr);
        C_ASSERT(videoConfig->mCgroupName == "video-cgroup");
        C_ASSERT(videoConfig->mIsThreaded == true);
    }


    static void TestInitConfigProcessorYAMLDataIntegrity5() {
        C_ASSERT(TargetRegistry::getInstance()->getCreatedMpamGroupsCount() == 3);
    }

    // Note don't rely on order here, since internally CGroup mapping data is stored
    // as an unordered_map.
    static void TestInitConfigProcessorYAMLDataIntegrity6() {
        std::vector<std::string> mpamGroupNames;
        TargetRegistry::getInstance()->getMpamGroupNames(mpamGroupNames);
        std::vector<std::string> expectedNames = {"camera-mpam-group", "audio-mpam-group", "video-mpam-group"};

        C_ASSERT(mpamGroupNames.size() == 3);

        std::unordered_set<std::string> expectedNamesSet;
        for(int32_t i = 0; i < mpamGroupNames.size(); i++) {
            expectedNamesSet.insert(mpamGroupNames[i]);
        }

        for(int32_t i = 0; i < expectedNames.size(); i++) {
            C_ASSERT(expectedNamesSet.find(expectedNames[i]) != expectedNamesSet.end());
        }
    }

    static void TestInitConfigProcessorYAMLDataIntegrity7() {
        MpamGroupConfigInfo* cameraConfig = TargetRegistry::getInstance()->getMpamGroupConfig(0);
        C_ASSERT(cameraConfig != nullptr);
        C_ASSERT(cameraConfig->mMpamGroupName == "camera-mpam-group");
        C_ASSERT(cameraConfig->mMpamGroupInfoID == 0);
        C_ASSERT(cameraConfig->mPriority == 0);

        MpamGroupConfigInfo* videoConfig = TargetRegistry::getInstance()->getMpamGroupConfig(2);
        C_ASSERT(videoConfig != nullptr);
        C_ASSERT(videoConfig->mMpamGroupName == "video-mpam-group");
        C_ASSERT(videoConfig->mMpamGroupInfoID == 2);
        C_ASSERT(videoConfig->mPriority == 2);
    }

    static void RunTestGroup() {
        std::cout<<"\nRunning tests from the Group: "<<__testGroupName<<std::endl;

        Init();
        RUN_TEST(TestInitConfigProcessorYAMLDataIntegrity1);
        RUN_TEST(TestInitConfigProcessorYAMLDataIntegrity2);
        RUN_TEST(TestInitConfigProcessorYAMLDataIntegrity3);
        RUN_TEST(TestInitConfigProcessorYAMLDataIntegrity4);
        RUN_TEST(TestInitConfigProcessorYAMLDataIntegrity5);
        RUN_TEST(TestInitConfigProcessorYAMLDataIntegrity6);
        RUN_TEST(TestInitConfigProcessorYAMLDataIntegrity7);

        std::cout<<"\n\nAll tests from the Group: "<<__testGroupName<<", Ran Successfully"<<std::endl;
    }
}

namespace PropertyParsingTests {
    std::string __testGroupName = "PropertyParsingTests";

    static ErrCode parsingStatus = RC_SUCCESS;
    static void Init() {
        ConfigProcessor configProcessor;
        parsingStatus = configProcessor.parsePropertiesConfigs("/etc/resource-tuner/custom/PropertiesConfig.yaml");
    }

    static void TestSysConfigProcessorYAMLDataIntegrity1() {
        C_ASSERT(PropertiesRegistry::getInstance() != nullptr);
        C_ASSERT(parsingStatus == RC_SUCCESS);
    }

    static void TestSysConfigGetPropSimpleRetrieval1() {
        std::string resultBuffer;

        int8_t propFound = submitPropGetRequest("test.debug.enabled", resultBuffer, "false");

        C_ASSERT(propFound == true);
        C_ASSERT(strcmp(resultBuffer.c_str(), "true") == 0);
    }

    static void TestSysConfigGetPropSimpleRetrieval2() {
        std::string resultBuffer;

        int8_t propFound = submitPropGetRequest("test.current.worker_thread.count", resultBuffer, "false");

        C_ASSERT(propFound == true);
        C_ASSERT(strcmp(resultBuffer.c_str(), "125") == 0);
    }

    static void TestSysConfigGetPropSimpleRetrievalInvalidProperty() {
        std::string resultBuffer;

        int8_t propFound = submitPropGetRequest("test.historic.worker_thread.count", resultBuffer, "5");

        C_ASSERT(propFound == false);
        C_ASSERT(strcmp(resultBuffer.c_str(), "5") == 0);
    }

    static void TestSysConfigGetPropConcurrentRetrieval() {
        std::thread th1([&]{
            std::string resultBuffer;
            int8_t propFound = submitPropGetRequest("test.current.worker_thread.count", resultBuffer, "false");

            C_ASSERT(propFound == true);
            C_ASSERT(strcmp(resultBuffer.c_str(), "125") == 0);
        });

        std::thread th2([&]{
            std::string resultBuffer;
            int8_t propFound = submitPropGetRequest("test.debug.enabled", resultBuffer, "false");

            C_ASSERT(propFound == true);
            C_ASSERT(strcmp(resultBuffer.c_str(), "true") == 0);
        });

        std::thread th3([&]{
            std::string resultBuffer;
            int8_t propFound = submitPropGetRequest("test.doc.build.mode.enabled", resultBuffer, "false");

            C_ASSERT(propFound == true);
            C_ASSERT(strcmp(resultBuffer.c_str(), "false") == 0);
        });

        th1.join();
        th2.join();
        th3.join();
    }

    static void RunTestGroup() {
        std::cout<<"\nRunning tests from the Group: "<<__testGroupName<<std::endl;

        Init();
        RUN_TEST(TestSysConfigProcessorYAMLDataIntegrity1);
        RUN_TEST(TestSysConfigGetPropSimpleRetrieval1);
        RUN_TEST(TestSysConfigGetPropSimpleRetrieval2);
        RUN_TEST(TestSysConfigGetPropSimpleRetrievalInvalidProperty);
        RUN_TEST(TestSysConfigGetPropConcurrentRetrieval);

        std::cout<<"\n\nAll tests from the Group: "<<__testGroupName<<", Ran Successfully"<<std::endl;
    }
}

namespace TargetConfigProcessorTests {
    std::string __testGroupName = "TargetConfigProcessorTests";

    static ErrCode parsingStatus = RC_SUCCESS;
    static void Init() {
        ResourceTunerSettings::targetConfigs.targetName = "TestDevice";
        ConfigProcessor configProcessor;
        parsingStatus = configProcessor.parseTargetConfigs("/etc/resource-tuner/custom/TargetConfigDup.yaml");
    }

    static void TestTargetConfigProcessorYAMLDataIntegrity1() {
        C_ASSERT(TargetRegistry::getInstance() != nullptr);
        C_ASSERT(parsingStatus == RC_SUCCESS);
    }

    static void TestTargetConfigProcessorYAMLDataIntegrity2() {
        std::cout<<"Determined Cluster Count = "<<ResourceTunerSettings::targetConfigs.mTotalClusterCount<<std::endl;
        C_ASSERT(ResourceTunerSettings::targetConfigs.mTotalClusterCount == 4);
    }

    static void TestTargetConfigProcessorYAMLDataIntegrity3() {
        C_ASSERT(TargetRegistry::getInstance()->getPhysicalClusterId(0) == 4);
        C_ASSERT(TargetRegistry::getInstance()->getPhysicalClusterId(1) == 0);
        C_ASSERT(TargetRegistry::getInstance()->getPhysicalClusterId(2) == 9);
        C_ASSERT(TargetRegistry::getInstance()->getPhysicalClusterId(3) == 7);
    }

    static void TestTargetConfigProcessorYAMLDataIntegrity4() {
        // Distribution of physical clusters
        // 1:0 => 0, 1, 2, 3
        // 0:4 => 4, 5, 6
        // 3:7 => 7, 8
        // 2:9 => 9
        C_ASSERT(TargetRegistry::getInstance()->getPhysicalCoreId(1, 3) == 2);

        C_ASSERT(TargetRegistry::getInstance()->getPhysicalCoreId(0, 2) == 5);

        C_ASSERT(TargetRegistry::getInstance()->getPhysicalCoreId(3, 1) == 7);

        C_ASSERT(TargetRegistry::getInstance()->getPhysicalCoreId(2, 1) == 9);
    }

    static void RunTestGroup() {
        std::cout<<"\nRunning tests from the Group: "<<__testGroupName<<std::endl;

        Init();
        RUN_TEST(TestTargetConfigProcessorYAMLDataIntegrity1);
        RUN_TEST(TestTargetConfigProcessorYAMLDataIntegrity2);
        RUN_TEST(TestTargetConfigProcessorYAMLDataIntegrity3);
        RUN_TEST(TestTargetConfigProcessorYAMLDataIntegrity4);

        std::cout<<"\n\nAll tests from the Group: "<<__testGroupName<<", Ran Successfully"<<std::endl;
    }
}

namespace ExtFeaturesParsingTests {
    std::string __testGroupName = "ExtFeaturesParsingTests";

    static ErrCode parsingStatus = RC_SUCCESS;
    static void Init() {
        SignalConfigProcessor configProcessor;
        parsingStatus = configProcessor.parseExtFeaturesConfigs("/etc/resource-tuner/custom/ExtFeaturesConfig.yaml");
    }

    static void TestExtFeatConfigProcessorYAMLDataIntegrity1() {
        C_ASSERT(ExtFeaturesRegistry::getInstance() != nullptr);
        C_ASSERT(parsingStatus == RC_SUCCESS);
    }

    static void TestExtFeatConfigProcessorYAMLDataIntegrity3() {
        ExtFeatureInfo* feature =
            ExtFeaturesRegistry::getInstance()->getExtFeatureConfigById(0x00000001);

        C_ASSERT(feature != nullptr);
        C_ASSERT(feature->mFeatureId == 0x00000001);
        C_ASSERT(feature->mFeatureName == "FEAT-1");
        C_ASSERT(feature->mFeatureLib == "/usr/lib/libtesttuner.so");

        C_ASSERT(feature->mSignalsSubscribedTo != nullptr);
        C_ASSERT(feature->mSignalsSubscribedTo->size() == 2);
        C_ASSERT((*feature->mSignalsSubscribedTo)[0] == 0x000dbbca);
        C_ASSERT((*feature->mSignalsSubscribedTo)[1] == 0x000a00ff);
    }

    static void TestExtFeatConfigProcessorYAMLDataIntegrity4() {
        ExtFeatureInfo* feature =
            ExtFeaturesRegistry::getInstance()->getExtFeatureConfigById(0x00000002);

        C_ASSERT(feature != nullptr);
        C_ASSERT(feature->mFeatureId == 0x00000002);
        C_ASSERT(feature->mFeatureName == "FEAT-2");
        C_ASSERT(feature->mFeatureLib == "/usr/lib/libpropagate.so");

        C_ASSERT(feature->mSignalsSubscribedTo != nullptr);
        C_ASSERT(feature->mSignalsSubscribedTo->size() == 2);
        C_ASSERT((*feature->mSignalsSubscribedTo)[0] == 0x80a105ea);
        C_ASSERT((*feature->mSignalsSubscribedTo)[1] == 0x800ccca5);
    }

    static void RunTestGroup() {
        std::cout<<"\nRunning tests from the Group: "<<__testGroupName<<std::endl;

        Init();
        RUN_TEST(TestExtFeatConfigProcessorYAMLDataIntegrity1);
        RUN_TEST(TestExtFeatConfigProcessorYAMLDataIntegrity3);
        RUN_TEST(TestExtFeatConfigProcessorYAMLDataIntegrity4);

        std::cout<<"\n\nAll tests from the Group: "<<__testGroupName<<", Ran Successfully"<<std::endl;
    }
}

namespace ResourceParsingTestsAddOn {
    std::string __testGroupName = "ResourceParsingTestsAddOn";

    static ErrCode parsingStatus = RC_SUCCESS;
    static void Init() {
        ConfigProcessor configProcessor;
        std::string additionalResources = "/etc/resource-tuner/custom/ResourcesConfigAddOn.yaml";

        if(RC_IS_OK(parsingStatus)) {
            parsingStatus = configProcessor.parseResourceConfigs(additionalResources, true);
        }
    }

    static void TestResourceParsingSanity() {
        C_ASSERT(ResourceRegistry::getInstance() != nullptr);
        C_ASSERT(parsingStatus == RC_SUCCESS);
    }

    static void TestResourceParsingResourcesMerged1() {
        ResConfInfo* resourceConfigInfo = ResourceRegistry::getInstance()->getResConf(0x80ff000b);

        C_ASSERT(resourceConfigInfo != nullptr);
        C_ASSERT(resourceConfigInfo->mResourceResType == 0xff);
        C_ASSERT(resourceConfigInfo->mResourceResID == 0x000b);
        C_ASSERT(strcmp((const char*)resourceConfigInfo->mResourceName.data(), "OVERRIDE_RESOURCE_1") == 0);
        C_ASSERT(strcmp((const char*)resourceConfigInfo->mResourcePath.data(), "/etc/resouce-tuner/tests/Configs/pathB/overwrite") == 0);
        C_ASSERT(resourceConfigInfo->mHighThreshold == 220);
        C_ASSERT(resourceConfigInfo->mLowThreshold == 150);
        C_ASSERT(resourceConfigInfo->mPolicy == LOWER_BETTER);
        C_ASSERT(resourceConfigInfo->mPermissions == PERMISSION_SYSTEM);
        C_ASSERT(resourceConfigInfo->mModes == (MODE_RESUME | MODE_DOZE));
        C_ASSERT(resourceConfigInfo->mApplyType == ResourceApplyType::APPLY_CORE);
    }

    static void TestResourceParsingResourcesMerged2() {
        ResConfInfo* resourceConfigInfo = ResourceRegistry::getInstance()->getResConf(0x80ff1000);

        C_ASSERT(resourceConfigInfo != nullptr);
        C_ASSERT(resourceConfigInfo->mResourceResType == 0xff);
        C_ASSERT(resourceConfigInfo->mResourceResID == 0x1000);
        C_ASSERT(strcmp((const char*)resourceConfigInfo->mResourceName.data(), "CUSTOM_SCALING_FREQ") == 0);
        C_ASSERT(strcmp((const char*)resourceConfigInfo->mResourcePath.data(), "/usr/local/customfreq/node") == 0);
        C_ASSERT(resourceConfigInfo->mHighThreshold == 90);
        C_ASSERT(resourceConfigInfo->mLowThreshold == 80);
        C_ASSERT(resourceConfigInfo->mPolicy == LAZY_APPLY);
        C_ASSERT(resourceConfigInfo->mPermissions == PERMISSION_THIRD_PARTY);
        C_ASSERT(resourceConfigInfo->mModes == MODE_DOZE);
        C_ASSERT(resourceConfigInfo->mApplyType == ResourceApplyType::APPLY_CORE);
    }

    static void TestResourceParsingResourcesMerged3() {
        ResConfInfo* resourceConfigInfo = ResourceRegistry::getInstance()->getResConf(0x80ff1001);

        C_ASSERT(resourceConfigInfo != nullptr);
        C_ASSERT(resourceConfigInfo->mResourceResType == 0xff);
        C_ASSERT(resourceConfigInfo->mResourceResID == 0x1001);
        C_ASSERT(strcmp((const char*)resourceConfigInfo->mResourceName.data(), "CUSTOM_RESOURCE_ADDED_BY_BU") == 0);
        C_ASSERT(strcmp((const char*)resourceConfigInfo->mResourcePath.data(), "/some/bu/specific/node/path/customized_to_usecase") == 0);
        C_ASSERT(resourceConfigInfo->mHighThreshold == 512);
        C_ASSERT(resourceConfigInfo->mLowThreshold == 128);
        C_ASSERT(resourceConfigInfo->mPolicy == LOWER_BETTER);
        C_ASSERT(resourceConfigInfo->mPermissions == PERMISSION_SYSTEM);
        C_ASSERT(resourceConfigInfo->mModes == MODE_RESUME);
        C_ASSERT(resourceConfigInfo->mApplyType == ResourceApplyType::APPLY_GLOBAL);
    }

    static void TestResourceParsingResourcesMerged4() {
        ResConfInfo* resourceConfigInfo = ResourceRegistry::getInstance()->getResConf(0x80ff000c);

        C_ASSERT(resourceConfigInfo != nullptr);
        C_ASSERT(resourceConfigInfo->mResourceResType == 0xff);
        C_ASSERT(resourceConfigInfo->mResourceResID == 0x000c);
        C_ASSERT(strcmp((const char*)resourceConfigInfo->mResourceName.data(), "OVERRIDE_RESOURCE_2") == 0);
        C_ASSERT(strcmp((const char*)resourceConfigInfo->mResourcePath.data(), "/proc/kernel/tid/kernel/uclamp.tid.sched/rt") == 0);
        C_ASSERT(resourceConfigInfo->mHighThreshold == 100022);
        C_ASSERT(resourceConfigInfo->mLowThreshold == 87755);
        C_ASSERT(resourceConfigInfo->mPolicy == INSTANT_APPLY);
        C_ASSERT(resourceConfigInfo->mPermissions == PERMISSION_THIRD_PARTY);
        C_ASSERT(resourceConfigInfo->mModes == (MODE_RESUME | MODE_DOZE));
        C_ASSERT(resourceConfigInfo->mApplyType == ResourceApplyType::APPLY_GLOBAL);
    }

    static void TestResourceParsingResourcesDefaultValuesCheck() {
        ResConfInfo* resourceConfigInfo = ResourceRegistry::getInstance()->getResConf(0x00ff0009);

        C_ASSERT(resourceConfigInfo != nullptr);
        C_ASSERT(resourceConfigInfo->mResourceResType == 0xff);
        C_ASSERT(resourceConfigInfo->mResourceResID == 0x0009);
        C_ASSERT(strcmp((const char*)resourceConfigInfo->mResourceName.data(), "DEFAULT_VALUES_TEST") == 0);
        C_ASSERT(strcmp((const char*)resourceConfigInfo->mResourcePath.data(), "") == 0);
        C_ASSERT(resourceConfigInfo->mHighThreshold == -1);
        C_ASSERT(resourceConfigInfo->mLowThreshold == -1);
        C_ASSERT(resourceConfigInfo->mPolicy == LAZY_APPLY);
        C_ASSERT(resourceConfigInfo->mPermissions == PERMISSION_THIRD_PARTY);
        C_ASSERT(resourceConfigInfo->mModes == 0);
        C_ASSERT(resourceConfigInfo->mApplyType == ResourceApplyType::APPLY_GLOBAL);
    }

    static void RunTestGroup() {
        std::cout<<"\nRunning tests from the Group: "<<__testGroupName<<std::endl;

        Init();
        RUN_TEST(TestResourceParsingSanity);
        RUN_TEST(TestResourceParsingResourcesMerged1);
        RUN_TEST(TestResourceParsingResourcesMerged2);
        RUN_TEST(TestResourceParsingResourcesMerged3);
        RUN_TEST(TestResourceParsingResourcesMerged4);
        RUN_TEST(TestResourceParsingResourcesDefaultValuesCheck);

        std::cout<<"\n\nAll tests from the Group: "<<__testGroupName<<", Ran Successfully"<<std::endl;
    }
}

namespace SignalParsingTestsAddOn {
    std::string __testGroupName = "SignalParsingTests";

    static ErrCode parsingStatus = RC_SUCCESS;
    static void Init() {
        SignalConfigProcessor configProcessor;

        std::string signalsClassA = "/etc/resource-tuner/custom/SignalsConfig.yaml";
        std::string signalsClassB = "/etc/resource-tuner/custom/SignalsConfigAddOn.yaml";

        parsingStatus = configProcessor.parseSignalConfigs(signalsClassA);
        if(RC_IS_OK(parsingStatus)) {
            parsingStatus = configProcessor.parseSignalConfigs(signalsClassB, true);
        }
    }

    static void TestSignalParsingSanity() {
        C_ASSERT(SignalRegistry::getInstance() != nullptr);
        C_ASSERT(parsingStatus == RC_SUCCESS);
    }

    static void TestSignalParsingSignalsMerged1() {
        SignalInfo* signalInfo = SignalRegistry::getInstance()->getSignalConfigById(0x80deaadd);

        C_ASSERT(signalInfo != nullptr);
        C_ASSERT(signalInfo->mSignalID == 0xaadd);
        C_ASSERT(signalInfo->mSignalCategory == 0xde);
        C_ASSERT(strcmp((const char*)signalInfo->mSignalName.data(), "OVERRIDE_SIGNAL_1") == 0);
        C_ASSERT(signalInfo->mTimeout == 14500);

        C_ASSERT(signalInfo->mPermissions != nullptr);
        C_ASSERT(signalInfo->mDerivatives != nullptr);
        C_ASSERT(signalInfo->mSignalResources != nullptr);

        C_ASSERT(signalInfo->mPermissions->size() == 1);
        C_ASSERT(signalInfo->mDerivatives->size() == 1);
        C_ASSERT(signalInfo->mSignalResources->size() == 1);

        C_ASSERT(signalInfo->mPermissions->at(0) == PERMISSION_SYSTEM);

        C_ASSERT(strcmp((const char*)signalInfo->mDerivatives->at(0).data(), "test-derivative") == 0);

        Resource* resource1 = signalInfo->mSignalResources->at(0);
        C_ASSERT(resource1->getResCode() == 0x80dbaaa0);
        C_ASSERT(resource1->getValuesCount() == 1);
        C_ASSERT(resource1->getValueAt(0) == 887);
        C_ASSERT(resource1->getResInfo() == 0x000776aa);
    }

    static void TestSignalParsingSignalsMerged2() {
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
        C_ASSERT(resource1->getValueAt(0) == -1);
        C_ASSERT(resource1->getValueAt(1) == -1);
        C_ASSERT(resource1->getValueAt(2) == 68);
        C_ASSERT(resource1->getResInfo() == 0);

        Resource* resource2 = signalInfo->mSignalResources->at(1);
        C_ASSERT(resource2->getResCode() == 0x000900dc);
        C_ASSERT(resource2->getValuesCount() == 4);
        C_ASSERT(resource2->getValueAt(0) == -1);
        C_ASSERT(resource2->getValueAt(1) == -1);
        C_ASSERT(resource2->getValueAt(2) == 50);
        C_ASSERT(resource2->getValueAt(3) == 512);
        C_ASSERT(resource2->getResInfo() == 0);
    }

    static void TestSignalParsingSignalsMerged3() {
        SignalInfo* signalInfo = SignalRegistry::getInstance()->getSignalConfigById(0x801e00ab);

        C_ASSERT(signalInfo != nullptr);
        C_ASSERT(signalInfo->mSignalID == 0x00ab);
        C_ASSERT(signalInfo->mSignalCategory == 0x1e);
        C_ASSERT(strcmp((const char*)signalInfo->mSignalName.data(), "CUSTOM_SIGNAL_1") == 0);
        C_ASSERT(signalInfo->mTimeout == 6700);

        C_ASSERT(signalInfo->mPermissions != nullptr);
        C_ASSERT(signalInfo->mDerivatives != nullptr);
        C_ASSERT(signalInfo->mSignalResources != nullptr);

        C_ASSERT(signalInfo->mPermissions->size() == 1);
        C_ASSERT(signalInfo->mDerivatives->size() == 1);
        C_ASSERT(signalInfo->mSignalResources->size() == 2);

        C_ASSERT(signalInfo->mPermissions->at(0) == PERMISSION_THIRD_PARTY);

        C_ASSERT(strcmp((const char*)signalInfo->mDerivatives->at(0).data(), "derivative-device1") == 0);

        Resource* resource1 = signalInfo->mSignalResources->at(0);
        C_ASSERT(resource1->getResCode() == 0x80f10000);
        C_ASSERT(resource1->getValuesCount() == 1);
        C_ASSERT(resource1->getValueAt(0) == 665);
        C_ASSERT(resource1->getResInfo() == 0x0a00f000);

        Resource* resource2 = signalInfo->mSignalResources->at(1);
        C_ASSERT(resource2->getResCode() == 0x800100d0);
        C_ASSERT(resource2->getValuesCount() == 2);
        C_ASSERT(resource2->getValueAt(0) == 679);
        C_ASSERT(resource2->getValueAt(1) == 812);
        C_ASSERT(resource2->getResInfo() == 0x00100112);
    }

    static void TestSignalParsingSignalsMerged4() {
        SignalInfo* signalInfo = SignalRegistry::getInstance()->getSignalConfigById(0x00080000);

        C_ASSERT(signalInfo == nullptr);
    }

    static void TestSignalParsingSignalsMerged5() {
        SignalInfo* signalInfo = SignalRegistry::getInstance()->getSignalConfigById(0x80ceffcf);

        C_ASSERT(signalInfo != nullptr);
        C_ASSERT(signalInfo->mSignalID == 0xffcf);
        C_ASSERT(signalInfo->mSignalCategory == 0xce);
        C_ASSERT(strcmp((const char*)signalInfo->mSignalName.data(), "CAMERA_OPEN_CUSTOM") == 0);
        C_ASSERT(signalInfo->mTimeout == 1);

        C_ASSERT(signalInfo->mPermissions != nullptr);
        C_ASSERT(signalInfo->mDerivatives == nullptr);
        C_ASSERT(signalInfo->mSignalResources != nullptr);

        C_ASSERT(signalInfo->mPermissions->size() == 1);
        C_ASSERT(signalInfo->mSignalResources->size() == 2);

        C_ASSERT(signalInfo->mPermissions->at(0) == PERMISSION_SYSTEM);

        Resource* resource1 = signalInfo->mSignalResources->at(0);
        C_ASSERT(resource1->getResCode() == 0x80d9aa00);
        C_ASSERT(resource1->getValuesCount() == 2);
        C_ASSERT(resource1->getValueAt(0) == 1);
        C_ASSERT(resource1->getValueAt(1) == 556);
        C_ASSERT(resource1->getResInfo() == 0);

        Resource* resource2 = signalInfo->mSignalResources->at(1);
        C_ASSERT(resource2->getResCode() == 0x80c6500f);
        C_ASSERT(resource2->getValuesCount() == 3);
        C_ASSERT(resource2->getValueAt(0) == 1);
        C_ASSERT(resource2->getValueAt(1)  == 900);
        C_ASSERT(resource2->getValueAt(2)  == 965);
        C_ASSERT(resource2->getResInfo() == 0);
    }

    static void RunTestGroup() {
        std::cout<<"\nRunning tests from the Group: "<<__testGroupName<<std::endl;

        Init();
        RUN_TEST(TestSignalParsingSanity)
        RUN_TEST(TestSignalParsingSignalsMerged1)
        RUN_TEST(TestSignalParsingSignalsMerged2)
        RUN_TEST(TestSignalParsingSignalsMerged3)
        RUN_TEST(TestSignalParsingSignalsMerged4)
        RUN_TEST(TestSignalParsingSignalsMerged5)

        std::cout<<"\n\nAll tests from the Group: "<<__testGroupName<<", Ran Successfully"<<std::endl;
    }
}

static void RunTests() {
    ResourceParsingTests::RunTestGroup();
    ResourceParsingTestsAddOn::RunTestGroup();

    SignalParsingTests::RunTestGroup();
    SignalParsingTestsAddOn::RunTestGroup();

    InitConfigParsingTests::RunTestGroup();
    PropertyParsingTests::RunTestGroup();
    ExtFeaturesParsingTests::RunTestGroup();
    TargetConfigProcessorTests::RunTestGroup();
}

REGISTER_TEST(RunTests);
