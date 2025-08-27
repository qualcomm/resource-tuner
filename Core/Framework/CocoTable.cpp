// Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
// SPDX-License-Identifier: BSD-3-Clause-Clear

#include "CocoTable.h"

static int8_t comparison(int32_t first, int32_t second, int32_t policy) {
    if(policy == HIGHER_BETTER) {
        return first > second;
    } else {
        return first < second;
    }
}

std::shared_ptr<CocoTable> CocoTable::mCocoTableInstance = nullptr;
std::mutex CocoTable::instanceProtectionLock {};

CocoTable::CocoTable() {
    this->mResourceTable = ResourceRegistry::getInstance()->getRegisteredResources();
    int32_t totalResources = ResourceRegistry::getInstance()->getTotalResourcesCount();

    this->mCurrentlyAppliedPriority.resize(totalResources, -1);

    /*
        Initialize the CocoTable, the table will contain a vector corresponding to each Resource from the ResourceTable
        For the Resource if there is no level of conflict (i.e. apply type is "global"), then a vector of size 4 will be
        allocated, where each of the 4 entry corresponds to a different priority, as Currently 4 priority levels are supported (SH / SL and TPH / TPL)
        In case of Conflicts:
        - For a Resource with Core Level Conflict (i.e. apply type is "core") a vector of size: 4 * NUMBER_OF_CORES will be created
        - For a Resource with Cluster Level Conflict (i.e. apply type is "cluster") a vector of size: 4 * NUMBER_OF_CLUSTER will be created
        - For a Resource with CGroup Level Conflict (i.e. apply type is "cgroup") a vector of size: 4 * NUMBER_OF_CGROUPS_CREATED will be created

        How the CocoTable (Vector) will look:
        - Say we have 5 resources R1, R2, R3, R4, R5
        - Where R3 has core level conflict
        - R4 has cluster level conflict
        - R5 has cgroup level conflict
        - R1 and R2 have no conflict
        - Say core count is 4, cluster count is 4 and cgroup count is 2
        - P1, P2, P3 and P4 represent the priorities
        [
            R1: [P1: [], P2: [], P3: [], P4: []],                                       # size of vector: 4
            R2: [P1: [], P2: [], P3: [], P4: []],                                       # size of vector: 4
            R3: [                                                                       # size of vector: 4 * 4 = 16
                Core1_P1: [], Core1_P2: [], Core1_P3: [], Core1_P4: [],
                Core2_P1: [], Core2_P2: [], Core2_P3: [], Core2_P4: [],
                Core3_P1: [], Core3_P2: [], Core3_P3: [], Core3_P4: [],
                Core4_P1: [], Core4_P2: [], Core4_P3: [], Core4_P4: []
                ],
            R4: [                                                                       # size of vector: 4 * 4 = 16
                Cluster1_P1: [], Cluster1_P2: [], Cluster1_P3: [], Cluster1_P4: [],
                Cluster2_P1: [], Cluster2_P2: [], Cluster2_P3: [], Cluster2_P4: [],
                Cluster3_P1: [], Cluster3_P2: [], Cluster3_P3: [], Cluster3_P4: [],
                Cluster4_P1: [], Cluster4_P2: [], Cluster4_P3: [], Cluster4_P4: []
                ],
            R5: [                                                                       # size of vector: 4 * 2 = 8
                Cgroup1_P1: [], Cgroup1_P2: [], Cgroup1_P3: [], Cgroup1_P4: [],
                Cgroup2_P1: [], Cgroup2_P2: [], Cgroup2_P3: [], Cgroup2_P4: []
                ]
        ]
    */
    for(ResourceConfigInfo* resourceConfig: this->mResourceTable) {
        if(resourceConfig->mApplyType == ResourceApplyType::APPLY_CORE) {
            int32_t totalCoreCount = ResourceTunerSettings::targetConfigs.totalCoreCount;
            this->mCocoTable.push_back(std::vector<std::pair<CocoNode*, CocoNode*>>(TOTAL_PRIORITIES * totalCoreCount));

        } else if(resourceConfig->mApplyType == ResourceApplyType::APPLY_CLUSTER) {
            int32_t totalClusterCount = ResourceTunerSettings::targetConfigs.totalClusterCount;
            this->mCocoTable.push_back(std::vector<std::pair<CocoNode*, CocoNode*>>(TOTAL_PRIORITIES * totalClusterCount));

        } else if(resourceConfig->mApplyType == ResourceApplyType::APPLY_CGROUP) {
            int32_t totalCGroupCount = TargetRegistry::getInstance()->getCreatedCGroupsCount();
            this->mCocoTable.push_back(std::vector<std::pair<CocoNode*, CocoNode*>>(TOTAL_PRIORITIES * totalCGroupCount));

        } else if(resourceConfig->mApplyType == ResourceApplyType::APPLY_GLOBAL){
            this->mCocoTable.push_back(std::vector<std::pair<CocoNode*, CocoNode*>>(TOTAL_PRIORITIES));
        }
    }
}

void CocoTable::applyAction(CocoNode* currNode, int32_t index, int8_t priority) {
    if(currNode == nullptr) return;

    Resource* resource = currNode->mResource;

    if(this->mCurrentlyAppliedPriority[index] >= priority ||
       this->mCurrentlyAppliedPriority[index] == -1) {
        ResourceConfigInfo* resourceConfig = ResourceRegistry::getInstance()->getResourceById(resource->getResCode());
        if(resourceConfig->mModes & ResourceTunerSettings::targetConfigs.currMode) {
            // Check if a custom Applier (Callback) has been provided for this Resource, if yes, then call it
            // Note for resources with multiple values, the BU will need to provide a custom applier, which provides
            // the aggregation / selection logic.
            if(resourceConfig->mResourceApplierCallback != nullptr) {
                resourceConfig->mResourceApplierCallback(resource);
            }
            this->mCurrentlyAppliedPriority[index] = priority;
        }
    }
}

void CocoTable::removeAction(int32_t index, Resource* resource) {
    if(resource == nullptr) return;
    ResourceConfigInfo* resourceConfigInfo = ResourceRegistry::getInstance()->getResourceById(resource->getResCode());
    if(resourceConfigInfo != nullptr) {
        if(resourceConfigInfo->mResourceTearCallback != nullptr) {
            resourceConfigInfo->mResourceTearCallback(resource);
        }
        this->mCurrentlyAppliedPriority[index] = -1;
    }
}

void CocoTable::deleteNode(CocoNode* node, int32_t primaryIndex, int32_t secondaryIndex, int8_t priority) {
    if(node->prev) {
        node->prev->next = node->next;
    } else {
        // Node is at the head
        this->mCocoTable[primaryIndex][secondaryIndex].second = node->next;
        if(this->mCocoTable[primaryIndex][secondaryIndex].second == nullptr) {
            this->mCocoTable[primaryIndex][secondaryIndex].first = nullptr;
        }
    }

    if(node->next) {
        node->next->prev = node->prev;
    } else {
        // Node is at the tail
       this->mCocoTable[primaryIndex][secondaryIndex].first = node->prev;
        if(this->mCocoTable[primaryIndex][secondaryIndex].first == nullptr) {
            this->mCocoTable[primaryIndex][secondaryIndex].second = nullptr;
        }
    }
}

void CocoTable::insertInCocoTableHigherLower(CocoNode* newNode, int32_t primaryIndex,
                                             int32_t secondaryIndex, int32_t policy, int8_t priority) {
    if(newNode == nullptr) return;

    CocoNode* currNode = this->mCocoTable[primaryIndex][secondaryIndex].second;
    int8_t inserted = false;

    while(currNode != nullptr) {
        CocoNode* currNext = currNode->next;

        int32_t newValue;
        int32_t curValue;

        if(newNode->mResource->getValuesCount() == 1) {
            newValue = newNode->mResource->mResValue.value;
        } else {
            newValue = (*newNode->mResource->mResValue.values)[1];
        }

        if(currNode->mResource->getValuesCount() == 1) {
            curValue = currNode->mResource->mResValue.value;
        } else {
            curValue = (*currNode->mResource->mResValue.values)[1];
        }

        if(!inserted && comparison(newValue, curValue, policy)) {
            newNode->next = currNode;
            newNode->prev = currNode->prev;

            if(currNode->prev == nullptr) {
                currNode->prev = newNode;
                if(this->mCocoTable[primaryIndex][secondaryIndex].second == nullptr) {
                    this->mCocoTable[primaryIndex][secondaryIndex].first = nullptr;
                    this->mCocoTable[primaryIndex][secondaryIndex].second = nullptr;
                } else {
                    this->mCocoTable[primaryIndex][secondaryIndex].second = newNode;
                }
                // New Head
                this->applyAction(newNode, primaryIndex, priority);

            } else {
                currNode->prev->next = newNode;
                currNode->prev = newNode;
            }
            inserted = true;
            // break;
        }
        currNode = currNext;
    }

    if(!inserted) {
        CocoNode* tail = this->mCocoTable[primaryIndex][secondaryIndex].first;
        newNode->next = nullptr;
        newNode->prev = tail;

        if(tail) {
            tail->next = newNode;
        } else {
            this->mCocoTable[primaryIndex][secondaryIndex].second = newNode;
            this->applyAction(newNode, primaryIndex, priority);
        }

        this->mCocoTable[primaryIndex][secondaryIndex].first = newNode;
    }
}

void CocoTable::insertInCocoTableLazyApply(CocoNode* newNode, int32_t primaryIndex,
                                           int32_t secondaryIndex, int8_t priority) {
    if(newNode == nullptr) return;

    CocoNode* head = this->mCocoTable[primaryIndex][secondaryIndex].second;
    CocoNode* tail = this->mCocoTable[primaryIndex][secondaryIndex].first;

    if(tail != nullptr) {
        this->mCocoTable[primaryIndex][secondaryIndex].first = newNode;
        newNode->next = nullptr;
        newNode->prev = tail;
        tail->next = newNode;
    } else {
        // Means set head and tail to newNode
        this->mCocoTable[primaryIndex][secondaryIndex].first = newNode;
        this->mCocoTable[primaryIndex][secondaryIndex].second = newNode;
        newNode->prev = nullptr;
        newNode->next = nullptr;
        this->applyAction(newNode, primaryIndex, priority);
    }
}

void CocoTable::insertInCocoTableInstantApply(CocoNode* newNode, int32_t primaryIndex,
                                              int32_t secondaryIndex, int8_t priority) {
    if(newNode == nullptr) return;

    CocoNode* head = mCocoTable[primaryIndex][secondaryIndex].second;

    if(head != nullptr) {
        this->mCocoTable[primaryIndex][secondaryIndex].second = newNode;
        newNode->next = head;
        newNode->prev = nullptr;
        head->prev = newNode;
    } else {
        newNode->prev = nullptr;
        newNode->next = nullptr;
        this->mCocoTable[primaryIndex][secondaryIndex].second = newNode;
        this->mCocoTable[primaryIndex][secondaryIndex].first = newNode;
    }

    this->applyAction(newNode, primaryIndex, priority);
}

int32_t CocoTable::getCocoTablePrimaryIndex(uint32_t opId) {
    if(ResourceRegistry::getInstance()->getResourceById(opId) == nullptr) {
        return -1;
    }

    return ResourceRegistry::getInstance()->getResourceTableIndex(opId);
}

int32_t CocoTable::getCocoTableSecondaryIndex(Resource* resource, int8_t priority) {
    std::shared_ptr<ResourceRegistry> resourceRegistry = ResourceRegistry::getInstance();
    if(resourceRegistry->getResourceById(resource->getResCode()) == nullptr) {
        return -1;
    }

    ResourceConfigInfo* resourceConfigInfo =
        resourceRegistry->getResourceById(resource->getResCode());

    if(resourceConfigInfo->mApplyType == ResourceApplyType::APPLY_CORE) {
        int32_t physicalCore = resource->getCoreValue();
        return physicalCore * TOTAL_PRIORITIES + priority;

    } else if(resourceConfigInfo->mApplyType == ResourceApplyType::APPLY_CLUSTER) {
        int32_t physicalCluster = resource->getClusterValue();
        return physicalCluster * TOTAL_PRIORITIES + priority;

    } else if(resourceConfigInfo->mApplyType == ResourceApplyType::APPLY_CGROUP) {
        int32_t cGroupIdentifier = -1;
        if(resource->getValuesCount() == 1) {
            cGroupIdentifier = resource->mResValue.value;
        } else {
            cGroupIdentifier = (*resource->mResValue.values)[0];
        }

        if(cGroupIdentifier == -1) return -1;
        return cGroupIdentifier * TOTAL_PRIORITIES + priority;

    } else if(resourceConfigInfo->mApplyType == ResourceApplyType::APPLY_GLOBAL) {
        return priority;
    }

    return -1;
}

int8_t CocoTable::insertInCocoTable(CocoNode* currNode, Resource* resource, int8_t priority) {
    int32_t primaryIndex = this->getCocoTablePrimaryIndex(resource->getResCode());
    int32_t secondaryIndex = this->getCocoTableSecondaryIndex(resource, priority);

    if(primaryIndex < 0 || secondaryIndex < 0 ||
       primaryIndex >= this->mCocoTable.size() || secondaryIndex >= this->mCocoTable[primaryIndex].size()) {
        return false;
    }

    enum Policy policy = ResourceRegistry::getInstance()->getResourceById(resource->getResCode())->mPolicy;

    switch(policy) {
        case INSTANT_APPLY:
            this->insertInCocoTableInstantApply(currNode, primaryIndex, secondaryIndex, priority);
            break;
        case HIGHER_BETTER:
        case LOWER_BETTER:
            this->insertInCocoTableHigherLower(currNode, primaryIndex, secondaryIndex, policy, priority);
            break;
        case LAZY_APPLY:
            this->insertInCocoTableLazyApply(currNode, primaryIndex, secondaryIndex, priority);
            break;
        default:
            return false;
    }

    return true;
}

// Insert a new Request into the CocoTable
// Note: This method is only called for Tune Requests
// As part of this Routine, we allocate CocoNodes for the Request.
// The count of CocoNodes allocated should be equal to the Number of Resources in the Request
// However, It is not guarenteed that all the CocoNodes get allocated due to potential
// Memory allocation failures, hence we resort to a best-effort approach, where in
// We allocate as many CocoNodes as possible for the Request.
int8_t CocoTable::insertRequest(Request* req) {
    if(req == nullptr) return false;

    TYPELOGV(NOTIFY_COCO_TABLE_INSERT_START, req->getHandle());
    // Create a List to Hold all the CocoNodes for the Request
    std::vector<CocoNode*>* cocoNodesList = nullptr;
    try {
        cocoNodesList = new (GetBlock<std::vector<CocoNode*>>()) std::vector<CocoNode*>;
        cocoNodesList->resize(req->getResourcesCount(), nullptr);

    } catch(const std::bad_alloc& e) {
        TYPELOGV(REQUEST_MEMORY_ALLOCATION_FAILURE_HANDLE, req->getHandle(), e.what());
        return false;
    }

    // Allocate as many CocoNodes as Possible
    int32_t allocatedCocoNodesCount = 0;
    for(int32_t i = 0; i < req->getResourcesCount(); i++) {
        Resource* resource = req->getResourceAt(i);
        CocoNode* currNode = nullptr;

        try {
            currNode = new (GetBlock<CocoNode>()) CocoNode;
            allocatedCocoNodesCount++;

        } catch(const std::bad_alloc& e) {
            break;
        }

        if(currNode != nullptr) {
            currNode->mResource = resource;
            (*cocoNodesList)[i] = currNode;
        }
    }

    req->setCocoNodes(cocoNodesList);
    req->setNumCocoNodes(allocatedCocoNodesCount);

    if(allocatedCocoNodesCount == 0) {
        // No Point of Processing this Request any Further
        TYPELOGV(REQUEST_MEMORY_ALLOCATION_FAILURE_HANDLE, req->getHandle(), "Insufficient Memory");
        return false;
    }

    // Create a time to associate with the request
    Timer* requestTimer = nullptr;
    try {
        requestTimer = new (GetBlock<Timer>())
                            Timer(std::bind(&CocoTable::timerOver, this, req));
    } catch(const std::bad_alloc& e) {
        TYPELOGV(REQUEST_MEMORY_ALLOCATION_FAILURE_HANDLE, req->getHandle(), e.what());
        return false;
    }

    req->setTimer(requestTimer);

    // Start the timer for this request
    if(!requestTimer->startTimer(req->getDuration())) {
        TYPELOGV(TIMER_START_FAILURE, req->getHandle());
        return false;
    }

    // Insert all the allocated CocoNodes to the CocoTable
    // This will actually trigger the Lock Provisioning Flow
    for(int32_t i = 0; i < allocatedCocoNodesCount; i++) {
        CocoNode* currNode = (*cocoNodesList)[i];

        if(currNode != nullptr) {
            Resource* resource = currNode->mResource;

            // Insert the request in the CocoTable
            this->insertInCocoTable(currNode, resource, req->getPriority());
        }
    }

    // At this point the CocoNodesList and the Timer were successfully created,
    // as well as a Non-Zero number of CocoNodes were allocated for the Request
    // Hence, we can safely return true.
    return true;
}

int8_t CocoTable::updateRequest(Request* req, int64_t duration) {
    if(req == nullptr || duration < -1 || (duration > 0 && (duration < req->getDuration()))) return false;
    TYPELOGV(NOTIFY_COCO_TABLE_UPDATE_START, req->getHandle(), duration);

    // Update the duration of the request, and the corresponding timer interval.
    Timer* currTimer = req->getTimer();
    req->unsetTimer();
    currTimer->killTimer();
    FreeBlock<Timer>(static_cast<void*>(currTimer));

    req->setDuration(duration);
    // Create a time to associate with the request
    Timer* requestTimer = nullptr;
    try {
        requestTimer = new (GetBlock<Timer>())
                            Timer(std::bind(&CocoTable::timerOver, this, req));
    } catch(const std::bad_alloc& e) {
        TYPELOGV(REQUEST_MEMORY_ALLOCATION_FAILURE_HANDLE, req->getHandle(), e.what());
        return false;
    }

    req->setTimer(requestTimer);

    // Start the timer for this request
    if(!requestTimer->startTimer(req->getDuration())) {
        TYPELOGV(TIMER_START_FAILURE, req->getHandle());
        return false;
    }

    TYPELOGV(NOTIFY_COCO_TABLE_INSERT_SUCCESS, req->getHandle());
    return true;
}

void CocoTable::processResourceCleanupAt(Request* request, int32_t index) {
    Resource* resource = request->getResourceAt(index);

    int8_t priority = request->getPriority();
    int32_t primaryIndex = this->getCocoTablePrimaryIndex(resource->getResCode());
    int32_t secondaryIndex = this->getCocoTableSecondaryIndex(resource, priority);

    int8_t nodeIsHead = false;
    // Proceed with CocoNode cleanup,
    // Note the actual allocated CocoNode count might be smaller than the Number of Resources.
    if(index < request->getCocoNodesCount()) {
        CocoNode* nodeToDelete = request->getCocoNodeAt(index);

        if(nodeToDelete != nullptr) {
            if(nodeToDelete->prev == nullptr) {
                nodeIsHead = true;
            }
            this->deleteNode(nodeToDelete, primaryIndex, secondaryIndex, priority);
        }
    }

    // Reset the Resource Node value or if there are pending Requests for this Resource
    // then apply those Requests in the order determined by Resource Policy and Priority Level.

    // If the current list becomes empty, start from the highest priority
    // and look for available requests.
    // If all lists are empty, apply default action.
    if(this->mCocoTable[primaryIndex][secondaryIndex].second == nullptr) {
        int8_t allListsEmpty = true;
        int32_t reIndexIncrement = 0;
        ResourceConfigInfo* resourceConfig = ResourceRegistry::getInstance()->getResourceById(resource->getResCode());

        if(resourceConfig->mApplyType == ResourceApplyType::APPLY_CORE ||
            resourceConfig->mApplyType == ResourceApplyType::APPLY_CLUSTER ||
            resourceConfig->mApplyType == ResourceApplyType::APPLY_CGROUP) {
            reIndexIncrement = this->getCocoTableSecondaryIndex(resource, SYSTEM_HIGH);
            if(reIndexIncrement == -1) return;
        }

        for(int32_t prioLevel = 0; prioLevel < TOTAL_PRIORITIES; prioLevel++) {
            if(this->mCocoTable[primaryIndex][prioLevel + reIndexIncrement].second != nullptr) {
                this->mCurrentlyAppliedPriority[primaryIndex] = prioLevel;
                this->applyAction(this->mCocoTable[primaryIndex][prioLevel + reIndexIncrement].second,
                                  primaryIndex, prioLevel);
                allListsEmpty = false;
                break;
            }
        }
        if(allListsEmpty == true) {
            this->removeAction(primaryIndex, resource);
        }

    } else {
        // Current list is not empty.
        // Check if current node is at the head
        if(nodeIsHead) {
            // If it is head, Apply the next node (i.e. the next Request)
            this->applyAction(this->mCocoTable[primaryIndex][secondaryIndex].second, primaryIndex, priority);
        }
        // If node is not head, it implies some other Request is already applied
        // for this Resource, hence no action is needed here.
    }
}

// Methods for Request Cleanup
int8_t CocoTable::removeRequest(Request* request) {
    TYPELOGV(NOTIFY_COCO_TABLE_REMOVAL_START, request->getHandle());

    if(request->getUntuneProcessingOrder() == 0) {
        // Forward Pass
        for(int32_t i = 0; i < request->getResourcesCount(); i++) {
            this->processResourceCleanupAt(request, i);
        }
    } else {
        for(int32_t i = request->getResourcesCount() - 1; i >= 0; i--) {
            this->processResourceCleanupAt(request, i);
        }
    }

    return 0;
}

int32_t CocoTable::timerOver(Request* request) {
    TYPELOGV(NOTIFY_COCO_TABLE_REQUEST_EXPIRY, request->getHandle());

    Request* untuneRequest = nullptr;
    try {
        untuneRequest = new (GetBlock<Request>()) Request();
    } catch(const std::bad_alloc& e) {
        return true;
    }

    if(untuneRequest != nullptr) {
        request->populateUntuneRequest(untuneRequest);
        std::shared_ptr<RequestQueue> requestQueue = RequestQueue::getInstance();
        requestQueue->addAndWakeup(untuneRequest);
    }

    return true;
}

// CocoNodes allocated for the Request will be freed up as part of Request Cleanup,
// Use the Request::cleanUpRequest method, for freeing up these nodes.
CocoTable::~CocoTable() {}
