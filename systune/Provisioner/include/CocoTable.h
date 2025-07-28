// Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
// SPDX-License-Identifier: BSD-3-Clause-Clear

#ifndef COCOQ_H
#define COCOQ_H

#include <fstream>
#include <vector>
#include <unordered_map>
#include <mutex>
#include <cerrno>
#include <atomic>
#include <memory>

#include "ResourceProcessor.h"
#include "Request.h"
#include "RequestQueue.h"
#include "SystuneSettings.h"
#include "MemoryPool.h"
#include "Logger.h"
#include "Utils.h"

/**
* @brief Concurrency Coordinator.
* @details

* The need for a separate module focusing on managing concurrencies arises due to the fact that multiple clients
* can send multiple requests with different priorities for the same resource.
* It becomes important for Systune to intelligently decide the order in which the requests are applied.
*
* Every resource is allotted a Policy beforehand according to the requirements and the nature of the resource. These four policies were included:
* - Instant Apply (or Always Apply): This policy is for resources where the latest request needs to be honored.
* This is kept as the default policy.
* - Higher is better: This policy honors the request writing the highest value to the node.
* One of the cases where this makes sense is for resources that describe the upper bound value.
* By applying the higher-valued request, the lower-valued request is implicitly honored.
* - Lower is better: Self-explanatory. Works exactly opposite of the higher is better policy.
* - Lazy Apply: Sometimes, you want the resources to apply requests in a first-in-first-out manner.
*
* We provide 4 different Priority Levels for all requests.
* The requests are first divided into either a system request or a 3rd party request based on the thread IDs of the client.
* Later, the client has the opportunity to provide either a High or Low priority according to their requirements.
* - System High
* - System Low
* - 3rd Party High
* - 3rd Party Low
*
* The Concurrency Coordinator needs to honor both, the policy of the resource and the priority of the requests while taking decisions.

* Algorithm: Create 4 (number of currently supported priorities) doubly linked lists for each resource
* (or for each core in each resource if core lever conflict exists).
* Behavior of each linked list would depend on the policy specified in the resource table.
*
* For each Tune request
*     Associate a timer with the requested duration with the request.
*     Create a node for each of the resources it requires
*
*     For each resource:
*         Push node into the list according to the policy.
*             if the node is at the head of that list:
*                 if currently applied priority is lower than its own priority:
*                     apply action
*                 else ignore
*
* For each Retune request:
*     Update timer duration
*
* For each Untune request:
*     For each resource in the request:
*         Remove the node from the list
*
*         if the nodeToBeDeleted is at the head of that list:
*         //Meaning it could have been the node which was currently applied.
*             if currently applied priority is lower than the newHead's priority:
*                 apply action
*             else ignore
*
*         if the nodeToBeDeleted is the only node in that list:
*             Find the next node to be applied by iterating the priority vector.
* When any timer expires:
*   Create an untune request and submit to the Request Queue
*
*/
class CocoTable {
    friend class CocoTableTest;

private:
    static std::shared_ptr<CocoTable> mCocoTableInstance;
    static std::mutex instanceProtectionLock;

    std::vector<ResourceConfigInfo*> mResourceTable;

    /**
    * @brief The main data structure which is a 2D vector. It stores entries for each resource and each entry stores a priority vector.
    *        For each resource, for each priority, you store a head (second) and tail (first) pointers to store a linked list in memory.
    */
    std::vector<std::vector<std::pair<CocoNode*, CocoNode*>>> mCocoTable;

    /**
    * @brief Data structure storing the c2urrently applied priority for each resource. It is referred to whenever a new request comes in.
    */
    std::vector<int32_t> mCurrentlyAppliedPriority;

    void deleteNode(CocoNode* node, int32_t primaryIndex, int32_t secondaryIndex, int32_t priority);

    void applyAction(CocoNode* currNode, int32_t index, int32_t priority);
    void applyDefaultAction(int32_t index, Resource* resource);
    int8_t insertInCocoTable(CocoNode* currNode, Resource* resource, int32_t priority);

    void insertInCocoTableHigherLower(CocoNode* newNode, int32_t primaryIndex, int32_t secondaryIndex,
                                                 int32_t policy, int32_t priority);

    void insertInCocoTableLazyApply(CocoNode* newNode, int32_t primaryIndex,
                                               int32_t secondaryIndex, int32_t priority);

    void insertInCocoTableInstantApply(CocoNode* newNode, int32_t primaryIndex,
                                                  int32_t secondaryIndex, int32_t priority);

    int32_t getCocoTablePrimaryIndex(uint32_t opId);

    int32_t getCocoTableSecondaryIndex(uint32_t opId, int32_t mOpInfo, int32_t priority);

    /**
    * @brief This is a private routine called when a timer finishes for a request. It initiates
    *        an untune request and submits it in the request queue.
    */
    int32_t timerOver(Request* req);

    void triggerDisplayOffOrDozeResetting();

    CocoTable();

public:
    ~CocoTable();

    /**
    * @brief It creates CocoNodes for each resource
    *       and inserts them in the appropriate linked lists.
    */
    int8_t insertRequest(Request* req);

    /**
    * @brief It is called when the request needs to be untuned.
    *        It deletes nodes from appropriate linked lists.
    * @details The request object stores the pointers to its
    *        corresponding coconodes which are deleted when this
    *        routine is called.
    *
    */
    int32_t removeRequest(Request* req);

    /**
    * @brief This routine updates the timer of the request with the newDuration.
    */
    int8_t updateRequest(Request* req, int64_t duration);

    static std::shared_ptr<CocoTable> getInstance() {
        if(mCocoTableInstance == nullptr) {
            instanceProtectionLock.lock();
            if(mCocoTableInstance == nullptr) {
                try {
                    mCocoTableInstance = std::shared_ptr<CocoTable> (new CocoTable());
                } catch(const std::bad_alloc& e) {
                    instanceProtectionLock.unlock();
                    return nullptr;
                }
            }
            instanceProtectionLock.unlock();
        }
        return mCocoTableInstance;
    }
};

#endif
