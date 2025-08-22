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
#include <cstring>
#include <memory>

#include "ResourceRegistry.h"
#include "TargetRegistry.h"
#include "Request.h"
#include "RequestQueue.h"
#include "ResourceTunerSettings.h"
#include "MemoryPool.h"
#include "Logger.h"
#include "Utils.h"

/*!
 * \file  CocoTable.h
 */

/*!
 * \ingroup  COCO_TABLE
 * \defgroup COCO_TABLE Concurrency Coordinator Table (CocoTable)
 * \details The need for a separate module focusing on managing concurrencies arises due to the fact that multiple clients
 * can send multiple requests with different priorities for the same resource.
 * It becomes important for Resource Tuner to intelligently decide the order in which the requests are applied.
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
 *
 * Algorithm: Create 4 (number of currently supported priorities) doubly linked lists for each resource
 * (or for each core in each resource if core level conflict exists).
 * Behavior of each linked list would depend on the policy specified in the resource table.
 *
 * Request Flow:\n\n
 * **Tune Request**:\n
 * -# Associate a timer with the requested duration with the request.\n
 * -# Create a CocoNode for each of the Resource part of the Request it requires\n
 * -# Insert each of the CocoNodes to the doubly-linked list corresponding to the Resource and the Priority\n
 * -# The Node will be inserted in accordance to the Resource Policy\n
 * -# When the node reaches the head of the Linked List it will be applied, i.e. the value\n
 *    specified by the Tune Request will take effect on that Resource Node.\n
 * -# When the Request Expires, the timer will trigger a Callback and an Untune Request will be issued for
 *    this handle, to clean up the Tune Request and Reset the Resource Nodes.
 *
 * **Retune Request**:\n
 *     Update the Request duration, which involves killing the Timer associated with the Request and starting
 *     it again with the new Duration\n\n
 *
 * **Untune Request**:\n
 * -# For each Resource in the request, remove the corresponding CocoNode node from the list\n
 * -# Reset each of the Resource Sysfs Nodes to their original values, if there are no
 * other Pending Requests for that Resource.\n
 *
 * @{
 */


/**
* @brief CocoTable
*/
class CocoTable {
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

    CocoTable();

    void deleteNode(CocoNode* node, int32_t primaryIndex, int32_t secondaryIndex, int8_t priority);

    void applyAction(CocoNode* currNode, int32_t index, int8_t priority);

    void applyDefaultAction(int32_t index, Resource* resource);

    int8_t insertInCocoTable(CocoNode* currNode, Resource* resource, int8_t priority);

    void insertInCocoTableHigherLower(CocoNode* newNode, int32_t primaryIndex, int32_t secondaryIndex,
                                      int32_t policy, int8_t priority);

    void insertInCocoTableLazyApply(CocoNode* newNode, int32_t primaryIndex,
                                    int32_t secondaryIndex, int8_t priority);

    void insertInCocoTableInstantApply(CocoNode* newNode, int32_t primaryIndex,
                                       int32_t secondaryIndex, int8_t priority);

    int32_t getCocoTablePrimaryIndex(uint32_t opId);

    int32_t getCocoTableSecondaryIndex(Resource* resource, int8_t priority);

    int32_t timerOver(Request* req);

    void triggerDisplayOffOrDozeResetting();

public:
    ~CocoTable();

    /**
    * @brief Insert a Request to the CocoTable for application.
    * @details CocoNodes are created for each resource and inserted in appropriate
    *          Resource level linked lists.
    * @param req A pointer to the Request to be inserted
    * @return int8_t:
    *           1: If the Request was inserted successfully into the CocoTable
    *           0: Otherwise
    */
    int8_t insertRequest(Request* req);

    /**
    * @brief It is called when the request needs to be untuned.
    *        It deletes nodes from appropriate linked lists.
    * @details The request object stores the pointers to its
    *        corresponding coconodes which are deleted when this
    *        routine is called.
    * @param req A pointer to the Request to be inserted
    * @return int8_t:
    *           1: If the Request was inserted successfully into the CocoTable
    *           0: Otherwise
    */
    int8_t removeRequest(Request* req);

    /**
    * @brief Used to update the duration of an Active Request
    * @details This routine is called to serve Retune Requests.
    * @param req A pointer to the Request to be inserted
    * @param duration The new duration of the request
    * @return int8_t:
    *           1: If the Request was inserted successfully into the CocoTable
    *           0: Otherwise
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

/*! @} */
