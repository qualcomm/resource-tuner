#include <thread>
#include <cstdint>
#include <gtest/gtest.h>

#include "ClientDataManager.h"

class ClientDataManagerTests: public::testing::Test {
protected:
    void SetUp() override {
        static int8_t firstTest = true;
        if(firstTest) {
            firstTest = false;
            MakeAlloc<ClientInfo> (30);
            MakeAlloc<ClientTidData> (30);
            MakeAlloc<std::unordered_set<int64_t>> (30);
            MakeAlloc<std::vector<int32_t>> (30);
        }
    }
};

TEST_F(ClientDataManagerTests, TestClientDataManagerClientEntryCreation1) {
    std::shared_ptr<ClientDataManager> clientDataManager = ClientDataManager::getInstance();

    int32_t testClientPID = 252;
    int32_t testClientTID = 252;

    ASSERT_EQ(clientDataManager->clientExists(testClientPID, testClientTID), false);
    clientDataManager->createNewClient(testClientPID, testClientTID);
    ASSERT_EQ(clientDataManager->clientExists(testClientPID, testClientTID), true);

    clientDataManager->deleteClientPID(testClientPID);
    clientDataManager->deleteClientTID(testClientTID);
}

// Use threads to simulate different clients (PIDs essentially)
TEST_F(ClientDataManagerTests, TestClientDataManagerClientEntryCreation2) {
    std::shared_ptr<ClientDataManager> clientDataManager = ClientDataManager::getInstance();

    std::vector<std::thread> threads;
    std::vector<int32_t> clientPIDs;

    for(int32_t i = 0; i < 10; i++) {
        auto threadRoutine = [&] (void* arg) {
            int32_t id = *(int32_t*) arg;

            ASSERT_EQ(clientDataManager->clientExists(id, id), false);
            clientDataManager->createNewClient(id, id);
            ASSERT_EQ(clientDataManager->clientExists(id, id), true);

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

TEST_F(ClientDataManagerTests, TestClientDataManagerClientEntryDeletion) {
    std::shared_ptr<ClientDataManager> clientDataManager = ClientDataManager::getInstance();

    int32_t testClientPID = 252;
    int32_t testClientTID = 252;

    ASSERT_EQ(clientDataManager->clientExists(testClientPID, testClientTID), false);
    clientDataManager->createNewClient(testClientPID, testClientTID);
    ASSERT_EQ(clientDataManager->clientExists(testClientPID, testClientTID), true);

    clientDataManager->deleteClientPID(testClientPID);
    clientDataManager->deleteClientTID(testClientTID);
    ASSERT_EQ(clientDataManager->clientExists(testClientPID, testClientTID), false);
}

TEST_F(ClientDataManagerTests, TestClientDataManagerRateLimiterUtilsHealth) {
    std::shared_ptr<ClientDataManager> clientDataManager = ClientDataManager::getInstance();

    int32_t testClientPID = 252;
    int32_t testClientTID = 252;

    ASSERT_EQ(clientDataManager->clientExists(testClientPID, testClientTID), false);
    clientDataManager->createNewClient(testClientPID, testClientTID);
    ASSERT_EQ(clientDataManager->clientExists(testClientPID, testClientTID), true);

    double health = clientDataManager->getHealthByClientID(testClientTID);
    ASSERT_EQ(health, 100.0);

    clientDataManager->deleteClientPID(testClientPID);
    clientDataManager->deleteClientTID(testClientTID);
}

TEST_F(ClientDataManagerTests, TestClientDataManagerRateLimiterUtilsHealthSetGet) {
    std::shared_ptr<ClientDataManager> clientDataManager = ClientDataManager::getInstance();

    int32_t testClientPID = 252;
    int32_t testClientTID = 252;

    ASSERT_EQ(clientDataManager->clientExists(testClientPID, testClientTID), false);
    clientDataManager->createNewClient(testClientPID, testClientTID);
    ASSERT_EQ(clientDataManager->clientExists(testClientPID, testClientTID), true);

    clientDataManager->updateHealthByClientID(testClientTID, 55);
    double health = clientDataManager->getHealthByClientID(testClientTID);

    ASSERT_EQ(health, 55.0);

    clientDataManager->deleteClientPID(testClientPID);
    clientDataManager->deleteClientTID(testClientTID);
}

TEST_F(ClientDataManagerTests, TestClientDataManagerRateLimiterUtilsLastRequestTimestampSetGet) {
    std::shared_ptr<ClientDataManager> clientDataManager = ClientDataManager::getInstance();

    int32_t testClientPID = 252;
    int32_t testClientTID = 252;

    ASSERT_EQ(clientDataManager->clientExists(testClientPID, testClientTID), false);
    clientDataManager->createNewClient(testClientPID, testClientTID);
    ASSERT_EQ(clientDataManager->clientExists(testClientPID, testClientTID), true);

    int64_t currentMillis = SystuneSettings::getCurrentTimeInMilliseconds();

    clientDataManager->updateLastRequestTimestampByClientID(testClientTID, currentMillis);
    int64_t lastRequestTimestamp = clientDataManager->getLastRequestTimestampByClientID(testClientTID);

    ASSERT_EQ(lastRequestTimestamp, currentMillis);

    clientDataManager->deleteClientPID(testClientPID);
    clientDataManager->deleteClientTID(testClientTID);
}

TEST_F(ClientDataManagerTests, TestClientDataManagerPulseMonitorClientListFetch) {
    std::shared_ptr<ClientDataManager> clientDataManager = ClientDataManager::getInstance();

    // Insert a few clients into the table
    for(int32_t i = 100; i < 120; i++) {
        ASSERT_EQ(clientDataManager->clientExists(i, i), false);
        clientDataManager->createNewClient(i, i);
        ASSERT_EQ(clientDataManager->clientExists(i, i), true);
    }

    std::vector<int32_t> clientList;
    clientDataManager->getActiveClientList(clientList);

    ASSERT_EQ(clientList.size(), 20);
    for(int32_t clientPID: clientList) {
        ASSERT_LT(clientPID, 120);
        ASSERT_GE(clientPID, 100);
    }

    for(int32_t i = 100; i < 120; i++) {
        clientDataManager->deleteClientPID(i);
        clientDataManager->deleteClientTID(i);
    }
}

TEST_F(ClientDataManagerTests, TestClientDataManagerRequestMapInsertion) {
    std::shared_ptr<ClientDataManager> clientDataManager = ClientDataManager::getInstance();

    int32_t testClientPID = 252;
    int32_t testClientTID = 252;

    ASSERT_EQ(clientDataManager->clientExists(testClientPID, testClientTID), false);
    clientDataManager->createNewClient(testClientPID, testClientTID);
    ASSERT_EQ(clientDataManager->clientExists(testClientPID, testClientTID), true);

    for(int32_t i = 0; i < 20; i++) {
        clientDataManager->insertRequestByClientId(testClientTID, i + 1);
    }

    std::unordered_set<int64_t>* clientRequests =
                    clientDataManager->getRequestsByClientID(testClientTID);

    ASSERT_NE(clientRequests, nullptr);
    ASSERT_EQ(clientRequests->size(), 20);

    for(int32_t i = 0; i < 20; i++) {
        clientDataManager->deleteRequestByClientId(testClientTID, i + 1);
    }

    clientDataManager->deleteClientPID(testClientPID);
    clientDataManager->deleteClientTID(testClientTID);

    ASSERT_EQ(clientRequests->size(), 0);
}

TEST_F(ClientDataManagerTests, TestClientDataManagerClientThreadTracking1) {
    std::shared_ptr<ClientDataManager> clientDataManager = ClientDataManager::getInstance();

    int32_t testClientPID = 252;
    std::vector<std::thread> clientThreads;

    for(int32_t i = 0; i < 20; i++) {
        auto threadRoutine = [&](void* arg) {
            int32_t threadID = *(int32_t*)arg;
            ASSERT_EQ(clientDataManager->clientExists(testClientPID, threadID), false);
            clientDataManager->createNewClient(testClientPID, threadID);
            ASSERT_EQ(clientDataManager->clientExists(testClientPID, threadID), true);

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
    ASSERT_NE(threadIds, nullptr);
    ASSERT_EQ(threadIds->size(), 20);

    clientDataManager->deleteClientPID(testClientPID);

    for(int32_t i = 0; i < 20; i++) {
        clientDataManager->deleteClientTID(i + 1);
    }
}

TEST_F(ClientDataManagerTests, TestClientDataManagerClientThreadTracking2) {
    std::shared_ptr<ClientDataManager> clientDataManager = ClientDataManager::getInstance();

    int32_t testClientPID = 252;

    for(int32_t i = 0; i < 20; i++) {
        ASSERT_EQ(clientDataManager->clientExists(testClientPID, i + 1), false);
        clientDataManager->createNewClient(testClientPID, i + 1);
        ASSERT_EQ(clientDataManager->clientExists(testClientPID, i + 1), true);
    }

    std::vector<int32_t>* threadIds = clientDataManager->getThreadsByClientId(testClientPID);
    ASSERT_NE(threadIds, nullptr);
    ASSERT_EQ(threadIds->size(), 20);

    for(int32_t i = 0; i < 20; i++) {
        clientDataManager->insertRequestByClientId(i + 1, 5 * i + 7);
    }

    for(int32_t i = 0; i < 20; i++) {
        std::unordered_set<int64_t>* clientRequests =
                    clientDataManager->getRequestsByClientID(i + 1);

        ASSERT_NE(clientRequests, nullptr);
        ASSERT_EQ(clientRequests->size(), 1);

        clientDataManager->deleteRequestByClientId(i + 1, 5 * i + 7);
    }

    for(int32_t i = 0; i < 20; i++) {
        std::unordered_set<int64_t>* clientRequests =
                clientDataManager->getRequestsByClientID(i + 1);

        ASSERT_NE(clientRequests, nullptr);
        ASSERT_EQ(clientRequests->size(), 0);
    }

    clientDataManager->deleteClientPID(testClientPID);

    for(int32_t i = 0; i < 20; i++) {
        clientDataManager->deleteClientTID(i + 1);
    }
}
