// #include <gtest/gtest.h>
// #include <thread>
// #include "RequestManager.h"
// #include "RateLimiter.h"

// class RateLimiterTests: public::testing::Test {
// protected:
//     void SetUp() override {
//         static int8_t firstTest = true;
//         if(firstTest == true) {
//             firstTest = false;
//             PoolAllocate<ClientInfo> (30);
//             PoolAllocate<ClientTidData> (30);
//             PoolAllocate<std::unordered_set<int64_t>> (30);
//             PoolAllocate<std::vector<int32_t>> (30);
//         }

//         SystuneSettings::metaConfigs.mDelta = 1000;
//         SystuneSettings::metaConfigs.mPenaltyFactor = 2.0;
//         SystuneSettings::metaConfigs.mRewardFactor = 0.4;
//     }
// };

// // Helper methods for Resource Generation
// Resource* generateResourceForTesting(int32_t seed) {
//     Resource* resource = (Resource*)malloc(sizeof(Resource));
//     resource->mOpId = 16 + seed;
//     resource->mOpInfo = 27 + 3 * seed;
//     resource->mOptionalInfo = 1445 + 8 * seed;
//     resource->mNumValues = 1;
//     resource->mConfigValue.singleValue = 2 * seed;

//     return resource;
// }

// TEST_F(RateLimiterTests, TestClientSpammingScenario) {
//     std::shared_ptr<ClientDataManager> clientDataManager = ClientDataManager::getInstance();
//     std::shared_ptr<RateLimiter> rateLimiter = RateLimiter::getInstance();

//     int32_t clientPID = 999;
//     int32_t clientTID = 999;

//     std::vector<Request*> requests;

//     // Generate 50 different requests from the same client
//     for(int32_t i = 0; i < 50; i++) {
//         std::vector<Resource*>* resources = new std::vector<Resource*>;
//         resources->push_back(generateResourceForTesting(i + 1));

//         Request* req = new Request(REQ_RESOURCE_TUNING, 300 + i, 50, 1,  1, clientPID, clientTID, resources);
//         if(!clientDataManager->clientExists(req->getClientPID(), req->getClientTID())) {
//             clientDataManager->createNewClient(req->getClientPID(), req->getClientTID());
//         }

//         requests.push_back(req);
//     }

//     // Add first 49 requests — should be accepted
//     for(int32_t i = 0; i < 49; i++) {
//         int8_t result = rateLimiter->isRateLimitHonored(requests[i]->getClientTID());
//         ASSERT_EQ(result, true);
//     }

//     // Add 50th request — should be rejected
//     int8_t result = rateLimiter->isRateLimitHonored(requests[49]->getClientTID());
//     ASSERT_EQ(result, false);

//     clientDataManager->deleteClientPID(clientPID);
//     clientDataManager->deleteClientTID(clientTID);

//     // Cleanup
//     for(Request* req : requests) {
//         std::vector<Resource*> reqResources = *(req->getResources());
//         for(int32_t i = 0; i < reqResources.size(); i++) {
//             free(reqResources[i]);
//         }
//         delete(req->getResources());
//         delete(req);
//     }
// }

// TEST_F(RateLimiterTests, TestClientHealthInCaseOfGoodRequests) {
//     std::shared_ptr<ClientDataManager> clientDataManager = ClientDataManager::getInstance();
//     std::shared_ptr<RateLimiter> rateLimiter = RateLimiter::getInstance();

//     int32_t clientPID = 999;
//     int32_t clientTID = 999;

//     std::vector<Request*> requests;

//     // Generate 50 different requests from the same client
//     for(int32_t i = 0; i < 50; i++) {
//         std::vector<Resource*>* resources = new std::vector<Resource*>;
//         resources->push_back(generateResourceForTesting(i + 1));

//         Request* req = new Request(REQ_RESOURCE_TUNING, 300 + i, 50, 1, 1, clientPID, clientTID, resources);
//         if(!clientDataManager->clientExists(req->getClientPID(), req->getClientTID())) {
//             clientDataManager->createNewClient(req->getClientPID(), req->getClientTID());
//         }

//         requests.push_back(req);
//         std::this_thread::sleep_for(std::chrono::seconds(2));

//         ASSERT_EQ(clientDataManager->getHealthByClientID(req->getClientTID()), 100);
//     }

//     clientDataManager->deleteClientPID(clientPID);
//     clientDataManager->deleteClientTID(clientTID);

//     // Cleanup
//     for(Request* req : requests) {
//         std::vector<Resource*> reqResources = *(req->getResources());
//         for(int32_t i = 0; i < reqResources.size(); i++) {
//             free(reqResources[i]);
//         }
//         delete(req->getResources());
//         delete(req);
//     }
// }

// TEST_F(RateLimiterTests, TestClientSpammingWithGoodRequests) {
//     std::shared_ptr<ClientDataManager> clientDataManager = ClientDataManager::getInstance();
//     std::shared_ptr<RateLimiter> rateLimiter = RateLimiter::getInstance();

//     int32_t clientPID = 999;
//     int32_t clientTID = 999;

//     std::vector<Request*> requests;

//     // Generate 63 different requests from the same client
//     for(int32_t i = 0; i < 63; i++) {
//         std::vector<Resource*>* resources = new std::vector<Resource*>;
//         resources->push_back(generateResourceForTesting(i + 1));
//         Request* req = new Request(REQ_RESOURCE_TUNING, 300 + i, 50, 1, 1, clientPID, clientTID, resources);

//         if(!clientDataManager->clientExists(req->getClientPID(), req->getClientTID())) {
//             clientDataManager->createNewClient(req->getClientPID(), req->getClientTID());
//         }
//         requests.push_back(req);
//     }

//     // Add first 61 requests — should be accepted
//     for(int32_t i = 0; i < 61; i++) {
//         if(i % 5 == 0 && i < 50){
//             std::this_thread::sleep_for(std::chrono::seconds(2));
//         }
//         int8_t result = rateLimiter->isRateLimitHonored(requests[i]->getClientTID());
//         ASSERT_EQ(result, true);
//     }

//     // Add 62th request — should be rejected
//     int8_t result = rateLimiter->isRateLimitHonored(requests[61]->getClientTID());
//     ASSERT_EQ(result, false);

//     clientDataManager->deleteClientPID(clientPID);
//     clientDataManager->deleteClientTID(clientTID);

//     // Cleanup
//     for(Request* req : requests) {
//         std::vector<Resource*> reqResources = *(req->getResources());
//         for(int32_t i = 0; i < reqResources.size(); i++) {
//             free(reqResources[i]);
//         }
//         delete(req->getResources());
//         delete(req);
//     }
// }
