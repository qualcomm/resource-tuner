// Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
// SPDX-License-Identifier: BSD-3-Clause-Clear

#include "ClientGarbageCollector.h"

std::shared_ptr<ClientGarbageCollector> ClientGarbageCollector::mClientGarbageCollectorInstance = nullptr;
ClientGarbageCollector::ClientGarbageCollector() {
    this->mTimer = nullptr;
    this->mGarbageCollectionDuration = SystuneSettings::metaConfigs.mClientGarbageCollectorDuration;
}

void ClientGarbageCollector::submitClientThreadsForCleanup(int32_t clientPid) {
    const std::lock_guard<std::mutex> lock(this->mGcQueueMutex);
    std::vector<int32_t>* clientThreads = ClientDataManager::getInstance()->getThreadsByClientId(clientPid);
    for(int32_t threadId: *clientThreads) {
        this->mGcQueue.push(threadId);
    }
}

void ClientGarbageCollector::performCleanup() {
    const std::lock_guard<std::mutex> lock(this->mGcQueueMutex);
    for(int32_t count = 0; count < std::min((int32_t)this->mGcQueue.size(), BATCH_SIZE); count++) {
        int32_t clientTID = this->mGcQueue.front();
        this->mGcQueue.pop();

        LOGD("URM_CLIENT_GARBAGE_COLLECTOR",
             "Proceeding with Cleanup for Client TID: " + std::to_string(clientTID));

        std::unordered_set<int64_t>* clientHandles =
            ClientDataManager::getInstance()->getRequestsByClientID(clientTID);

        // Issue Untune Requests, to remove the corresponding Tune Requests from the CocoTable
        // Delete this Request from RequestManager's activeList and tracker.

        std::vector<int64_t> handlesToRemove;
        for(int64_t handle: *clientHandles) {
            handlesToRemove.push_back(handle);
        }

        ClientDataManager::getInstance()->deleteClientTID(clientTID);

        for(int64_t handle: handlesToRemove) {
            Request* request = RequestManager::getInstance()->getRequestFromMap(handle);
            if(request == nullptr) continue;

            Request* untuneRequest = nullptr;

            try {
                untuneRequest = new (GetBlock<Request>()) Request();
                request->populateUntuneRequest(untuneRequest);
            } catch(const std::bad_alloc& e) {
                LOGI("URM_CLIENT_GARBAGE_COLLECTOR",
                     "Failed to Allocate Memory for Untune Request. Error: " + std::string(e.what()));
            }

            // Keep the Untune Request's Priority as high as possible
            // So that all the existing Requests are untuned before the new Requests are Added.
            if(untuneRequest != nullptr) {
                untuneRequest->setPriority(HIGH_TRANSFER_PRIORITY);
                RequestQueue::getInstance()->addAndWakeup(untuneRequest);
            }
        }
    }
}

ErrCode ClientGarbageCollector::startClientGarbageCollectorDaemon() {
    try {
        this->mTimer = new (GetBlock<Timer>())
                            Timer(std::bind(&ClientGarbageCollector::performCleanup, this), true);

    } catch(const std::bad_alloc& e) {
        return RC_MEMORY_ALLOCATION_FAILURE;
    }

    if(!this->mTimer->startTimer(this->mGarbageCollectionDuration)) {
        return RC_WORKER_THREAD_ASSIGNMENT_FAILURE;
    }

    LOGI("URM_CLIENT_GARBAGE_COLLECTOR", "Garbage Collector Daemon Thread Started");
    return RC_SUCCESS;
}

ClientGarbageCollector::~ClientGarbageCollector() {
    if(this->mTimer != nullptr) {
        delete this->mTimer;
        this->mTimer = nullptr;
    }
}

ErrCode startClientGarbageCollectorDaemon() {
    if(ClientGarbageCollector::getInstance() == nullptr) {
        return RC_MEMORY_ALLOCATION_FAILURE;
    }
    return ClientGarbageCollector::getInstance()->startClientGarbageCollectorDaemon();
}
