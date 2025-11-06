// Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
// SPDX-License-Identifier: BSD-3-Clause-Clear

#include "TestUtils.h"
#include "RequestManager.h"
#include "RateLimiter.h"

static void Init() {
    MakeAlloc<ClientInfo> (30);
    MakeAlloc<ClientTidData> (30);
    MakeAlloc<std::unordered_set<int64_t>> (30);
    MakeAlloc<std::vector<int32_t>> (30);
    MakeAlloc<Resource> (120);
    MakeAlloc<Request> (100);

    ResourceTunerSettings::metaConfigs.mDelta = 1000;
    ResourceTunerSettings::metaConfigs.mPenaltyFactor = 2.0;
    ResourceTunerSettings::metaConfigs.mRewardFactor = 0.4;
}

// Helper methods for Resource Generation
Resource* generateResourceForTesting(int32_t seed) {
    Resource* resource = (Resource*)malloc(sizeof(Resource));
    resource->setResCode(16 + seed);
    resource->setNumValues(1);
    resource->mResValue.value = 2 * seed;

    return resource;
}

static void TestClientSpammingScenario() {
    std::shared_ptr<ClientDataManager> clientDataManager = ClientDataManager::getInstance();
    std::shared_ptr<RateLimiter> rateLimiter = RateLimiter::getInstance();

    int32_t clientPID = 999;
    int32_t clientTID = 999;

    std::vector<Request*> requests;

    try {
        // Generate 51 different requests from the same client
        for(int32_t i = 0; i < 51; i++) {
            Request* request = new (GetBlock<Request>()) Request;
            request->setRequestType(REQ_RESOURCE_TUNING);
            request->setHandle(300 + i);
            request->setDuration(-1);
            request->setPriority(REQ_PRIORITY_HIGH);
            request->setClientPID(clientPID);
            request->setClientTID(clientTID);

            if(!clientDataManager->clientExists(request->getClientPID(), request->getClientTID())) {
                clientDataManager->createNewClient(request->getClientPID(), request->getClientTID());
            }

            requests.push_back(request);
        }

        // Add first 50 requests — should be accepted
        for(int32_t i = 0; i < 50; i++) {
            int8_t result = rateLimiter->isRateLimitHonored(requests[i]->getClientTID());
            C_ASSERT(result == true);
        }

        // Add 51st request — should be rejected
        int8_t result = rateLimiter->isRateLimitHonored(requests[50]->getClientTID());
        C_ASSERT(result == false);

    } catch(const std::bad_alloc& e) {}

    clientDataManager->deleteClientPID(clientPID);
    clientDataManager->deleteClientTID(clientTID);

    // Cleanup
    for(Request* req : requests) {
        Request::cleanUpRequest(req);
    }
}

static void TestClientHealthInCaseOfGoodRequests() {
    std::shared_ptr<ClientDataManager> clientDataManager = ClientDataManager::getInstance();
    std::shared_ptr<RateLimiter> rateLimiter = RateLimiter::getInstance();

    int32_t clientPID = 999;
    int32_t clientTID = 999;

    std::vector<Request*> requests;

    try {
        // Generate 50 different requests from the same client
        for(int32_t i = 0; i < 50; i++) {
            Request* req = new (GetBlock<Request>()) Request;
            req->setRequestType(REQ_RESOURCE_TUNING);
            req->setHandle(300 + i);
            req->setDuration(-1);
            req->setPriority(REQ_PRIORITY_HIGH);
            req->setClientPID(clientPID);
            req->setClientTID(clientTID);

            if(!clientDataManager->clientExists(req->getClientPID(), req->getClientTID())) {
                clientDataManager->createNewClient(req->getClientPID(), req->getClientTID());
            }

            requests.push_back(req);
            std::this_thread::sleep_for(std::chrono::seconds(2));

            int8_t isRateLimitHonored = rateLimiter->isRateLimitHonored(req->getClientTID());
            C_ASSERT(isRateLimitHonored == true);
            C_ASSERT(clientDataManager->getHealthByClientID(req->getClientTID()) == 100);
        }

    } catch(const std::bad_alloc& e) {}

    clientDataManager->deleteClientPID(clientPID);
    clientDataManager->deleteClientTID(clientTID);

    // Cleanup
    for(Request* req : requests) {
        Request::cleanUpRequest(req);
    }
}

static void TestClientSpammingWithGoodRequests() {
    std::shared_ptr<ClientDataManager> clientDataManager = ClientDataManager::getInstance();
    std::shared_ptr<RateLimiter> rateLimiter = RateLimiter::getInstance();

    int32_t clientPID = 999;
    int32_t clientTID = 999;

    std::vector<Request*> requests;

    // Generate 63 different requests from the same client
    try {
        for(int32_t i = 0; i < 63; i++) {
            Request* req = new (GetBlock<Request>()) Request;
            req->setRequestType(REQ_RESOURCE_TUNING);
            req->setHandle(300 + i);
            req->setDuration(-1);
            req->setPriority(REQ_PRIORITY_HIGH);
            req->setClientPID(clientPID);
            req->setClientTID(clientTID);

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
            C_ASSERT(result == true);
        }

        // Add 62th request — should be rejected
        int8_t result = rateLimiter->isRateLimitHonored(requests[61]->getClientTID());
        C_ASSERT(result == false);

    } catch(const std::bad_alloc& e) {}

    clientDataManager->deleteClientPID(clientPID);
    clientDataManager->deleteClientTID(clientTID);

    // Cleanup
    for(Request* req : requests) {
        Request::cleanUpRequest(req);
    }
}

int32_t main() {
    std::cout<<"Running Test Suite: [RateLimiterTests]\n"<<std::endl;

    Init();
    RUN_TEST(TestClientSpammingScenario);
    RUN_TEST(TestClientHealthInCaseOfGoodRequests);
    RUN_TEST(TestClientSpammingWithGoodRequests);

    std::cout<<"\nAll Tests from the suite: [RateLimiterTests], executed successfully"<<std::endl;
    return 0;
}
