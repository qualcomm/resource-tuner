// Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
// SPDX-License-Identifier: BSD-3-Clause-Clear

#ifndef CLIENT_DATA_MANAGER_H
#define CLIENT_DATA_MANAGER_H

#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <shared_mutex>
#include <memory>
#include <mutex>
#include "string.h"
#include "unistd.h"
#include "fstream"
#include "sstream"

#include "ResourceTunerSettings.h"
#include "MemoryPool.h"
#include "Logger.h"
#include "Utils.h"

typedef struct {
    std::vector<int32_t>* mClientTIDs;
    uint8_t mClientType;
} ClientInfo;

typedef struct {
    std::unordered_set<int64_t>* mClientHandles;
    int64_t mLastRequestTimestamp;
    double mHealth;
} ClientTidData;

/**
 * @details Stores and Maintains Client Tracking Data for all the Active Clients (i.e. clients with
 *          outstanding Requests). The Data Tracked for each Client includes:
 *          - PID, and the Access Level Permissions (Third Party or System) for the Client
 *          - List of Threads Belonging to the PID
 *          - List of Requests (identified by Handle) belonging to this Client
 *          - Health and Timestamp of Last Request (Used by RateLimiter)
 *          - Essentially ClientDataManager is a central storage for Client Data, and other Components
 *            like RateLimiter, PulseMonitor and RequestManager are clients of the ClientDataManager.
 */
class ClientDataManager {
private:
    static std::shared_ptr<ClientDataManager> mClientDataManagerInstance;
    static std::mutex instanceProtectionLock;
    std::unordered_map<int32_t, ClientInfo*> mClientRepo;
    std::unordered_map<int32_t, ClientTidData*> mClientTidRepo;
    std::shared_timed_mutex mGlobalTableMutex;

    ClientDataManager();

public:
    /**
    * @brief Checks if the client with the given ID exists in the Client Data Table.
    * @param clientPID PID of the client
    * @param clientTID TID of the client
    * @return int8_t
    *            1: if the client already exists
    *            0: otherwise
    */
    int8_t clientExists(int32_t clientPID, int32_t clientTID);

    /**
    * @brief Create a new entry for the client with the given PID in the ClientData Table.
    * @details This method should only be called if the clientExists method returns 0.
    * @param clientPID PID of the client
    * @param clientTID TID of the client
    * @returns int8_t:
    *             1: Indicating that a new Client Tracking Entry was successfully Created.
    *             0: Otherwise
    */
    int8_t createNewClient(int32_t clientPID, int32_t clientTID);

    /**
    * @brief Returns a list of active requests for the client with the given PID.
    * @param clientTID Process TID of the client
    * @return std::unordered_set<int64_t>*:
    *             Pointer to an unordered_set containing the requests.
    */
    std::unordered_set<int64_t>* getRequestsByClientID(int32_t clientTID);

    /**
    * @brief This method is called by the RequestMap to insert a new Request (represented by it's handle)
    *        for the client with the given ID in the Client Data Table.
    * @param clientTID Process TID of the client
    * @param requestHandle Handle of the Request
    */
    void insertRequestByClientId(int32_t clientTID, int64_t requestHandle);

    /**
    * @brief This method is called by the RequestMap to delete a Request (represented by it's handle)
    *        for the client with the given ID in the Client Data Table.
    * @param clientTID Process TID of the client
    * @param requestHandle Handle of the Request
    */
    void deleteRequestByClientId(int32_t clientTID, int64_t requestHandle);

    /**
    * @brief This method is called by the RateLimiter to fetch the current health for a given
    *        client in the Client Data Table.
    * @param clientPID Process ID of the client
    * @return double: Health of the Client.
    */
    double getHealthByClientID(int32_t clientTID);

    /**
    * @brief This method is called by the RateLimiter to fetch the Last Request Timestamp for a given
    *        client in the Client Data Table.
    * @param clientTID TID of the client
    * @returns int64_t: Timestamp of Last Request (A value of 0, indicates no prior Requests).
    */
    int64_t getLastRequestTimestampByClientID(int32_t clientTID);

    /**
    * @brief This method is called by the RateLimiter to update the current health for a given
    *        client in the Client Data Table.
    * @param clientTID TID of the client
    * @param health  Update value of the Health for the client
    */
    void updateHealthByClientID(int32_t clientTID, double health);

    /**
    * @brief This method is called by the RateLimiter to update the Last Request Timestamp for a given
    *        client in the Client Data Table.
    * @param clientTID TID of the client
    * @param currentMillis New value for the Last Request Timestamp for the client
    */
    void updateLastRequestTimestampByClientID(int32_t clientTID, int64_t currentMillis);

    /**
    * @brief This method is called by the Verifier to fetch the Permission Level for a given
    *        client in the Client Data Table, i.e. whether the client has SYSTEM (Root) or THIRD_PARTY (User) permissions.
    * @param clientPID Process ID of the client
    * @return int8_t:
    *            PERMISSION_SYSTEM: If the client has System level access
    *            PERMISSION_THIRD_PARTY: If the Client has Third Party level access
    *            -1: If the Client could not be determined.
    */
    int8_t getClientLevelByClientID(int32_t clientPID);

    /**
    * @brief Returns the list of threads corresponding to the thread with the given ID.
    * @param clientPID Process ID of the client
    * @return std::vector<int32_t>*:
    *            Pointer to a vector containing the threads ids.
    */
    std::vector<int32_t>* getThreadsByClientId(int32_t clientPID);

    /**
    * @brief This method is called by the PulseMonitor to fetch the list of all active clients.
    * @param clientList An IN/OUT parameter to store the list of active clients.
    */
    void getActiveClientList(std::vector<int32_t>& clientList);

    /**
    * @brief Delete a client PID Entry from the Client Table.
    * @param clientPID Process ID of the client
    */
    void deleteClientPID(int32_t clientPID);

    /**
    * @brief Delete a client TID Entry from the Client TID Data Table.
    * @param clientTID TID of the client
    */
    void deleteClientTID(int32_t clientTID);

    static std::shared_ptr<ClientDataManager> getInstance() {
        if(mClientDataManagerInstance == nullptr) {
            instanceProtectionLock.lock();
            if(mClientDataManagerInstance == nullptr) {
                try {
                    mClientDataManagerInstance = std::shared_ptr<ClientDataManager> (new ClientDataManager());
                } catch(const std::bad_alloc& e) {
                    instanceProtectionLock.unlock();
                    return nullptr;
                }
            }
            instanceProtectionLock.unlock();
        }
        return mClientDataManagerInstance;
    }
};

#endif
