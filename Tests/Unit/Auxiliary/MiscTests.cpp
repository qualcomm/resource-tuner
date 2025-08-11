// Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
// SPDX-License-Identifier: BSD-3-Clause-Clear

#include <thread>
#include <cstdint>
#include <gtest/gtest.h>

#include "MemoryPool.h"
#include "Request.h"
#include "SysConfig.h"
#include "Signal.h"

// Request Cleanup Tests
TEST(MiscTests, TestResourceStructCoreClusterSettingAndExtraction) {
    Resource* resource = new Resource;
    ASSERT_NE(resource, nullptr);

    resource->setCoreValue(2);
    resource->setClusterValue(1);

    ASSERT_EQ(resource->getCoreValue(), 2);
    ASSERT_EQ(resource->getClusterValue(), 1);

    resource->setResourceID(1);
    resource->setResourceType(1);

    ASSERT_EQ(resource->getOpCode(), (uint32_t)((1 << 16) | (1 << 0)));

    delete resource;
}

TEST(MiscTests, TestRequestSerializingAndDeserializing) {
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

        ASSERT_EQ(firstRequest->getRequestType(), REQ_RESOURCE_TUNING);
        ASSERT_EQ(firstRequest->getClientPID(), 1003);
        ASSERT_EQ(firstRequest->getClientTID(), 1009);
        ASSERT_EQ(firstRequest->getHandle(), 15);
        ASSERT_EQ(firstRequest->getResourcesCount(), 1);
        ASSERT_EQ(firstRequest->getDuration(), 5600);
        ASSERT_EQ(firstRequest->isBackgroundProcessingEnabled(), 1);
        ASSERT_EQ(firstRequest->getPriority(), 1);

        std::vector<Resource*>* firstReqResourceList = new (GetBlock<std::vector<Resource*>>())
                                                            std::vector<Resource*>;

        Resource* res1 = (Resource*) GetBlock<Resource>();
        res1->setOpCode(65536);
        res1->setNumValues(1);
        res1->mConfigValue.singleValue = 754;
        firstReqResourceList->push_back(res1);
        firstRequest->setResources(firstReqResourceList);

        Resource* resource = firstRequest->getResourceAt(0);
        ASSERT_EQ(resource->getOpCode(), 65536);
        ASSERT_EQ(resource->getValuesCount(), 1);
        ASSERT_EQ(resource->mConfigValue.singleValue, 754);

        char buf[1024];

        // Serializing the Request
        ErrCode rc = firstRequest->serialize(buf);
        ASSERT_EQ(rc, RC_SUCCESS);

        // Deserialize to another Request Object
        Request* secondRequest = new (GetBlock<Request>()) Request();
        rc = secondRequest->deserialize(buf);
        ASSERT_EQ(rc, RC_SUCCESS);

        ASSERT_EQ(firstRequest->getRequestType(), secondRequest->getRequestType());
        ASSERT_EQ(firstRequest->getClientPID(), secondRequest->getClientPID());
        ASSERT_EQ(firstRequest->getClientTID(), secondRequest->getClientTID());
        ASSERT_EQ(firstRequest->getResourcesCount(), secondRequest->getResourcesCount());
        ASSERT_EQ(firstRequest->getProperties(), secondRequest->getProperties());
        ASSERT_EQ(firstRequest->getPriority(), secondRequest->getPriority());
        ASSERT_EQ(firstRequest->getHandle(), secondRequest->getHandle());
        ASSERT_EQ(firstRequest->getDuration(), secondRequest->getDuration());
        ASSERT_EQ(firstRequest->isBackgroundProcessingEnabled(), secondRequest->isBackgroundProcessingEnabled());
        ASSERT_EQ(firstRequest->getPriority(), secondRequest->getPriority());

        ASSERT_EQ(firstRequest->getResourcesCount(), secondRequest->getResourcesCount());

        for(int32_t i = 0; i < firstRequest->getResources()->size(); i++) {
            Resource* firstResource = firstRequest->getResourceAt(i);
            Resource* secondResource = secondRequest->getResourceAt(i);

            if(secondResource == nullptr) {
                FAIL();
            }

            ASSERT_EQ(firstResource->getOperationalInfo(), secondResource->getOperationalInfo());
            ASSERT_EQ(firstResource->getValuesCount(), secondResource->getValuesCount());
            ASSERT_EQ(firstResource->mConfigValue.singleValue, secondResource->mConfigValue.singleValue);
        }

        Request::cleanUpRequest(firstRequest);
        Request::cleanUpRequest(secondRequest);

    } catch(const std::bad_alloc& e) {
        FAIL();

    } catch(const std::exception& e) {
        FAIL();
    }
}

TEST(MiscTests, TestSignalSerializingAndDeserializing) {
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
        firstSignal->setSignalID(78099);
        firstSignal->setNumArgs(1);
        firstSignal->setAppName("example-app-name");
        firstSignal->setScenario("example-scenario-name");

        ASSERT_EQ(firstSignal->getRequestType(), REQ_RESOURCE_TUNING);
        ASSERT_EQ(firstSignal->getClientPID(), 1003);
        ASSERT_EQ(firstSignal->getClientTID(), 1009);
        ASSERT_EQ(firstSignal->getHandle(), 15);
        ASSERT_EQ(firstSignal->getSignalID(), 78099);
        ASSERT_EQ(firstSignal->getDuration(), 5600);
        ASSERT_EQ(firstSignal->isBackgroundProcessingEnabled(), 1);
        ASSERT_EQ(firstSignal->getPriority(), 1);
        ASSERT_EQ(firstSignal->getNumArgs(), 1);
        ASSERT_EQ(firstSignal->getAppName(), "example-app-name");
        ASSERT_EQ(firstSignal->getScenario(), "example-scenario-name");

        std::vector<uint32_t>* listArgs = new (GetBlock<std::vector<uint32_t>>())
                                               std::vector<uint32_t>;

        listArgs->push_back(23);
        firstSignal->setList(listArgs);

        uint32_t firstListArg = firstSignal->getListArgAt(0);
        ASSERT_EQ(firstListArg, 23);

        char buf[1024];

        // Serializing the Signal
        ErrCode rc = firstSignal->serialize(buf);
        ASSERT_EQ(rc, RC_SUCCESS);

        // Deserialize to another Signal Object
        Signal* secondSignal = new (GetBlock<Signal>()) Signal();
        rc = secondSignal->deserialize(buf);
        ASSERT_EQ(rc, RC_SUCCESS);

        ASSERT_EQ(firstSignal->getRequestType(), secondSignal->getRequestType());
        ASSERT_EQ(firstSignal->getClientPID(), secondSignal->getClientPID());
        ASSERT_EQ(firstSignal->getClientTID(), secondSignal->getClientTID());
        ASSERT_EQ(firstSignal->getPriority(), secondSignal->getPriority());
        ASSERT_EQ(firstSignal->getHandle(), secondSignal->getHandle());
        ASSERT_EQ(firstSignal->getDuration(), secondSignal->getDuration());
        ASSERT_EQ(firstSignal->getProperties(), secondSignal->getProperties());
        ASSERT_EQ(firstSignal->isBackgroundProcessingEnabled(), secondSignal->isBackgroundProcessingEnabled());
        ASSERT_EQ(firstSignal->getPriority(), secondSignal->getPriority());

        ASSERT_EQ(firstSignal->getNumArgs(), secondSignal->getNumArgs());

        for(int32_t i = 0; i < firstSignal->getNumArgs(); i++) {
            ASSERT_EQ(firstSignal->getListArgAt(i), secondSignal->getListArgAt(i));
        }

        Signal::cleanUpSignal(firstSignal);
        Signal::cleanUpSignal(secondSignal);

    } catch(const std::bad_alloc& e) {
        FAIL();

    } catch(const std::exception& e) {
        FAIL();
    }
}

TEST(MiscTests, TestSysConfigSerializingAndDeserializing) {
 try {
        MakeAlloc<SysConfig> (5);

        SysConfig* firstConfig = new (GetBlock<SysConfig>()) SysConfig();
        firstConfig->setRequestType(REQ_SYSCONFIG_GET_PROP);
        firstConfig->setClientPID(1003);
        firstConfig->setClientTID(1009);
        firstConfig->setProp("resourceTuner.rate_limiter.min_health");
        firstConfig->setValue("104");
        firstConfig->setDefaultValue("67");
        firstConfig->setBufferSize(1445);

        ASSERT_EQ(firstConfig->getRequestType(), REQ_SYSCONFIG_GET_PROP);
        ASSERT_EQ(firstConfig->getClientPID(), 1003);
        ASSERT_EQ(firstConfig->getClientTID(), 1009);
        ASSERT_EQ(firstConfig->getProp(), "resourceTuner.rate_limiter.min_health");
        ASSERT_EQ(firstConfig->getValue(), "104");
        ASSERT_EQ(firstConfig->getDefaultValue(), "67");
        ASSERT_EQ(firstConfig->getBufferSize(), 1445);

        char buf[1024];

        // Serializing the SysConfig struct
        ErrCode rc = firstConfig->serialize(buf);
        ASSERT_EQ(rc, RC_SUCCESS);

        // Deserialize to another SysConfig Object
        SysConfig* secondConfig = new (GetBlock<SysConfig>()) SysConfig();
        rc = secondConfig->deserialize(buf);
        ASSERT_EQ(rc, RC_SUCCESS);

        ASSERT_EQ(firstConfig->getRequestType(), secondConfig->getRequestType());
        ASSERT_EQ(firstConfig->getClientPID(), secondConfig->getClientPID());
        ASSERT_EQ(firstConfig->getClientTID(), secondConfig->getClientTID());
        ASSERT_EQ(firstConfig->getProp(), secondConfig->getProp());
        ASSERT_EQ(firstConfig->getValue(), secondConfig->getValue());
        ASSERT_EQ(firstConfig->getDefaultValue(), secondConfig->getDefaultValue());
        ASSERT_EQ(firstConfig->getBufferSize(), secondConfig->getBufferSize());

    } catch(const std::bad_alloc& e) {
        FAIL();

    } catch(const std::exception& e) {
        FAIL();
    }
}
