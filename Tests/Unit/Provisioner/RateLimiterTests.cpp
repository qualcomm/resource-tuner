// Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
// SPDX-License-Identifier: BSD-3-Clause-Clear

#include <gtest/gtest.h>
#include <thread>

#include "RequestManager.h"
#include "RateLimiter.h"

class RateLimiterTests: public::testing::Test {
protected:
    void SetUp() override {
        static int8_t firstTest = true;
        if(firstTest == true) {
            firstTest = false;
            MakeAlloc<ClientInfo> (30);
            MakeAlloc<ClientTidData> (30);
            MakeAlloc<std::unordered_set<int64_t>> (30);
            MakeAlloc<std::vector<int32_t>> (30);
            MakeAlloc<Resource> (120);
            MakeAlloc<std::vector<Resource*>> (100);
            MakeAlloc<Request> (100);
        }

        SystuneSettings::metaConfigs.mDelta = 1000;
        SystuneSettings::metaConfigs.mPenaltyFactor = 2.0;
        SystuneSettings::metaConfigs.mRewardFactor = 0.4;
    }
};

// Helper methods for Resource Generation
Resource* generateResourceForTesting(int32_t seed) {
    Resource* resource = (Resource*)malloc(sizeof(Resource));
    resource->setOpCode(16 + seed);
    resource->setNumValues(1);
    resource->mConfigValue.singleValue = 2 * seed;

    return resource;
}

TEST_F(RateLimiterTests, TestClientSpammingScenario) {
    std::shared_ptr<ClientDataManager> clientDataManager = ClientDataManager::getInstance();
    std::shared_ptr<RateLimiter> rateLimiter = RateLimiter::getInstance();

    int32_t clientPID = 999;
    int32_t clientTID = 999;

    std::vector<Request*> requests;

    try {
        // Generate 51 different requests from the same client
        for(int32_t i = 0; i < 51; i++) {
            Resource* resource = (Resource*) GetBlock<Resource>();
            resource->setOpCode(16);
            resource->setNumValues(1);
            resource->mConfigValue.singleValue = 8;

            std::vector<Resource*>* resources =
                new (GetBlock<std::vector<Resource*>>())std::vector<Resource*>;
            resources->push_back(resource);

            Request* request = new (GetBlock<Request>()) Request;
            request->setRequestType(REQ_RESOURCE_TUNING);
            request->setHandle(300 + i);
            request->setDuration(-1);
            request->setPriority(REQ_PRIORITY_HIGH);
            request->setNumResources(1);
            request->setClientPID(clientPID);
            request->setClientTID(clientTID);
            request->setResources(resources);

            if(!clientDataManager->clientExists(request->getClientPID(), request->getClientTID())) {
                clientDataManager->createNewClient(request->getClientPID(), request->getClientTID());
            }

            requests.push_back(request);
        }

        // Add first 50 requests — should be accepted
        for(int32_t i = 0; i < 50; i++) {
            int8_t result = rateLimiter->isRateLimitHonored(requests[i]->getClientTID());
            ASSERT_EQ(result, true);
        }

        // Add 51st request — should be rejected
        int8_t result = rateLimiter->isRateLimitHonored(requests[50]->getClientTID());
        ASSERT_EQ(result, false);

    } catch(const std::bad_alloc& e) {
        FAIL();
    }

    clientDataManager->deleteClientPID(clientPID);
    clientDataManager->deleteClientTID(clientTID);

    // Cleanup
    for(Request* req : requests) {
        Request::cleanUpRequest(req);
    }
}

TEST_F(RateLimiterTests, TestClientHealthInCaseOfGoodRequests) {
    std::shared_ptr<ClientDataManager> clientDataManager = ClientDataManager::getInstance();
    std::shared_ptr<RateLimiter> rateLimiter = RateLimiter::getInstance();

    int32_t clientPID = 999;
    int32_t clientTID = 999;

    std::vector<Request*> requests;

    try {
        // Generate 50 different requests from the same client
        for(int32_t i = 0; i < 50; i++) {
            Resource* resource = (Resource*) GetBlock<Resource>();
            resource->setOpCode(16);
            resource->setNumValues(1);
            resource->mConfigValue.singleValue = 8;

            std::vector<Resource*>* resources =
                new (GetBlock<std::vector<Resource*>>())std::vector<Resource*>;
            resources->push_back(resource);

            Request* req = new (GetBlock<Request>()) Request;
            req->setRequestType(REQ_RESOURCE_TUNING);
            req->setHandle(300 + i);
            req->setDuration(-1);
            req->setNumResources(1);
            req->setPriority(REQ_PRIORITY_HIGH);
            req->setClientPID(clientPID);
            req->setClientTID(clientTID);
            req->setResources(resources);

            if(!clientDataManager->clientExists(req->getClientPID(), req->getClientTID())) {
                clientDataManager->createNewClient(req->getClientPID(), req->getClientTID());
            }

            requests.push_back(req);
            std::this_thread::sleep_for(std::chrono::seconds(2));

            int8_t isRateLimitHonored = rateLimiter->isRateLimitHonored(req->getClientTID());
            ASSERT_EQ(isRateLimitHonored, true);
            ASSERT_EQ(clientDataManager->getHealthByClientID(req->getClientTID()), 100);
        }

    } catch(const std::bad_alloc& e) {
        FAIL();
    }

    clientDataManager->deleteClientPID(clientPID);
    clientDataManager->deleteClientTID(clientTID);

    // Cleanup
    for(Request* req : requests) {
        Request::cleanUpRequest(req);
    }
}

TEST_F(RateLimiterTests, TestClientSpammingWithGoodRequests) {
    std::shared_ptr<ClientDataManager> clientDataManager = ClientDataManager::getInstance();
    std::shared_ptr<RateLimiter> rateLimiter = RateLimiter::getInstance();

    int32_t clientPID = 999;
    int32_t clientTID = 999;

    std::vector<Request*> requests;

    // Generate 63 different requests from the same client
    try {
        for(int32_t i = 0; i < 63; i++) {
            Resource* resource = (Resource*) GetBlock<Resource>();
            resource->setOpCode(16);
            resource->setNumValues(1);
            resource->mConfigValue.singleValue = 8;

            std::vector<Resource*>* resources =
                new (GetBlock<std::vector<Resource*>>())std::vector<Resource*>;
            resources->push_back(resource);

            Request* req = new (GetBlock<Request>()) Request;
            req->setRequestType(REQ_RESOURCE_TUNING);
            req->setHandle(300 + i);
            req->setDuration(-1);
            req->setNumResources(1);
            req->setPriority(REQ_PRIORITY_HIGH);
            req->setClientPID(clientPID);
            req->setClientTID(clientTID);
            req->setResources(resources);

            if(!clientDataManager->clientExists(req->getClientPID(), req->getClientTID())) {
                clientDataManager->createNewClient(req->getClientPID(), req->getClientTID());
            }
            requests.push_back(req);
        }

        // Add first 61 requests — should be accepted
        for(int32_t i = 0; i < 61; i++) {
            if(i % 5 == 0 && i < 50){
                std::this_thread::sleep_for(std::chrono::seconds(2));
            }
            int8_t result = rateLimiter->isRateLimitHonored(requests[i]->getClientTID());
            ASSERT_EQ(result, true);
        }

        // Add 62th request — should be rejected
        int8_t result = rateLimiter->isRateLimitHonored(requests[61]->getClientTID());
        ASSERT_EQ(result, false);

    } catch(const std::bad_alloc& e) {
        FAIL();
    }

    clientDataManager->deleteClientPID(clientPID);
    clientDataManager->deleteClientTID(clientTID);

    // Cleanup
    for(Request* req : requests) {
        Request::cleanUpRequest(req);
    }
}
