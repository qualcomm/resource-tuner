// Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
// SPDX-License-Identifier: BSD-3-Clause-Clear

#include <thread>
#include <cstdint>

#include "TestUtils.h"
#include "AuxRoutines.h"
#include "ClientDataManager.h"

static void Init() {
    MakeAlloc<ClientInfo> (30);
    MakeAlloc<ClientTidData> (30);
    MakeAlloc<std::unordered_set<int64_t>> (30);
    MakeAlloc<std::vector<int32_t>> (30);
}

static void TestClientDataManagerClientEntryCreation1() {
    std::shared_ptr<ClientDataManager> clientDataManager = ClientDataManager::getInstance();

    int32_t testClientPID = 252;
    int32_t testClientTID = 252;

    C_ASSERT(clientDataManager->clientExists(testClientPID, testClientTID) == false);
    clientDataManager->createNewClient(testClientPID, testClientTID);
    C_ASSERT(clientDataManager->clientExists(testClientPID, testClientTID) == true);

    clientDataManager->deleteClientPID(testClientPID);
    clientDataManager->deleteClientTID(testClientTID);
}

// Use threads to simulate different clients (PIDs essentially)
static void TestClientDataManagerClientEntryCreation2() {
    std::shared_ptr<ClientDataManager> clientDataManager = ClientDataManager::getInstance();

    std::vector<std::thread> threads;
    std::vector<int32_t> clientPIDs;

    for(int32_t i = 0; i < 10; i++) {
        auto threadRoutine = [&] (void* arg) {
            int32_t id = *(int32_t*) arg;

            C_ASSERT(clientDataManager->clientExists(id, id) == false);
            clientDataManager->createNewClient(id, id);
            C_ASSERT(clientDataManager->clientExists(id, id) == true);

            free(arg);
        };

        int32_t* ptr = (int32_t*) malloc(sizeof(int32_t));
        *ptr = i + 1;

        threads.push_back(std::thread(threadRoutine, ptr));
        clientPIDs.push_back(i + 1);
    }

    for(int32_t i = 0; i < threads.size(); i++) {
        threads[i].join();
    }

    for(int32_t clientID: clientPIDs) {
        clientDataManager->deleteClientPID(clientID);
        clientDataManager->deleteClientTID(clientID);
    }
}

static void TestClientDataManagerClientEntryDeletion() {
    std::shared_ptr<ClientDataManager> clientDataManager = ClientDataManager::getInstance();

    int32_t testClientPID = 252;
    int32_t testClientTID = 252;

    C_ASSERT(clientDataManager->clientExists(testClientPID, testClientTID) == false);
    clientDataManager->createNewClient(testClientPID, testClientTID);
    C_ASSERT(clientDataManager->clientExists(testClientPID, testClientTID) == true);

    clientDataManager->deleteClientPID(testClientPID);
    clientDataManager->deleteClientTID(testClientTID);
    C_ASSERT(clientDataManager->clientExists(testClientPID, testClientTID) == false);
}

static void TestClientDataManagerRateLimiterUtilsHealth() {
    std::shared_ptr<ClientDataManager> clientDataManager = ClientDataManager::getInstance();

    int32_t testClientPID = 252;
    int32_t testClientTID = 252;

    C_ASSERT(clientDataManager->clientExists(testClientPID, testClientTID) == false);
    clientDataManager->createNewClient(testClientPID, testClientTID);
    C_ASSERT(clientDataManager->clientExists(testClientPID, testClientTID) == true);

    double health = clientDataManager->getHealthByClientID(testClientTID);
    C_ASSERT(health == 100.0);

    clientDataManager->deleteClientPID(testClientPID);
    clientDataManager->deleteClientTID(testClientTID);
}

static void TestClientDataManagerRateLimiterUtilsHealthSetGet() {
    std::shared_ptr<ClientDataManager> clientDataManager = ClientDataManager::getInstance();

    int32_t testClientPID = 252;
    int32_t testClientTID = 252;

    C_ASSERT(clientDataManager->clientExists(testClientPID, testClientTID) == false);
    clientDataManager->createNewClient(testClientPID, testClientTID);
    C_ASSERT(clientDataManager->clientExists(testClientPID, testClientTID) == true);

    clientDataManager->updateHealthByClientID(testClientTID, 55);
    double health = clientDataManager->getHealthByClientID(testClientTID);

    C_ASSERT(health == 55.0);

    clientDataManager->deleteClientPID(testClientPID);
    clientDataManager->deleteClientTID(testClientTID);
}

static void TestClientDataManagerRateLimiterUtilsLastRequestTimestampSetGet() {
    std::shared_ptr<ClientDataManager> clientDataManager = ClientDataManager::getInstance();

    int32_t testClientPID = 252;
    int32_t testClientTID = 252;

    C_ASSERT(clientDataManager->clientExists(testClientPID, testClientTID) == false);
    clientDataManager->createNewClient(testClientPID, testClientTID);
    C_ASSERT(clientDataManager->clientExists(testClientPID, testClientTID) == true);

    int64_t currentMillis = AuxRoutines::getCurrentTimeInMilliseconds();

    clientDataManager->updateLastRequestTimestampByClientID(testClientTID, currentMillis);
    int64_t lastRequestTimestamp = clientDataManager->getLastRequestTimestampByClientID(testClientTID);

    C_ASSERT(lastRequestTimestamp == currentMillis);

    clientDataManager->deleteClientPID(testClientPID);
    clientDataManager->deleteClientTID(testClientTID);
}

static void TestClientDataManagerPulseMonitorClientListFetch() {
    std::shared_ptr<ClientDataManager> clientDataManager = ClientDataManager::getInstance();

    // Insert a few clients into the table
    for(int32_t i = 100; i < 120; i++) {
        C_ASSERT(clientDataManager->clientExists(i, i) == false);
        clientDataManager->createNewClient(i, i);
        C_ASSERT(clientDataManager->clientExists(i, i) == true);
    }

    std::vector<int32_t> clientList;
    clientDataManager->getActiveClientList(clientList);

    C_ASSERT(clientList.size() == 20);
    for(int32_t clientPID: clientList) {
        C_ASSERT(clientPID < 120);
        C_ASSERT(clientPID >= 100);
    }

    for(int32_t i = 100; i < 120; i++) {
        clientDataManager->deleteClientPID(i);
        clientDataManager->deleteClientTID(i);
    }
}

static void TestClientDataManagerRequestMapInsertion() {
    std::shared_ptr<ClientDataManager> clientDataManager = ClientDataManager::getInstance();

    int32_t testClientPID = 252;
    int32_t testClientTID = 252;

    C_ASSERT(clientDataManager->clientExists(testClientPID, testClientTID) == false);
    clientDataManager->createNewClient(testClientPID, testClientTID);
    C_ASSERT(clientDataManager->clientExists(testClientPID, testClientTID) == true);

    for(int32_t i = 0; i < 20; i++) {
        clientDataManager->insertRequestByClientId(testClientTID, i + 1);
    }

    std::unordered_set<int64_t>* clientRequests =
                    clientDataManager->getRequestsByClientID(testClientTID);

    C_ASSERT(clientRequests != nullptr);
    C_ASSERT(clientRequests->size() == 20);

    for(int32_t i = 0; i < 20; i++) {
        clientDataManager->deleteRequestByClientId(testClientTID, i + 1);
    }

    clientDataManager->deleteClientPID(testClientPID);
    clientDataManager->deleteClientTID(testClientTID);

    C_ASSERT(clientRequests->size() == 0);
}

static void TestClientDataManagerClientThreadTracking1() {
    std::shared_ptr<ClientDataManager> clientDataManager = ClientDataManager::getInstance();

    int32_t testClientPID = 252;
    std::vector<std::thread> clientThreads;

    for(int32_t i = 0; i < 20; i++) {
        auto threadRoutine = [&](void* arg) {
            int32_t threadID = *(int32_t*)arg;
            C_ASSERT(clientDataManager->clientExists(testClientPID, threadID) == false);
            clientDataManager->createNewClient(testClientPID, threadID);
            C_ASSERT(clientDataManager->clientExists(testClientPID, threadID) == true);

            free(arg);
        };

        int32_t* index = (int32_t*)malloc(sizeof(int32_t));
        *index = i + 1;
        clientThreads.push_back(std::thread(threadRoutine, index));
    }

    for(int32_t i = 0; i < clientThreads.size(); i++) {
        clientThreads[i].join();
    }

    std::vector<int32_t>* threadIds = clientDataManager->getThreadsByClientId(testClientPID);
    C_ASSERT(threadIds != nullptr);
    C_ASSERT(threadIds->size() == 20);

    clientDataManager->deleteClientPID(testClientPID);

    for(int32_t i = 0; i < 20; i++) {
        clientDataManager->deleteClientTID(i + 1);
    }
}

static void TestClientDataManagerClientThreadTracking2() {
    std::shared_ptr<ClientDataManager> clientDataManager = ClientDataManager::getInstance();

    int32_t testClientPID = 252;

    for(int32_t i = 0; i < 20; i++) {
        C_ASSERT(clientDataManager->clientExists(testClientPID, i + 1) == false);
        clientDataManager->createNewClient(testClientPID, i + 1);
        C_ASSERT(clientDataManager->clientExists(testClientPID, i + 1) == true);
    }

    std::vector<int32_t>* threadIds = clientDataManager->getThreadsByClientId(testClientPID);
    C_ASSERT(threadIds != nullptr);
    C_ASSERT(threadIds->size() == 20);

    for(int32_t i = 0; i < 20; i++) {
        clientDataManager->insertRequestByClientId(i + 1, 5 * i + 7);
    }

    for(int32_t i = 0; i < 20; i++) {
        std::unordered_set<int64_t>* clientRequests =
                    clientDataManager->getRequestsByClientID(i + 1);

        C_ASSERT(clientRequests != nullptr);
        C_ASSERT(clientRequests->size() == 1);

        clientDataManager->deleteRequestByClientId(i + 1, 5 * i + 7);
    }

    for(int32_t i = 0; i < 20; i++) {
        std::unordered_set<int64_t>* clientRequests =
                clientDataManager->getRequestsByClientID(i + 1);

        C_ASSERT(clientRequests != nullptr);
        C_ASSERT(clientRequests->size() == 0);
    }

    clientDataManager->deleteClientPID(testClientPID);

    for(int32_t i = 0; i < 20; i++) {
        clientDataManager->deleteClientTID(i + 1);
    }
}

int32_t main() {
    std::cout<<"Running Test Suite: [ClientDataManager Tests]\n"<<std::endl;
    Init();

    RUN_TEST(TestClientDataManagerClientEntryCreation1);
    RUN_TEST(TestClientDataManagerClientEntryCreation2);
    RUN_TEST(TestClientDataManagerClientEntryDeletion);
    RUN_TEST(TestClientDataManagerRateLimiterUtilsHealth);
    RUN_TEST(TestClientDataManagerRateLimiterUtilsHealthSetGet);
    RUN_TEST(TestClientDataManagerRateLimiterUtilsLastRequestTimestampSetGet);
    RUN_TEST(TestClientDataManagerPulseMonitorClientListFetch);
    RUN_TEST(TestClientDataManagerRequestMapInsertion);
    RUN_TEST(TestClientDataManagerClientThreadTracking1);
    RUN_TEST(TestClientDataManagerClientThreadTracking2);

    std::cout<<"\nAll Tests from the suite: [ClientDataManager Tests], executed successfully"<<std::endl;
    return 0;
}
