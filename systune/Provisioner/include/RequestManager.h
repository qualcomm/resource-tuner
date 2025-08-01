// Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
// SPDX-License-Identifier: BSD-3-Clause-Clear

#ifndef REQUEST_MANAGER_H
#define REQUEST_MANAGER_H

#include <unordered_set>
#include <unordered_map>
#include <memory>

#include "Request.h"
#include "CocoTable.h"
#include "ClientDataManager.h"

enum RequestListType {
    ACTIVE_TUNE,
    PENDING_TUNE,
    ACTIVE_UNTUNE,
    ACTIVE_RETUNE
};

enum RequestProcessingStatus {
    REQ_UNCHANGED = -2,
    REQ_CANCELLED,
    REQ_COMPLETED
};

/**
 * @details Responsible for Tracking and Maintaining all the active Requests, currently
 *          submitted to the Syslock Server. Additionally it is responsible for performing
 *          Request Duplication Check, which aims to improve System efficiency by reducing
 *          wasteful duplicate processing.
 */
class RequestManager {
private:
    static std::shared_ptr<RequestManager> mReqeustManagerInstance;
    static std::mutex instanceProtectionLock;

    int64_t mActiveRequestCount;
    std::unordered_set<Request*> mRequestsList[4];
    std::unordered_map<int64_t, Request*> mActiveRequests;
    std::unordered_map<int64_t, int64_t> mRequestProcessingStatus;
    std::shared_timed_mutex mRequestMapMutex;

    RequestManager();

    int8_t checkOwnership(Request* request, Request* targetRequest);
    int8_t isSane(Request* request);
    int8_t requestMatch(Request* request);

public:
    ~RequestManager();

    /**
    * @brief Check if a Request with the specified handle exists in the RequestMap
    * @details This routine is used by the retune/untune APIs
    * @param handle Request Handle
    * @return: int8_t
    *                   1: if a request with the specified handle exists
    *                   0: otherwise
    */
    int8_t verifyHandle(int64_t handle);

    /**
    * @brief Checks whether the specified Request should be added to the RequestMap
    * @details This routine will perform Request Sanity and Duplicate checking.
    * @param request pointer to the request to be added to the map
    * @return: int8_t
    *                   1: if the request should be added
    *                   0: otherwise
    */
    int8_t shouldRequestBeAdded(Request* request);

    /**
    * @brief Add the specified request to the RequestMap
    * @details This routine should only be called if shouldRequestBeAdded returns 1.
    * @param request pointer to the request to be added to the map
    */
    void addRequest(Request* request);

    /**
    * @brief Remove a given request from the RequestMap
    * @param request pointer to the request to be removed from the map
    * @return: void
    */
    void removeRequest(Request* request);

    /**
    * @brief Retrieve the Request with the given Handle.
    * @param handle Request Handle
    * @return Pointer to the request with the specified index.
    */
    Request* getRequestFromMap(int64_t handle);

    /**
    * @brief Get the current Global Active Requests Count
    * @return: int64_t
    */
    int64_t getActiveReqeustsCount();

    int8_t isRequestAlive(int64_t handle);

    /**
    * @brief Mark the Request Handle, so that the Request won't be applied.
    * @details This method is used to Handle Untune Requests, as soon as an Untune
    *          Request for Handle h1 is received, we call this Rountine for h1, so that
    *          it doesn't get serviced in case it has not been inserted into the Coco
    *          Table yet.
    *          Note: If the Request was already inserted, then it will be untuned as and
    *          when the Untune Request gets serviced by the RequestQueue.
    *
    * @param handle Request Identifier, for which processing needs to be disabled.
    */
    void disableRequestProcessing(int64_t handle);

    void modifyRequestDuration(int64_t handle, int64_t duration);

    void markRequestAsComplete(int64_t handle);

    int64_t getRequestProcessingStatus(int64_t handle);

    std::unordered_map<int64_t, Request*> getActiveRequests();

    void triggerDisplayOffOrDozeMode();

    void triggerDisplayOnMode();

    void floodInRequestsForProcessing();

    static std::shared_ptr<RequestManager> getInstance() {
        if(mReqeustManagerInstance == nullptr) {
            instanceProtectionLock.lock();
            if(mReqeustManagerInstance == nullptr) {
                try {
                    mReqeustManagerInstance = std::shared_ptr<RequestManager> (new RequestManager());
                } catch(const std::bad_alloc& e) {
                    instanceProtectionLock.unlock();
                    return nullptr;
                }
            }
            instanceProtectionLock.unlock();
        }
        return mReqeustManagerInstance;
    }
};

#endif
