// Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
// SPDX-License-Identifier: BSD-3-Clause-Clear

#include "RequestManager.h"

std::shared_ptr<RequestManager> RequestManager::mReqeustManagerInstance = nullptr;
std::mutex RequestManager::instanceProtectionLock{};

RequestManager::RequestManager() {}

int8_t RequestManager::isSane(Request* request) {
    try {
        if(request == nullptr) {
            throw std::invalid_argument("Request is nullptr");
        }
    } catch(const std::exception& e) {
        LOGE("RESTUNE_REQUEST_MANAGER",
            "Cannot Check Request Sanity: " +  std::string(e.what()));
        return false;
    }

    if(request->getRequestType() != REQ_RESOURCE_TUNING &&
       request->getRequestType() != REQ_RESOURCE_RETUNING &&
       request->getRequestType() != REQ_RESOURCE_UNTUNING) {
        return false;
    }

    if(request->getHandle() <= 0 || request->getResourcesCount() < 0 ||
       request->getPriority() < 0 || request->getClientPID() < 0 || request->getClientTID() < 0) {
        return false;
    }

    for(int32_t i = 0; i < request->getResourcesCount(); i++) {
        if(request->getResourceAt(i) == nullptr) return false;
    }

    return true;
}

int8_t RequestManager::requestMatch(Request* request) {
    int32_t clientTID = request->getClientTID();

    // Get the list of Requests for this client
    std::unordered_set<int64_t>* clientHandles =
        ClientDataManager::getInstance()->getRequestsByClientID(clientTID);

    if(clientHandles == nullptr || clientHandles->size() == 0) {
        return false;
    }
    // Assumptions: Number of Resources per request won't be too large
    // If it is, we can use multiple threads from the pool for faster checking

    for(int64_t handle: *clientHandles) {
        Request* targetRequest = this->mActiveRequests[handle];

        // Check that the Number of Resources in the requests are the same
        if(request->getResourcesCount() != targetRequest->getResourcesCount()) {
            return false;
        }

        // This covers the case where the client fires the exact same request
        // multiple times
        for(int32_t i = 0; i < request->getResourcesCount(); i++) {
            Resource* res1 = request->getResourceAt(i);
            Resource* res2 = targetRequest->getResourceAt(i);

            if(res1->getResCode() != res2->getResCode()) return false;
            if(res1->getResInfo() != res2->getResInfo()) return false;
            if(res1->getOptionalInfo() != res2->getOptionalInfo()) return false;
            if(res1->getValuesCount() != res2->getValuesCount()) return false;

            if(res1->getValuesCount() == 1) {
                if(res1->mResValue.value != res2->mResValue.value) {
                    return false;
                }
            } else {
                for(int32_t i = 0; i < res1->getValuesCount(); i++) {
                    if(res1->mResValue.values[i] != res2->mResValue.values[i]) return false;
                }
            }
        }

        // Consideration:
        // How to efficiently check duplicate in cases where the client sends 2
        // requests with the same Resources, but in different orders. For example:
        // Rq1 -> Rs1, Rs2, Rs3
        // Rq2 - Rs1, Rs3, Rs2
        // - These requests are still duplicates, but the above logic won't catch it
    }

    return true;
}

int8_t RequestManager::verifyHandle(int64_t handle) {
    this->mRequestMapMutex.lock_shared();
    int8_t handleExists = (mActiveRequests.find(handle) != mActiveRequests.end());
    this->mRequestMapMutex.unlock_shared();

    return handleExists;
}

Request* RequestManager::getRequestFromMap(int64_t handle) {
    this->mRequestMapMutex.lock_shared();
    Request* request = mActiveRequests[handle];
    this->mRequestMapMutex.unlock_shared();

    return request;
}

int8_t RequestManager::isRequestAlive(int64_t handle) {
   return this->mActiveRequests.find(handle) != this->mActiveRequests.end();
}

int8_t RequestManager::shouldRequestBeAdded(Request* request) {
    //sanity check.
    if(!isSane(request)) return false;

    this->mRequestMapMutex.lock_shared();

    // Check for duplicates
    int8_t duplicateFound = requestMatch(request);

    this->mRequestMapMutex.unlock_shared();

    return !duplicateFound;
}

void RequestManager::addRequest(Request* request) {
    if(request == nullptr) return;
    this->mRequestMapMutex.lock();

    int64_t handle = request->getHandle();

    // Populate all the Trackers with info for this Request
    this->mActiveRequests[handle] = request;
    this->mRequestsList[ACTIVE_TUNE].insert(request);
    this->mRequestProcessingStatus[handle] = REQ_UNCHANGED;

    // Add this request handle to the client list
    int32_t clientTID = request->getClientTID();
    ClientDataManager::getInstance()->insertRequestByClientId(clientTID, handle);

    this->mActiveRequestCount++;
    this->mRequestMapMutex.unlock();
}

void RequestManager::removeRequest(Request* request) {
    if(request == nullptr) return;
    this->mRequestMapMutex.lock();

    // Remove the handle reference from the client handles list
    int32_t clientTID = request->getClientTID();
    int64_t handle = request->getHandle();

    ClientDataManager::getInstance()->deleteRequestByClientId(clientTID, handle);

    // Remove the handle from the list of active requests
    this->mActiveRequests.erase(handle);
    // Remove the Request from the list of Active Requests
    this->mRequestsList[ACTIVE_TUNE].erase(request);
    this->mRequestProcessingStatus.erase(handle);

    this->mActiveRequestCount--;
    this->mRequestMapMutex.unlock();
}

std::unordered_map<int64_t, Request*> RequestManager::getActiveRequests() {
    this->mRequestMapMutex.lock_shared();
    std::unordered_map<int64_t, Request*> activeRequests = mActiveRequests;
    this->mRequestMapMutex.unlock_shared();
    return activeRequests;
}

void RequestManager::disableRequestProcessing(int64_t handle) {
    this->mRequestProcessingStatus[handle] = REQ_CANCELLED;
}

void RequestManager::modifyRequestDuration(int64_t handle, int64_t duration) {
    if(this->mRequestProcessingStatus[handle] != REQ_CANCELLED) {
        this->mRequestProcessingStatus[handle] = duration;
    }
}

int64_t RequestManager::getActiveReqeustsCount() {
    return this->mActiveRequestCount;
}

void RequestManager::markRequestAsComplete(int64_t handle) {
    this->mRequestProcessingStatus[handle] = REQ_COMPLETED;
}

int64_t RequestManager::getRequestProcessingStatus(int64_t handle) {
    return this->mRequestProcessingStatus[handle];
}

void RequestManager::triggerDisplayOffOrDozeMode() {
    // This method will essentially drain out the CocoTable
    // The Requests will be moved to the Pending List or Kept in the Active Requests List
    // based on whether background Processing is enabled for the Request.
    std::vector<Request*> requestsToBeRemoved;

    for(Request* request: this->mRequestsList[ACTIVE_TUNE]) {
        // Iterate over all the Requests in the activeRequestsList
        // - Send Corresponding Untune Request for each of the Requests
        // - Add the Requests to the Pending List, which are not eligible for background processing
        //   while removing them from the activeRequestsList
        Request* untuneRequest = nullptr;
        try {
            untuneRequest = new (GetBlock<Request>()) Request();
        } catch(const std::bad_alloc& e) {
            LOGI("RESTUNE_REQUEST_MANAGER"
                 "Failed to create Untune Request for Request: ", std::to_string(request->getHandle()));
        }

        if(untuneRequest != nullptr) {
            request->populateUntuneRequest(untuneRequest);

            // Keep the Untune Request's Priority as high as possible
            // So that all the existing Requests are untuned before the new Requests are Added.
            untuneRequest->setPriority(HIGH_TRANSFER_PRIORITY);
            RequestQueue::getInstance()->addAndWakeup(untuneRequest);
        }

        if(request->isBackgroundProcessingEnabled() == false) {
            // If the Request is not a background Request, then add it to the Pending List
            // and remove it from the activeRequestsList
            if(getRequestProcessingStatus(request->getHandle()) != REQ_CANCELLED) {
                this->mRequestsList[PENDING_TUNE].insert(request);
                requestsToBeRemoved.push_back(request);
            }
        } else {
            if(getRequestProcessingStatus(request->getHandle()) == REQ_CANCELLED) {
                requestsToBeRemoved.push_back(request);
            }
        }
    }

    // All the Active Tune Requests have been Untuned
    // Now, delete any Background Requests from the Active List for which we have received an Untune Request.
    for(Request* request: requestsToBeRemoved) {
        this->mRequestsList[ACTIVE_TUNE].erase(request);
    }
}

void RequestManager::triggerDisplayOnMode() {
    // This method will essentially drain out the CocoTable
    // All The Requests in the Pending List will be moved to the Active Requests List
    std::vector<Request*> untunedRequests;

    for(Request* request: this->mRequestsList[ACTIVE_TUNE]) {
        // Iterate over all the Requests in the activeRequestsList
        // - Send Corresponding Untune Request for each of the Requests
        // - Add the Requests to the Pending List, which are not eligible for background processing
        //   while removing them from the activeRequestsList
        Request* untuneRequest = nullptr;
        try {
            untuneRequest = new (GetBlock<Request>()) Request();
        } catch(const std::bad_alloc& e) {
            LOGI("RESTUNE_REQUEST_MANAGER"
                 "Failed to create Untune Request for Request: ", std::to_string(request->getHandle()));
        }

        if(untuneRequest != nullptr) {
            request->populateUntuneRequest(untuneRequest);
            RequestQueue::getInstance()->addAndWakeup(untuneRequest);
        }

        if(getRequestProcessingStatus(request->getHandle()) == REQ_CANCELLED) {
            untunedRequests.push_back(request);
        }
    }

    // All the Active Tune Requests have been Untuned
    // Now, delete any Requests from the Active List for which we have received an Untune Request.
    for(Request* request: untunedRequests) {
        this->mRequestsList[ACTIVE_TUNE].erase(request);
    }

    // Add all the Requests from the pending list into the Active List
    for(Request* request: this->mRequestsList[PENDING_TUNE]) {
        this->mRequestsList[ACTIVE_TUNE].insert(request);
    }
}

void RequestManager::floodInRequestsForProcessing() {
    // Issue back all the Tune Requests, which can be processed in Background.
    std::vector<Request*> tuneRequests;
    for(Request* request: this->mRequestsList[ACTIVE_TUNE]) {
        tuneRequests.push_back(request);
    }
    for(Request* request: tuneRequests) {
        RequestQueue::getInstance()->addAndWakeup(request);
    }
}

RequestManager::~RequestManager() {}
