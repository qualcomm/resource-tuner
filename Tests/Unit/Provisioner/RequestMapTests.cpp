// Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
// SPDX-License-Identifier: BSD-3-Clause-Clear

#include <gtest/gtest.h>
#include <thread>

#include "RequestManager.h"
#include "RateLimiter.h"
#include "MemoryPool.h"

class RequestMapTests: public::testing::Test {
protected:
    void SetUp() override {
        static int8_t firstTest = true;
        if(firstTest == true) {
            MakeAlloc<ClientInfo> (30);
            MakeAlloc<ClientTidData> (30);
            MakeAlloc<std::vector<int32_t>> (30);
            MakeAlloc<std::unordered_set<int64_t>> (30);
            MakeAlloc<Resource> (30);
            MakeAlloc<std::vector<Resource*>> (30);
            MakeAlloc<Request> (30);
            firstTest = false;
        }
    }
};

// Helper methods for Resource Generation
Resource* generateResourceForTesting(int32_t seed) {
    Resource* resource = nullptr;
    try {
        resource = (Resource*) GetBlock<Resource>();
        resource->setOpCode(16 + seed);
        resource->setNumValues(1);
        resource->mConfigValue.singleValue = 2 * seed;
    } catch(const std::bad_alloc& e) {
        throw std::bad_alloc();
    }

    return resource;
}

Resource* generateResourceFromMemoryPoolForTesting(int32_t seed) {
    Resource* resource = new(GetBlock<Resource>()) Resource;
    resource->setOpCode(16 + seed);
    resource->setNumValues(1);
    resource->mConfigValue.singleValue = 2 * seed;

    return resource;
}

// No prior requests in the map, add a new one
// The request should be accepted
TEST_F(RequestMapTests, TestSingleRequestScenario) {
    std::shared_ptr<ClientDataManager> clientDataManager = ClientDataManager::getInstance();
    std::shared_ptr<RequestManager> requestMap = RequestManager::getInstance();

    Resource* resource = (Resource*) GetBlock<Resource>();

    resource->setOpCode(16);
    resource->setNumValues(1);
    resource->mConfigValue.singleValue = 8;

    std::vector<Resource*>* resources =
        new (GetBlock<std::vector<Resource*>>())std::vector<Resource*>;
    resources->push_back(resource);

    Request* request = new (GetBlock<Request>()) Request;
    request->setRequestType(REQ_RESOURCE_TUNING);
    request->setHandle(25);
    request->setDuration(-1);
    request->setPriority(REQ_PRIORITY_HIGH);
    request->setNumResources(1);
    request->setClientPID(321);
    request->setClientTID(321);
    request->setResources(resources);
    request->setBackgroundProcessing(false);

    if(!clientDataManager->clientExists(request->getClientPID(), request->getClientTID())) {
        clientDataManager->createNewClient(request->getClientPID(), request->getClientTID());
    }

    int8_t result = requestMap->shouldRequestBeAdded(request);

    ASSERT_EQ(result, true);

    clientDataManager->deleteClientPID(request->getClientPID());
    clientDataManager->deleteClientTID(request->getClientTID());

    Request::cleanUpRequest(request);
}

// Add duplicate requests. The second request should not be accepted
TEST_F(RequestMapTests, TestDuplicateRequestScenario1) {
    std::shared_ptr<ClientDataManager> clientDataManager = ClientDataManager::getInstance();
    std::shared_ptr<RequestManager> requestMap = RequestManager::getInstance();

    Resource* resource1 = generateResourceForTesting(1);
    Resource* resource2 = generateResourceForTesting(1);

    std::vector<Resource*>* resources1 =
        new (GetBlock<std::vector<Resource*>>()) std::vector<Resource*>;
    resources1->push_back(resource1);

    std::vector<Resource*>* resources2 =
        new (GetBlock<std::vector<Resource*>>()) std::vector<Resource*>;
    resources2->push_back(resource2);

    Request* firstRequest = new (GetBlock<Request>()) Request;
    firstRequest->setRequestType(REQ_RESOURCE_TUNING);
    firstRequest->setHandle(20);
    firstRequest->setDuration(-1);
    firstRequest->setPriority(REQ_PRIORITY_HIGH);
    firstRequest->setNumResources(1);
    firstRequest->setClientPID(321);
    firstRequest->setClientTID(321);
    firstRequest->setResources(resources1);
    firstRequest->setBackgroundProcessing(false);

    Request* secondRequest = new (GetBlock<Request>()) Request;
    secondRequest->setRequestType(REQ_RESOURCE_TUNING);
    secondRequest->setHandle(21);
    secondRequest->setDuration(-1);
    secondRequest->setPriority(REQ_PRIORITY_HIGH);
    secondRequest->setNumResources(1);
    secondRequest->setClientPID(321);
    secondRequest->setClientTID(321);
    secondRequest->setResources(resources2);
    secondRequest->setBackgroundProcessing(false);

    if(!clientDataManager->clientExists(firstRequest->getClientPID(), firstRequest->getClientTID())) {
        clientDataManager->createNewClient(firstRequest->getClientPID(), firstRequest->getClientTID());
    }

    int8_t resultFirst = requestMap->shouldRequestBeAdded(firstRequest);
    if(resultFirst) {
        requestMap->addRequest(firstRequest);
    }

    int8_t resultSecond = requestMap->shouldRequestBeAdded(secondRequest);
    if(resultSecond) {
        requestMap->addRequest(secondRequest);
    }

    ASSERT_EQ(resultFirst, true);
    ASSERT_EQ(resultSecond, false);

    requestMap->removeRequest(firstRequest);
    requestMap->removeRequest(secondRequest);

    clientDataManager->deleteClientPID(firstRequest->getClientPID());
    clientDataManager->deleteClientTID(firstRequest->getClientTID());

    Request::cleanUpRequest(firstRequest);
    Request::cleanUpRequest(secondRequest);
}

// Add duplicate requests with multiple resources. The second request should not be accepted
TEST_F(RequestMapTests, TestDuplicateRequestScenario2) {
    std::shared_ptr<ClientDataManager> clientDataManager = ClientDataManager::getInstance();
    std::shared_ptr<RequestManager> requestMap = RequestManager::getInstance();

    Resource* resource1 = generateResourceForTesting(1);
    Resource* resource2 = generateResourceForTesting(2);

    Resource* resource3 = generateResourceForTesting(1);
    Resource* resource4 = generateResourceForTesting(2);

    std::vector<Resource*>* resources1 =
        new (GetBlock<std::vector<Resource*>>()) std::vector<Resource*>;
    resources1->push_back(resource1);
    resources1->push_back(resource2);

    std::vector<Resource*>* resources2 =
        new (GetBlock<std::vector<Resource*>>()) std::vector<Resource*>;
    resources2->push_back(resource3);
    resources2->push_back(resource4);

    Request* firstRequest = new (GetBlock<Request>()) Request;
    firstRequest->setRequestType(REQ_RESOURCE_TUNING);
    firstRequest->setHandle(103);
    firstRequest->setDuration(-1);
    firstRequest->setPriority(REQ_PRIORITY_HIGH);
    firstRequest->setNumResources(1);
    firstRequest->setClientPID(321);
    firstRequest->setClientTID(321);
    firstRequest->setResources(resources1);
    firstRequest->setBackgroundProcessing(false);

    Request* secondRequest = new (GetBlock<Request>()) Request;
    secondRequest->setRequestType(REQ_RESOURCE_TUNING);
    secondRequest->setHandle(108);
    secondRequest->setDuration(-1);
    secondRequest->setPriority(REQ_PRIORITY_HIGH);
    secondRequest->setNumResources(1);
    secondRequest->setClientPID(321);
    secondRequest->setClientTID(321);
    secondRequest->setResources(resources2);
    secondRequest->setBackgroundProcessing(false);

    if(!clientDataManager->clientExists(firstRequest->getClientPID(), firstRequest->getClientTID())) {
        clientDataManager->createNewClient(firstRequest->getClientPID(), firstRequest->getClientTID());
    }

    int8_t resultFirst = requestMap->shouldRequestBeAdded(firstRequest);
    if(resultFirst) {
        requestMap->addRequest(firstRequest);
    }

    int8_t resultSecond = requestMap->shouldRequestBeAdded(secondRequest);
    if(resultSecond) {
        requestMap->addRequest(secondRequest);
    }

    ASSERT_EQ(resultFirst, true);
    ASSERT_EQ(resultSecond, false);

    requestMap->removeRequest(firstRequest);
    requestMap->removeRequest(secondRequest);

    clientDataManager->deleteClientPID(firstRequest->getClientPID());
    clientDataManager->deleteClientTID(firstRequest->getClientTID());

    Request::cleanUpRequest(firstRequest);
    Request::cleanUpRequest(secondRequest);
}

// For 2 requests to be considered duplicate, each and every one of their
// attributes should match. Else the request should be accepted
TEST_F(RequestMapTests, TestDuplicateRequestScenario3_1) {
    std::shared_ptr<ClientDataManager> clientDataManager = ClientDataManager::getInstance();
    std::shared_ptr<RequestManager> requestMap = RequestManager::getInstance();

    std::vector<Request*> requestsCreated;

    for(int32_t index = 0; index < 10; index++) {
        Resource* resource = generateResourceForTesting(0);

        // Slight modification
        resource->mConfigValue.singleValue = 8 + index;

        std::vector<Resource*>* resources =
            new (GetBlock<std::vector<Resource*>>())std::vector<Resource*>;

        resources->push_back(resource);

        Request* request = new (GetBlock<Request>()) Request;
        request->setRequestType(REQ_RESOURCE_TUNING);
        request->setHandle(112 + index);
        request->setDuration(-1);
        request->setNumResources(1);
        request->setPriority(REQ_PRIORITY_HIGH);
        request->setClientPID(321);
        request->setClientTID(321);
        request->setResources(resources);
        request->setBackgroundProcessing(false);

        if(!clientDataManager->clientExists(request->getClientPID(), request->getClientTID())) {
            clientDataManager->createNewClient(request->getClientPID(), request->getClientTID());
        }

        requestsCreated.push_back(request);

        int8_t requestCheck = requestMap->shouldRequestBeAdded(request);
        ASSERT_EQ(requestCheck, true);
        requestMap->addRequest(request);
    }

    clientDataManager->deleteClientPID(321);
    clientDataManager->deleteClientTID(321);

    for(int32_t i = 0; i < requestsCreated.size(); i++) {
        requestMap->removeRequest(requestsCreated[i]);
        Request::cleanUpRequest(requestsCreated[i]);
    }
}

// Duplicate Verification check, where the number of resources itself is different
// in the second request, hence it should be accepted.
TEST_F(RequestMapTests, TestDuplicateRequestScenario3_2) {
    std::shared_ptr<ClientDataManager> clientDataManager = ClientDataManager::getInstance();
    std::shared_ptr<RequestManager> requestMap = RequestManager::getInstance();

    Resource *resource1, *resource2, *duplicateResource1;
    try {
        resource1 = generateResourceForTesting(1);
        duplicateResource1 = generateResourceForTesting(1);
        resource2 = generateResourceForTesting(2);
    } catch(const std::bad_alloc& e) {
        FAIL();
    }

    std::vector<Resource*>* resources1;
    try {
        resources1 = new (GetBlock<std::vector<Resource*>>())std::vector<Resource*>;
    } catch(const std::bad_alloc& e) {
        FAIL();
    }

    resources1->push_back(resource1);

    std::vector<Resource*>* resources2;
    try {
        resources2 = new (GetBlock<std::vector<Resource*>>())std::vector<Resource*>;
    } catch(const std::bad_alloc& e) {
        FAIL();
    }

    resources2->push_back(duplicateResource1);
    resources2->push_back(resource2);

    Request* firstRequest;
    try {
        firstRequest = new (GetBlock<Request>()) Request;
    } catch(const std::bad_alloc& e) {
        FAIL();
    }

    firstRequest->setRequestType(REQ_RESOURCE_TUNING);
    firstRequest->setHandle(245);
    firstRequest->setDuration(-1);
    firstRequest->setPriority(REQ_PRIORITY_HIGH);
    firstRequest->setNumResources(1);
    firstRequest->setClientPID(321);
    firstRequest->setClientTID(321);
    firstRequest->setResources(resources1);
    firstRequest->setBackgroundProcessing(false);

    if(!clientDataManager->clientExists(firstRequest->getClientPID(), firstRequest->getClientTID())) {
        if(!clientDataManager->createNewClient(firstRequest->getClientPID(), firstRequest->getClientTID())) {
            FAIL();
        }
    }

    int8_t resultFirst = requestMap->shouldRequestBeAdded(firstRequest);
    if(resultFirst) {
        requestMap->addRequest(firstRequest);
    }

    Request* secondRequest;
    try {
        secondRequest = new (GetBlock<Request>()) Request;
    } catch(const std::bad_alloc& e) {
        FAIL();
    }

    secondRequest->setRequestType(REQ_RESOURCE_TUNING);
    secondRequest->setHandle(300);
    secondRequest->setDuration(-1);
    secondRequest->setPriority(REQ_PRIORITY_HIGH);
    secondRequest->setNumResources(2);
    secondRequest->setClientPID(321);
    secondRequest->setClientTID(321);
    secondRequest->setResources(resources2);
    secondRequest->setBackgroundProcessing(false);

    if(!clientDataManager->clientExists(secondRequest->getClientPID(), secondRequest->getClientTID())) {
        if(!clientDataManager->createNewClient(secondRequest->getClientPID(), secondRequest->getClientTID())) {
            FAIL();
        }
    }

    int8_t resultSecond = requestMap->shouldRequestBeAdded(secondRequest);
    if(resultSecond) {
        requestMap->addRequest(secondRequest);
    }

    ASSERT_EQ(resultFirst, true);
    ASSERT_EQ(resultSecond, true);

    requestMap->removeRequest(firstRequest);
    requestMap->removeRequest(secondRequest);

    clientDataManager->deleteClientPID(firstRequest->getClientPID());
    clientDataManager->deleteClientTID(firstRequest->getClientTID());

    Request::cleanUpRequest(firstRequest);
    Request::cleanUpRequest(secondRequest);

}

// Add requests from the same client with multiple resources, where not all resources are identical.
// Both requests should be accepted
TEST_F(RequestMapTests, TestDuplicateRequestScenario4) {
    std::shared_ptr<ClientDataManager> clientDataManager = ClientDataManager::getInstance();
    std::shared_ptr<RequestManager> requestMap = RequestManager::getInstance();

    Resource* resource1;
    Resource* duplicateResource1;
    Resource* resource2;
    Resource* resource3;

    try {
        resource1 = generateResourceForTesting(1);
        duplicateResource1 = generateResourceForTesting(1);
        resource2 = generateResourceForTesting(2);
        resource3 = generateResourceForTesting(3);
    } catch(const std::bad_alloc& e) {
        FAIL();
    }

    std::vector<Resource*>* resources1;
    try {
        resources1 = new (GetBlock<std::vector<Resource*>>())std::vector<Resource*>;
    } catch(const std::bad_alloc& e) {
        FAIL();
    }

    std::vector<Resource*>* resources2;
    try {
        resources2 = new (GetBlock<std::vector<Resource*>>())std::vector<Resource*>;
    } catch(const std::bad_alloc& e) {
        FAIL();
    }

    resources1->push_back(resource1);
    resources1->push_back(resource2);

    resources2->push_back(duplicateResource1);
    resources2->push_back(resource3);

    Request* firstRequest;
    try {
        firstRequest = new (GetBlock<Request>()) Request();
    } catch(const std::bad_alloc &e) {
        FAIL();
    }

    firstRequest->setRequestType(REQ_RESOURCE_TUNING);
    firstRequest->setHandle(320);
    firstRequest->setDuration(-1);
    firstRequest->setNumResources(2);
    firstRequest->setPriority(REQ_PRIORITY_HIGH);
    firstRequest->setClientPID(321);
    firstRequest->setClientTID(321);
    firstRequest->setResources(resources1);
    firstRequest->setBackgroundProcessing(false);

    if(!clientDataManager->clientExists(firstRequest->getClientPID(), firstRequest->getClientTID())) {
        if(!clientDataManager->createNewClient(firstRequest->getClientPID(), firstRequest->getClientTID())) {
            FAIL();
        }
    }

    int8_t resultFirst = requestMap->shouldRequestBeAdded(firstRequest);
    if(resultFirst) {
        requestMap->addRequest(firstRequest);
    }

    Request* secondRequest;
    try {
        secondRequest = new (GetBlock<Request>()) Request();
    } catch(const std::bad_alloc &e) {
        FAIL();
    }

    secondRequest->setRequestType(REQ_RESOURCE_TUNING);
    secondRequest->setHandle(334);
    secondRequest->setDuration(-1);
    secondRequest->setNumResources(2);
    secondRequest->setPriority(REQ_PRIORITY_HIGH);
    secondRequest->setClientPID(321);
    secondRequest->setClientTID(321);
    secondRequest->setResources(resources2);
    secondRequest->setBackgroundProcessing(false);

    if(!clientDataManager->clientExists(secondRequest->getClientPID(), secondRequest->getClientTID())) {
        if(!clientDataManager->createNewClient(secondRequest->getClientPID(), secondRequest->getClientTID())) {
            FAIL();
        }
    }

    int8_t resultSecond = requestMap->shouldRequestBeAdded(secondRequest);
    if(resultSecond) {
        requestMap->addRequest(secondRequest);
    }

    ASSERT_EQ(resultFirst, true);
    ASSERT_EQ(resultSecond, true);

    requestMap->removeRequest(firstRequest);
    requestMap->removeRequest(secondRequest);

    clientDataManager->deleteClientPID(firstRequest->getClientPID());
    clientDataManager->deleteClientTID(firstRequest->getClientTID());

    Request::cleanUpRequest(firstRequest);
    Request::cleanUpRequest(secondRequest);
}

// Multiple clients try to add requests which are duplicates of each other in
// terms of resources.
// All request should still be accepted, since clients are different
TEST_F(RequestMapTests, TestMultipleClientsScenario5) {
    std::shared_ptr<ClientDataManager> clientDataManager = ClientDataManager::getInstance();
    std::shared_ptr<RequestManager> requestMap = RequestManager::getInstance();

    std::vector<Resource*> resource1;
    std::vector<Resource*> resource2;

    try {
        resource1.push_back(generateResourceForTesting(1));
        resource1.push_back(generateResourceForTesting(1));
        resource1.push_back(generateResourceForTesting(1));

        resource2.push_back(generateResourceForTesting(2));
        resource2.push_back(generateResourceForTesting(2));
        resource2.push_back(generateResourceForTesting(2));

    } catch(const std::bad_alloc& e) {
        FAIL();
    }

    std::vector<Resource*>* resources1;
    try {
        resources1 = new (GetBlock<std::vector<Resource*>>())std::vector<Resource*>;
    } catch(const std::bad_alloc& e) {
        FAIL();
    }

    std::vector<Resource*>* resources2;
    try {
        resources2 = new (GetBlock<std::vector<Resource*>>())std::vector<Resource*>;
    } catch(const std::bad_alloc& e) {
        FAIL();
    }

    std::vector<Resource*>* resources3;
    try {
        resources3 = new (GetBlock<std::vector<Resource*>>())std::vector<Resource*>;
    } catch(const std::bad_alloc& e) {
        FAIL();
    }

    resources1->push_back(resource1[0]);
    resources1->push_back(resource2[0]);

    resources2->push_back(resource1[1]);
    resources2->push_back(resource2[1]);

    resources3->push_back(resource1[2]);
    resources3->push_back(resource2[2]);

    Request* firstRequest;
    Request* secondRequest;
    Request* thirdRequest;

    try {
        firstRequest = new (GetBlock<Request>()) Request();
        secondRequest = new (GetBlock<Request>()) Request();
        thirdRequest = new (GetBlock<Request>()) Request();

    } catch(const std::bad_alloc &e) {
        FAIL();
    }

    firstRequest->setRequestType(REQ_RESOURCE_TUNING);
    firstRequest->setHandle(133);
    firstRequest->setDuration(-1);
    firstRequest->setPriority(REQ_PRIORITY_HIGH);
    firstRequest->setNumResources(1);
    firstRequest->setClientPID(321);
    firstRequest->setClientTID(321);
    firstRequest->setResources(resources1);
    firstRequest->setBackgroundProcessing(false);

    secondRequest->setRequestType(REQ_RESOURCE_TUNING);
    secondRequest->setHandle(144);
    secondRequest->setDuration(-1);
    secondRequest->setPriority(REQ_PRIORITY_HIGH);
    secondRequest->setNumResources(1);
    secondRequest->setClientPID(354);
    secondRequest->setClientTID(354);
    secondRequest->setResources(resources2);
    secondRequest->setBackgroundProcessing(false);

    thirdRequest->setRequestType(REQ_RESOURCE_TUNING);
    thirdRequest->setHandle(155);
    thirdRequest->setDuration(-1);
    thirdRequest->setPriority(REQ_PRIORITY_HIGH);
    thirdRequest->setNumResources(1);
    thirdRequest->setClientPID(100);
    thirdRequest->setClientTID(127);
    thirdRequest->setResources(resources3);
    thirdRequest->setBackgroundProcessing(false);

    if(!clientDataManager->clientExists(firstRequest->getClientPID(), firstRequest->getClientTID())) {
        if(!clientDataManager->createNewClient(firstRequest->getClientPID(), firstRequest->getClientTID())) {
            FAIL();
        }
    }

    if(!clientDataManager->clientExists(secondRequest->getClientPID(), secondRequest->getClientTID())) {
        if(!clientDataManager->createNewClient(secondRequest->getClientPID(), secondRequest->getClientTID())) {
            FAIL();
        }
    }

    if(!clientDataManager->clientExists(thirdRequest->getClientPID(), thirdRequest->getClientTID())) {
        if(!clientDataManager->createNewClient(thirdRequest->getClientPID(), thirdRequest->getClientTID())) {
            FAIL();
        }
    }

    int8_t resultFirst = requestMap->shouldRequestBeAdded(firstRequest);
    if(resultFirst) {
        requestMap->addRequest(firstRequest);
    }

    int8_t resultSecond = requestMap->shouldRequestBeAdded(secondRequest);
    if(resultSecond) {
        requestMap->addRequest(secondRequest);
    }

    int8_t resultThird = requestMap->shouldRequestBeAdded(thirdRequest);
    if(resultThird) {
        requestMap->addRequest(thirdRequest);
    }

    ASSERT_EQ(resultFirst, true);
    ASSERT_EQ(resultSecond, true);
    ASSERT_EQ(resultThird, true);

    requestMap->removeRequest(firstRequest);
    requestMap->removeRequest(secondRequest);
    requestMap->removeRequest(thirdRequest);

    clientDataManager->deleteClientPID(firstRequest->getClientPID());
    clientDataManager->deleteClientPID(secondRequest->getClientPID());
    clientDataManager->deleteClientPID(thirdRequest->getClientPID());
    clientDataManager->deleteClientTID(firstRequest->getClientTID());
    clientDataManager->deleteClientTID(secondRequest->getClientTID());
    clientDataManager->deleteClientTID(thirdRequest->getClientTID());

    Request::cleanUpRequest(firstRequest);
    Request::cleanUpRequest(secondRequest);
    Request::cleanUpRequest(thirdRequest);
}

// For retune / untune APIs, request with specified handle should be
// present in the RequestMap
TEST_F(RequestMapTests, TestRequestWithHandleExists1) {
    std::shared_ptr<ClientDataManager> clientDataManager = ClientDataManager::getInstance();
    std::shared_ptr<RequestManager> requestMap = RequestManager::getInstance();

    Resource* resource;
    try {
        resource = generateResourceForTesting(1);
    } catch(const std::bad_alloc& e) {
        FAIL();
    }

    std::vector<Resource*>* resources;
    try {
        resources = new (GetBlock<std::vector<Resource*>>())std::vector<Resource*>;
    } catch(const std::bad_alloc& e) {
        FAIL();
    }
    resources->push_back(resource);

    Request* request;
    try {
        request = new (GetBlock<Request>()) Request();
    } catch(const std::bad_alloc& e) {
        FAIL();
    }

    request->setRequestType(REQ_RESOURCE_TUNING);
    request->setHandle(20);
    request->setDuration(-1);
    request->setNumResources(1);
    request->setPriority(REQ_PRIORITY_HIGH);
    request->setClientTID(321);
    request->setClientTID(321);
    request->setResources(resources);
    request->setBackgroundProcessing(false);

    if(!clientDataManager->clientExists(request->getClientPID(), request->getClientTID())) {
        if(!clientDataManager->createNewClient(request->getClientPID(), request->getClientTID())) {
            FAIL();
        }
    }

    int8_t requestCheck = requestMap->shouldRequestBeAdded(request);
    if(requestCheck) {
        requestMap->addRequest(request);
    }
    ASSERT_EQ(requestCheck, true);

    int8_t result = requestMap->verifyHandle(20);
    ASSERT_EQ(result, true);

    requestMap->removeRequest(request);

    clientDataManager->deleteClientPID(request->getClientPID());
    clientDataManager->deleteClientTID(request->getClientTID());

    Request::cleanUpRequest(request);
}

TEST_F(RequestMapTests, TestRequestWithHandleExists2) {
    std::shared_ptr<ClientDataManager> clientDataManager = ClientDataManager::getInstance();
    std::shared_ptr<RequestManager> requestMap = RequestManager::getInstance();

    Resource* resource;
    try {
        resource = generateResourceForTesting(1);
    } catch(const std::bad_alloc& e) {
        FAIL();
    }

    std::vector<Resource*>* resources;
    try {
        resources = new (GetBlock<std::vector<Resource*>>())std::vector<Resource*>;
    } catch(const std::bad_alloc& e) {
        FAIL();
    }
    resources->push_back(resource);

    Request* request;
    try {
        request = new (GetBlock<Request>()) Request();
    } catch(const std::bad_alloc& e) {
        FAIL();
    }

    request->setRequestType(REQ_RESOURCE_TUNING);
    request->setHandle(20);
    request->setDuration(-1);
    request->setNumResources(1);
    request->setPriority(REQ_PRIORITY_HIGH);
    request->setClientTID(321);
    request->setClientTID(321);
    request->setResources(resources);
    request->setBackgroundProcessing(false);

    if(!clientDataManager->clientExists(request->getClientPID(), request->getClientTID())) {
        if(!clientDataManager->createNewClient(request->getClientPID(), request->getClientTID())) {
            FAIL();
        }
    }

    int8_t requestCheck = requestMap->shouldRequestBeAdded(request);
    if(requestCheck) {
        requestMap->addRequest(request);
    }
    ASSERT_EQ(requestCheck, true);

    int8_t result = requestMap->verifyHandle(64);
    ASSERT_EQ(result, false);

    requestMap->removeRequest(request);

    clientDataManager->deleteClientPID(request->getClientPID());
    clientDataManager->deleteClientTID(request->getClientTID());

    Request::cleanUpRequest(request);
}

// Add a request to the map
// Check if a request with that handle exists
// free the request from the RequestMap
// Verify that no request with that handle exists in the map now.
TEST_F(RequestMapTests, TestRequestDeletion1) {
    int32_t testClientPID = 321;
    int32_t testClientTID = 321;

    std::shared_ptr<ClientDataManager> clientDataManager = ClientDataManager::getInstance();
    std::shared_ptr<RequestManager> requestMap = RequestManager::getInstance();

    Resource* resource;
    try {
        resource = generateResourceFromMemoryPoolForTesting(1);

    } catch(const std::bad_alloc& e) {
        FAIL();
    }

    std::vector<Resource*>* resources;

    try {
        resources = new (GetBlock<std::vector<Resource*>>()) std::vector<Resource*>;
    } catch(const std::bad_alloc& e) {
        FAIL();
    }

    resources->push_back(resource);

    Request* request;
    try {
        request = new(GetBlock<Request>()) Request();
    } catch(const std::bad_alloc& e) {
        FAIL();
    }

    request->setRequestType(REQ_RESOURCE_TUNING);
    request->setHandle(25);
    request->setDuration(-1);
    request->setPriority(REQ_PRIORITY_HIGH);
    request->setNumResources(1);
    request->setClientPID(testClientPID);
    request->setClientTID(testClientTID);
    request->setResources(resources);
    request->setBackgroundProcessing(false);

    if(!clientDataManager->clientExists(request->getClientPID(), request->getClientTID())) {
        if(!clientDataManager->createNewClient(request->getClientPID(), request->getClientTID())) {
            FAIL();
        }
    }

    int8_t requestCheck = requestMap->shouldRequestBeAdded(request);
    if(requestCheck) {
        requestMap->addRequest(request);
    }
    ASSERT_EQ(requestCheck, true);

    ASSERT_EQ(requestMap->verifyHandle(25), true);
    requestMap->removeRequest(request);
    ASSERT_EQ(requestMap->verifyHandle(25), false);

    clientDataManager->deleteClientPID(testClientPID);
    clientDataManager->deleteClientTID(testClientTID);

    Request::cleanUpRequest(request);
}

// Add a request R from client C, verify it's added successfully
// Try adding R again, the operation should fail
// free(Request R from the map
// Now try adding R back to the RequestMap, the operation should succeed.
TEST_F(RequestMapTests, TestRequestDeletion2) {
    int32_t testClientPID = 321;
    int32_t testClientTID = 321;

    std::shared_ptr<ClientDataManager> clientDataManager = ClientDataManager::getInstance();
    std::shared_ptr<RequestManager> requestMap = RequestManager::getInstance();

    Resource* resource;
    Resource* duplicateResource;

    try {
        resource = generateResourceFromMemoryPoolForTesting(1);
        duplicateResource = generateResourceFromMemoryPoolForTesting(1);
    } catch(const std::bad_alloc& e) {
        FAIL();
    }

    std::vector<Resource*>* resources1;
    std::vector<Resource*>* resources2;

    try {
        resources1 = new (GetBlock<std::vector<Resource*>>()) std::vector<Resource*>;
        resources2 = new (GetBlock<std::vector<Resource*>>()) std::vector<Resource*>;
    } catch(const std::bad_alloc& e) {
        FAIL();
    }

    resources1->push_back(resource);
    resources2->push_back(duplicateResource);

    Request *request, *duplicateRequest;
    try {
        request = new(GetBlock<Request>()) Request();
        duplicateRequest = new(GetBlock<Request>()) Request();
    } catch(const std::bad_alloc& e) {
        FAIL();
    }

    request->setRequestType(REQ_RESOURCE_TUNING);
    request->setHandle(25);
    request->setDuration(-1);
    request->setPriority(REQ_PRIORITY_HIGH);
    request->setNumResources(1);
    request->setClientPID(testClientPID);
    request->setClientTID(testClientTID);
    request->setResources(resources1);
    request->setBackgroundProcessing(false);

    duplicateRequest->setRequestType(REQ_RESOURCE_TUNING);
    duplicateRequest->setHandle(25);
    duplicateRequest->setDuration(-1);
    duplicateRequest->setPriority(REQ_PRIORITY_HIGH);
    duplicateRequest->setNumResources(1);
    duplicateRequest->setClientPID(testClientPID);
    duplicateRequest->setClientTID(testClientTID);
    duplicateRequest->setResources(resources2);
    duplicateRequest->setBackgroundProcessing(false);

    if(!clientDataManager->clientExists(request->getClientPID(), request->getClientTID())) {
        if(!clientDataManager->createNewClient(request->getClientPID(), request->getClientTID())) {
            FAIL();
        }
    }

    int8_t requestCheck = requestMap->shouldRequestBeAdded(request);
    if(requestCheck) {
        requestMap->addRequest(request);
    }
    ASSERT_EQ(requestCheck, true);

    requestCheck = requestMap->shouldRequestBeAdded(duplicateRequest);
    if(requestCheck) {
        requestMap->addRequest(duplicateRequest);
    }
    ASSERT_EQ(requestCheck, false);

    requestMap->removeRequest(request);

    requestCheck = requestMap->shouldRequestBeAdded(duplicateRequest);
    if(requestCheck) {
        requestMap->addRequest(duplicateRequest);
    }
    ASSERT_EQ(requestCheck, true);

    requestMap->removeRequest(duplicateRequest);

    clientDataManager->deleteClientPID(testClientPID);
    clientDataManager->deleteClientTID(testClientTID);

    Request::cleanUpRequest(request);
    Request::cleanUpRequest(duplicateRequest);
}

// Corner cases
// These tests cover the cases of null requests and requests
// with one or more resources being null.
// For such cases, RequestMap rejects the request,
// No need to futher process such a malformed request.
TEST_F(RequestMapTests, TestNullRequestAddition) {
    std::shared_ptr<ClientDataManager> clientDataManager = ClientDataManager::getInstance();
    std::shared_ptr<RequestManager> requestMap = RequestManager::getInstance();

    ASSERT_EQ(requestMap->shouldRequestBeAdded(nullptr), false);
}

TEST_F(RequestMapTests, TestRequestWithNullResourcesAddition) {
    std::shared_ptr<ClientDataManager> clientDataManager = ClientDataManager::getInstance();
    std::shared_ptr<RequestManager> requestMap = RequestManager::getInstance();

    std::vector<Resource*>* resources;

    try {
        resources = new (GetBlock<std::vector<Resource*>>()) std::vector<Resource*>;

    } catch(const std::bad_alloc& e) {
        FAIL();
    }

    resources->push_back(nullptr);

    Request *request;
    try {
        request = new(GetBlock<Request>()) Request();
    } catch(const std::bad_alloc& e) {
        FAIL();
    }

    request->setRequestType(REQ_RESOURCE_TUNING);
    request->setHandle(25);
    request->setDuration(-1);
    request->setNumResources(1);
    request->setPriority(REQ_PRIORITY_HIGH);
    request->setClientPID(321);
    request->setClientTID(321);
    request->setResources(resources);
    request->setBackgroundProcessing(false);

    if(!clientDataManager->clientExists(request->getClientPID(), request->getClientTID())) {
        if(!clientDataManager->createNewClient(request->getClientPID(), request->getClientTID())) {
            FAIL();
        }
    }

    int8_t requestCheck = requestMap->shouldRequestBeAdded(request);
    if(requestCheck) {
        requestMap->addRequest(request);
    }
    ASSERT_EQ(requestCheck, false);

    clientDataManager->deleteClientPID(request->getClientPID());
    clientDataManager->deleteClientTID(request->getClientTID());

    Request::cleanUpRequest(request);
}

TEST_F(RequestMapTests, TestGetRequestFromMap) {
    int32_t testClientPID = 321;
    int32_t testClientTID = 321;

    std::shared_ptr<ClientDataManager> clientDataManager = ClientDataManager::getInstance();
    std::shared_ptr<RequestManager> requestMap = RequestManager::getInstance();

    Resource* resource;

    try {
        resource = generateResourceFromMemoryPoolForTesting(1);
    } catch(const std::bad_alloc& e) {
        FAIL();
    }

    resource->setOpCode(15564);
    resource->setOptionalInfo(4445);
    resource->setNumValues(1);
    resource->mConfigValue.singleValue = 42;

    std::vector<Resource*>* resources;

    try {
        resources = new (GetBlock<std::vector<Resource*>>()) std::vector<Resource*>;
    } catch(const std::bad_alloc& e) {
        FAIL();
    }

    resources->push_back(resource);

    Request *request;
    try {
        request = new(GetBlock<Request>()) Request();
    } catch(const std::bad_alloc& e) {
        FAIL();
    }

    request->setRequestType(REQ_RESOURCE_TUNING);
    request->setHandle(325);
    request->setDuration(-1);
    request->setNumResources(1);
    request->setPriority(REQ_PRIORITY_HIGH);
    request->setClientPID(testClientPID);
    request->setClientTID(testClientTID);
    request->setResources(resources);
    request->setBackgroundProcessing(false);

    if(!clientDataManager->clientExists(request->getClientPID(), request->getClientTID())) {
        if(!clientDataManager->createNewClient(request->getClientPID(), request->getClientTID())) {
            FAIL();
        }
    }

    int8_t result = requestMap->shouldRequestBeAdded(request);
    if(result) {
        requestMap->addRequest(request);
    }

    ASSERT_EQ(result, true);

    // Retrieve request and check it's integrity
    Request* fetchedRequest = requestMap->getRequestFromMap(325);

    ASSERT_NE(fetchedRequest, nullptr);
    ASSERT_EQ(fetchedRequest->getDuration(), -1);
    ASSERT_EQ(fetchedRequest->getClientPID(), testClientPID);
    ASSERT_EQ(fetchedRequest->getClientTID(), testClientTID);
    ASSERT_EQ(fetchedRequest->getResourcesCount(), 1);

    requestMap->removeRequest(request);

    fetchedRequest = requestMap->getRequestFromMap(325);
    ASSERT_EQ(fetchedRequest, nullptr);

    clientDataManager->deleteClientPID(testClientPID);
    clientDataManager->deleteClientTID(testClientTID);

    Request::cleanUpRequest(request);
}
