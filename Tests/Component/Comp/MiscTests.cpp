// Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
// SPDX-License-Identifier: BSD-3-Clause-Clear

#include <thread>
#include <cstdint>

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

    resource->setResourceID(1);
    resource->setResourceType(1);

    C_ASSERT(resource->getResCode() == (uint32_t)((1 << 16) | (1 << 0)));

    delete resource;
}

static void TestRequestSerializingAndDeserializing() {
    try {
        MakeAlloc<Request> (5);
        MakeAlloc<Resource> (10);
        MakeAlloc<std::vector<Resource*>> (5);

        Request* firstRequest = new (GetBlock<Request>()) Request();
        firstRequest->setRequestType(REQ_RESOURCE_TUNING);
        firstRequest->setClientPID(1003);
        firstRequest->setClientTID(1009);
        firstRequest->setNumResources(1);
        firstRequest->setProperties((1 << 8) | (1 << 0));
        firstRequest->setHandle(15);
        firstRequest->setDuration(5600);

        C_ASSERT(firstRequest->getRequestType() == REQ_RESOURCE_TUNING);
        C_ASSERT(firstRequest->getClientPID() == 1003);
        C_ASSERT(firstRequest->getClientTID() == 1009);
        C_ASSERT(firstRequest->getHandle() == 15);
        C_ASSERT(firstRequest->getResourcesCount() == 1);
        C_ASSERT(firstRequest->getDuration() == 5600);
        C_ASSERT(firstRequest->isBackgroundProcessingEnabled() == 1);
        C_ASSERT(firstRequest->getPriority() == 1);

        std::vector<Resource*>* firstReqResourceList = new (GetBlock<std::vector<Resource*>>())
                                                            std::vector<Resource*>;

        Resource* res1 = (Resource*) GetBlock<Resource>();
        res1->setResCode(65536);
        res1->setNumValues(1);
        res1->mResValue.value = 754;
        firstReqResourceList->push_back(res1);
        firstRequest->setResources(firstReqResourceList);

        Resource* resource = firstRequest->getResourceAt(0);
        C_ASSERT(resource->getResCode() == 65536);
        C_ASSERT(resource->getValuesCount() == 1);
        C_ASSERT(resource->mResValue.value == 754);

        char buf[1024];

        // Serializing the Request
        ErrCode rc = firstRequest->serialize(buf);
        C_ASSERT(rc == RC_SUCCESS);

        // Deserialize to another Request Object
        Request* secondRequest = new (GetBlock<Request>()) Request();
        rc = secondRequest->deserialize(buf);
        C_ASSERT(rc == RC_SUCCESS);

        C_ASSERT(firstRequest->getRequestType() == secondRequest->getRequestType());
        C_ASSERT(firstRequest->getClientPID() == secondRequest->getClientPID());
        C_ASSERT(firstRequest->getClientTID() == secondRequest->getClientTID());
        C_ASSERT(firstRequest->getResourcesCount() == secondRequest->getResourcesCount());
        C_ASSERT(firstRequest->getProperties() == secondRequest->getProperties());
        C_ASSERT(firstRequest->getPriority() == secondRequest->getPriority());
        C_ASSERT(firstRequest->getHandle() == secondRequest->getHandle());
        C_ASSERT(firstRequest->getDuration() == secondRequest->getDuration());
        C_ASSERT(firstRequest->isBackgroundProcessingEnabled() == secondRequest->isBackgroundProcessingEnabled());
        C_ASSERT(firstRequest->getPriority() == secondRequest->getPriority());

        C_ASSERT(firstRequest->getResourcesCount() == secondRequest->getResourcesCount());

        for(int32_t i = 0; i < firstRequest->getResources()->size(); i++) {
            Resource* firstResource = firstRequest->getResourceAt(i);
            Resource* secondResource = secondRequest->getResourceAt(i);

            if(secondResource == nullptr) {
                return;
            }

            C_ASSERT(firstResource->getResInfo() == secondResource->getResInfo());
            C_ASSERT(firstResource->getValuesCount() == secondResource->getValuesCount());
            C_ASSERT(firstResource->mResValue.value == secondResource->mResValue.value);
        }

        Request::cleanUpRequest(firstRequest);
        Request::cleanUpRequest(secondRequest);

    } catch(const std::bad_alloc& e) {
    } catch(const std::exception& e) {}
}

static void TestSignalSerializingAndDeserializing() {
    try {
        MakeAlloc<Signal> (5);
        MakeAlloc<std::vector<uint32_t>> (5);

        Signal* firstSignal = new (GetBlock<Signal>()) Signal();
        firstSignal->setRequestType(REQ_RESOURCE_TUNING);
        firstSignal->setClientPID(1003);
        firstSignal->setClientTID(1009);
        firstSignal->setProperties((1 << 8) | (1 << 0));
        firstSignal->setHandle(15);
        firstSignal->setDuration(5600);
        firstSignal->setSignalCode(78099);
        firstSignal->setNumArgs(1);
        firstSignal->setAppName("example-app-name");
        firstSignal->setScenario("example-scenario-name");

        C_ASSERT(firstSignal->getRequestType() == REQ_RESOURCE_TUNING);
        C_ASSERT(firstSignal->getClientPID() == 1003);
        C_ASSERT(firstSignal->getClientTID() == 1009);
        C_ASSERT(firstSignal->getHandle() == 15);
        C_ASSERT(firstSignal->getSignalCode() == 78099);
        C_ASSERT(firstSignal->getDuration() == 5600);
        C_ASSERT(firstSignal->isBackgroundProcessingEnabled() == 1);
        C_ASSERT(firstSignal->getPriority() == 1);
        C_ASSERT(firstSignal->getNumArgs() == 1);
        C_ASSERT(firstSignal->getAppName() == "example-app-name");
        C_ASSERT(firstSignal->getScenario() == "example-scenario-name");

        std::vector<uint32_t>* listArgs = new (GetBlock<std::vector<uint32_t>>())
                                               std::vector<uint32_t>;

        listArgs->push_back(23);
        firstSignal->setList(listArgs);

        uint32_t firstListArg = firstSignal->getListArgAt(0);
        C_ASSERT(firstListArg == 23);

        char buf[1024];

        // Serializing the Signal
        ErrCode rc = firstSignal->serialize(buf);
        C_ASSERT(rc == RC_SUCCESS);

        // Deserialize to another Signal Object
        Signal* secondSignal = new (GetBlock<Signal>()) Signal();
        rc = secondSignal->deserialize(buf);
        C_ASSERT(rc == RC_SUCCESS);

        C_ASSERT(firstSignal->getRequestType() == secondSignal->getRequestType());
        C_ASSERT(firstSignal->getClientPID() == secondSignal->getClientPID());
        C_ASSERT(firstSignal->getClientTID() == secondSignal->getClientTID());
        C_ASSERT(firstSignal->getPriority() == secondSignal->getPriority());
        C_ASSERT(firstSignal->getHandle() == secondSignal->getHandle());
        C_ASSERT(firstSignal->getDuration() == secondSignal->getDuration());
        C_ASSERT(firstSignal->getProperties() == secondSignal->getProperties());
        C_ASSERT(firstSignal->isBackgroundProcessingEnabled() == secondSignal->isBackgroundProcessingEnabled());
        C_ASSERT(firstSignal->getPriority() == secondSignal->getPriority());

        C_ASSERT(firstSignal->getNumArgs() == secondSignal->getNumArgs());

        for(int32_t i = 0; i < firstSignal->getNumArgs(); i++) {
            C_ASSERT(firstSignal->getListArgAt(i) == secondSignal->getListArgAt(i));
        }

        Signal::cleanUpSignal(firstSignal);
        Signal::cleanUpSignal(secondSignal);

    } catch(const std::bad_alloc& e) {
    } catch(const std::exception& e) {}
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

int32_t main() {
    std::cout<<"Running Test Suite: [MiscTests]\n"<<std::endl;

    RUN_TEST(TestResourceStructCoreClusterSettingAndExtraction);
    RUN_TEST(TestRequestSerializingAndDeserializing);
    RUN_TEST(TestSignalSerializingAndDeserializing);
    RUN_TEST(TestHandleGeneration);
    RUN_TEST(TestAuxRoutineFileExists);

    std::cout<<"\nAll Tests from the suite: [MiscTests], executed successfully"<<std::endl;
    return 0;
}
