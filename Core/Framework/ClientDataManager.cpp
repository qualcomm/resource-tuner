// Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
// SPDX-License-Identifier: BSD-3-Clause-Clear

#include "ClientDataManager.h"
#include "ErrCodes.h"

static int8_t isRootProcess(pid_t pid) {
    std::string statusFile = "/proc/" + std::to_string(pid) + "/status";
    std::ifstream file(statusFile);
    std::string line;

    if(!file.is_open()) {
        LOGE("RESTUNE_CLIENT_DATA_MANAGER", "Failed to open file: " + fileName + ", Error: " + strerror(errno));
        return -1;
    }

    while(std::getline(file, line)) {
        if(line.rfind("Uid:", 0) == 0) {
            // Format: Uid: real effective saved fs
            std::istringstream iss(line);
            std::string label;
            int32_t real, effective, saved, fs;
            iss >> label >> real >> effective >> saved >> fs;

            if(effective == 0) {
                return PERMISSION_SYSTEM;
            } else {
                return PERMISSION_THIRD_PARTY;
            }
        }
    }
    return PERMISSION_THIRD_PARTY;
}

std::mutex ClientDataManager::instanceProtectionLock {};
std::shared_ptr<ClientDataManager> ClientDataManager::mClientDataManagerInstance = nullptr;
ClientDataManager::ClientDataManager() {}

int8_t ClientDataManager::clientExists(int32_t clientPID, int32_t clientTID) {
    this->mGlobalTableMutex.lock_shared();

    // Check that an entry corresponding to the client PID exists in the mClientRepo table, and
    // An entry for the client TID exists in the mClientTidRepo table.
    int8_t clientCheck = (this->mClientRepo.find(clientPID) != this->mClientRepo.end()) &&
                         (this->mClientTidRepo.find(clientTID) != this->mClientTidRepo.end());

    this->mGlobalTableMutex.unlock_shared();
    return clientCheck;
}

int8_t ClientDataManager::createNewClient(int32_t clientPID, int32_t clientTID) {
    this->mGlobalTableMutex.lock();
    // First create an entry in the mClientTidRepo table

    int8_t clientCheck = (this->mClientRepo.find(clientPID) != this->mClientRepo.end()) &&
                         (this->mClientTidRepo.find(clientTID) != this->mClientTidRepo.end());

    if(clientCheck) {
        // Edge Case, control should not reach here since it is expected that createNewClient
        // Routine is used in conjunction with the clientExists Routine
        this->mGlobalTableMutex.unlock();
        return true;
    }

    ClientTidData* clientData = nullptr;
    try {
        clientData = new (GetBlock<ClientTidData>()) ClientTidData;
        clientData->mLastRequestTimestamp = 0;
        clientData->mHealth = 100.0;
        clientData->mClientHandles =
            new (GetBlock<std::unordered_set<int64_t>>()) std::unordered_set<int64_t>;

    } catch(const std::bad_alloc& e) {
        TYPELOGV(CLIENT_ALLOCATION_FAILURE, clientPID, clientTID, e.what());

        this->mGlobalTableMutex.unlock();
        return false;

    } catch(const std::exception& e) {
        TYPELOGV(CLIENT_ALLOCATION_FAILURE, clientPID, clientTID, e.what());

        this->mGlobalTableMutex.unlock();
        return false;
    }

    // Check if a client PID entry exists
    int8_t clientPIDExists = (this->mClientRepo.find(clientPID) != this->mClientRepo.end());

    this->mClientTidRepo[clientTID] = clientData;
    if(clientPIDExists) {
        // If it does, then add the client TID to the list of TIDs for that client PID
        this->mClientRepo[clientPID]->mClientTIDs->push_back(clientTID);
    } else {
        // If it doesn't, then create a new entry in the mClientRepo table
        try {
            ClientInfo* clientInfo = new (GetBlock<ClientInfo>()) ClientInfo;
            clientInfo->mClientTIDs =
                new (GetBlock<std::vector<int32_t>>())
                std::vector<int32_t>;

            clientInfo->mClientTIDs->push_back(clientTID);
            clientInfo->mClientType = isRootProcess(clientPID);
            this->mClientRepo[clientPID] = clientInfo;

        } catch(const std::bad_alloc& e) {
            TYPELOGV(CLIENT_ALLOCATION_FAILURE, clientPID, clientTID, e.what());

            this->mGlobalTableMutex.unlock();
            return false;

        } catch(const std::exception& e) {
            TYPELOGV(CLIENT_ALLOCATION_FAILURE, clientPID, clientTID, e.what());

            this->mGlobalTableMutex.unlock();
            return false;
        }
    }

    this->mGlobalTableMutex.unlock();
    return true;
}

std::unordered_set<int64_t>* ClientDataManager::getRequestsByClientID(int32_t clientTID) {
    this->mGlobalTableMutex.lock_shared();

    if(this->mClientTidRepo[clientTID] == nullptr || this->mClientTidRepo.size() == 0) {
        this->mGlobalTableMutex.unlock_shared();
        return nullptr;
    }

    std::unordered_set<int64_t>* clientHandlesPtr = this->mClientTidRepo[clientTID]->mClientHandles;
    this->mGlobalTableMutex.unlock_shared();

    return clientHandlesPtr;
}

void ClientDataManager::insertRequestByClientId(int32_t clientTID, int64_t requestHandle) {
    this->mGlobalTableMutex.lock_shared();

    if(this->mClientTidRepo[clientTID] == nullptr || this->mClientTidRepo.size() == 0) {
        this->mGlobalTableMutex.unlock_shared();
        return;
    }

    this->mClientTidRepo[clientTID]->mClientHandles->insert(requestHandle);
    this->mGlobalTableMutex.unlock_shared();
}

void ClientDataManager::deleteRequestByClientId(int32_t clientTID, int64_t requestHandle) {
    this->mGlobalTableMutex.lock_shared();

    if(this->mClientTidRepo.find(clientTID) == this->mClientTidRepo.end()) {
        this->mGlobalTableMutex.unlock_shared();
        return;
    }

    this->mClientTidRepo[clientTID]->mClientHandles->erase(requestHandle);
    this->mGlobalTableMutex.unlock_shared();
}

int8_t ClientDataManager::getClientLevelByClientID(int32_t clientPID) {
    this->mGlobalTableMutex.lock_shared();

    if(this->mClientRepo.find(clientPID) == this->mClientRepo.end()) {
        this->mGlobalTableMutex.unlock_shared();
        return -1;
    }

    int8_t clientLevel = this->mClientRepo[clientPID]->mClientType;
    this->mGlobalTableMutex.unlock_shared();

    return clientLevel;
}

std::vector<int32_t>* ClientDataManager::getThreadsByClientId(int32_t clientPID) {
    this->mGlobalTableMutex.lock_shared();

    if(this->mClientRepo.find(clientPID) == this->mClientRepo.end()) {
        this->mGlobalTableMutex.unlock_shared();
        return nullptr;
    }

    std::vector<int32_t>* threadIds = this->mClientRepo[clientPID]->mClientTIDs;
    this->mGlobalTableMutex.unlock_shared();

    return threadIds;
}

double ClientDataManager::getHealthByClientID(int32_t clientTID) {
    this->mGlobalTableMutex.lock_shared();

    if(this->mClientTidRepo.find(clientTID) == this->mClientTidRepo.end()) {
        this->mGlobalTableMutex.unlock_shared();
        return -1;
    }

    double health = this->mClientTidRepo[clientTID]->mHealth;
    this->mGlobalTableMutex.unlock_shared();

    return health;
}

int64_t ClientDataManager::getLastRequestTimestampByClientID(int32_t clientTID) {
    this->mGlobalTableMutex.lock_shared();

    if(this->mClientTidRepo.find(clientTID) == this->mClientTidRepo.end()) {
        this->mGlobalTableMutex.unlock_shared();
        return 0;
    }

    int64_t lastRequestTimestamp = this->mClientTidRepo[clientTID]->mLastRequestTimestamp;
    this->mGlobalTableMutex.unlock_shared();

    return lastRequestTimestamp;
}

void ClientDataManager::updateHealthByClientID(int32_t clientTID, double health) {
    this->mGlobalTableMutex.lock_shared();

    if(this->mClientTidRepo.find(clientTID) == this->mClientTidRepo.end()) {
        this->mGlobalTableMutex.unlock_shared();
        return;
    }

    this->mClientTidRepo[clientTID]->mHealth = health;
    this->mGlobalTableMutex.unlock_shared();
}

void ClientDataManager::updateLastRequestTimestampByClientID(int32_t clientTID, int64_t currentMillis) {
    this->mGlobalTableMutex.lock_shared();

    if(this->mClientTidRepo.find(clientTID) == this->mClientTidRepo.end()) {
        this->mGlobalTableMutex.unlock_shared();
        return;
    }

    this->mClientTidRepo[clientTID]->mLastRequestTimestamp = currentMillis;
    this->mGlobalTableMutex.unlock_shared();
}

void ClientDataManager::getActiveClientList(std::vector<int32_t>& clientList) {
    this->mGlobalTableMutex.lock_shared();

    for(std::pair<int32_t, ClientInfo*> clientInfo : this->mClientRepo) {
        clientList.push_back(clientInfo.first);
    }

    this->mGlobalTableMutex.unlock_shared();
}

void ClientDataManager::deleteClientPID(int32_t clientPID) {
    this->mGlobalTableMutex.lock();

    if(this->mClientRepo.find(clientPID) == this->mClientRepo.end()) {
        this->mGlobalTableMutex.unlock();
        return;
    }

    ClientInfo* clientInfo = this->mClientRepo[clientPID];

    FreeBlock<std::vector<int32_t>>(static_cast<void*>(clientInfo->mClientTIDs));
    FreeBlock<ClientInfo>(static_cast<void*>(clientInfo));

    this->mClientRepo.erase(clientPID);
    this->mGlobalTableMutex.unlock();
}

void ClientDataManager::deleteClientTID(int32_t clientTID) {
    this->mGlobalTableMutex.lock();

    if(this->mClientTidRepo.find(clientTID) == this->mClientTidRepo.end()) {
        this->mGlobalTableMutex.unlock();
        return;
    }

    ClientTidData* clientData = this->mClientTidRepo[clientTID];
    this->mClientTidRepo.erase(clientTID);

    FreeBlock<std::unordered_set<int64_t>>
            (static_cast<void*>(clientData->mClientHandles));
    FreeBlock<ClientTidData>(static_cast<void*>(clientData));

    this->mGlobalTableMutex.unlock();
}
