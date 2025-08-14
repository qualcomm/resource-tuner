// Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
// SPDX-License-Identifier: BSD-3-Clause-Clear

#include "ServerRequests.h"

static int8_t getRequestPriority(int8_t clientPermissions, int8_t reqSpecifiedPriority) {
    if(clientPermissions == PERMISSION_SYSTEM) {
        switch(reqSpecifiedPriority) {
            case RequestPriority::REQ_PRIORITY_HIGH:
                return SYSTEM_HIGH;
            case RequestPriority::REQ_PRIORITY_LOW:
                return SYSTEM_LOW;
            default:
                return -1;
        }

    } else if(clientPermissions == PERMISSION_THIRD_PARTY) {
        switch(reqSpecifiedPriority) {
            case RequestPriority::REQ_PRIORITY_HIGH:
                return THIRD_PARTY_HIGH;
            case RequestPriority::REQ_PRIORITY_LOW:
                return THIRD_PARTY_LOW;
            default:
                return -1;
        }
    }

    // Note: Since Client Permissions and Request Specified Priority have already been individually
    // validated, hence control should not reach here.
    return -1;
}

/**
 * @brief Check if Valid Logical to Physical Core / Cluster Mapping can be performed.
 * @details If the logical core and Cluster values are valid then the Logical to Physical
 *          translation will be performed, and the result will be stored in the IN/OUT params.
 * @param int32_t Logical Core Value.
 * @param int32_t Logical Cluster Value.
 * @return int8_t: 1 if a valid Logical to Physical Translation exists.
 *                 0 otherwise.
 */
static int8_t performPhysicalMapping(int32_t& coreValue, int32_t& clusterValue) {
    std::shared_ptr<TargetRegistry> targetRegistry = TargetRegistry::getInstance();
    if(targetRegistry == nullptr) return false;

    int32_t physicalClusterValue = targetRegistry->getPhysicalClusterId(clusterValue);
    int32_t physicalCoreValue = targetRegistry->getPhysicalCoreId(clusterValue, coreValue);

    if(physicalCoreValue != -1 && physicalClusterValue != -1) {
        coreValue = physicalCoreValue;
        clusterValue = physicalClusterValue;
    } else {
        return false;
    }

    return true;
}

/**
 * @brief Verifies the validity of an incoming request.
 * @details This function checks the request's resources against configuration constraints,
 * permissions, and performs logical-to-physical mapping.
 *
 * @param req Pointer to the Request object.
 * @return int8_t True if the request is valid, false otherwise.
 */
static int8_t VerifyIncomingRequest(Request* req) {
    if(req->getDuration() < -1 || req->getDuration() == 0) return false;

    std::vector<Resource*> resourcesToBeTuned = *(req->getResources());

    // No Resources to be Tuned, Reject this Request
    if(resourcesToBeTuned.size() == 0) return false;

    int8_t clientPermissions =
        ClientDataManager::getInstance()->getClientLevelByClientID(req->getClientPID());
    // If the client permissions could not be determined, reject this request.
    // This could happen if the /proc/<pid>/status file for the Process could not be opened.
    if(clientPermissions == -1) {
        TYPELOGV(VERIFIER_INVALID_PERMISSION, req->getClientPID(), req->getClientTID());
        return false;
    }

    // Check Request Priority
    int8_t reqSpecifiedPriority = req->getPriority();
    if(reqSpecifiedPriority > RequestPriority::REQ_PRIORITY_LOW ||
       reqSpecifiedPriority < RequestPriority::REQ_PRIORITY_HIGH) {
        TYPELOGV(VERIFIER_INVALID_PRIORITY, reqSpecifiedPriority);
        return false;
    }

    int8_t allowedPriority = getRequestPriority(clientPermissions, reqSpecifiedPriority);
    if(allowedPriority == -1) return false;
    req->setPriority(allowedPriority);

    for(int32_t i = 0; i < resourcesToBeTuned.size(); i++) {
        Resource* resource = resourcesToBeTuned[i];
        if(resource == nullptr) {
            return false;
        }

        ResourceConfigInfo* resourceConfig = ResourceRegistry::getInstance()->getResourceById(resource->getOpCode());

        // Basic sanity: Invalid resource Opcode
        if(resourceConfig == nullptr) {
            TYPELOGV(VERIFIER_INVALID_OPCODE, resource->getOpCode());
            return false;
        }

        // if(resourceConfig->mApplyType == APPLY_CORE) {
        //     if(resourceConfig->resourceApplierCallback != nullptr) {
        //         CGroupApplyInfo* cGroupInfo = new CGroupApplyInfo;
        //         cGroupInfo->mClientPID = req->getClientPID();
        //         cGroupInfo->mClientTID = req->getClientTID();
        //         cGroupInfo->mResource = resource;
        //         resourceConfig->resourceApplierCallback(cGroupInfo);
        //     }
        //     continue;
        // }

        if(resource->getValuesCount() == 1) {
            // Verify value is in the range [LT, HT]
            int32_t configValue = resource->mConfigValue.singleValue;
            int32_t lowThreshold = resourceConfig->mLowThreshold;
            int32_t highThreshold = resourceConfig->mHighThreshold;

            if(configValue < lowThreshold || configValue > highThreshold) {
                TYPELOGV(VERIFIER_VALUE_OUT_OF_BOUNDS, configValue, resource->getOpCode());
                return false;
            }
        } else {
            // Note: Extend this verification for multiple values
        }

        // Verify tuning is supported for the resource in question
        if(!resourceConfig->mSupported) return false;

        // Check for Client permissions
        if(resourceConfig->mPermissions == PERMISSION_SYSTEM && clientPermissions == PERMISSION_THIRD_PARTY) {
            TYPELOGV(VERIFIER_NOT_SUFFICIENT_PERMISSION, resource->getOpCode());
            return false;
        }

        // If CoreLevelConflict for the Resource is set to true, then perform Logical to Physical Translation
        if(resourceConfig->mCoreLevelConflict) {
            // Check for invalid Core / cluster values, these are the logical values
            int32_t coreValue = resource->getCoreValue();
            int32_t clusterValue = resource->getClusterValue();

            if(coreValue <= 0 || clusterValue < 0) return false;

            // Perform logical to physical mapping here, as part of which verification can happen
            // Replace mOpInfo with the Physical values here:
            if(!performPhysicalMapping(coreValue, clusterValue)) {
                TYPELOGV(VERIFIER_LOGICAL_TO_PHYSICAL_MAPPING_FAILED, resource->getOpCode());
                return false;
            }

            resource->setCoreValue(coreValue);
            resource->setClusterValue(clusterValue);
        }
    }

    return true;
}

static void dumpRequest(Request* clientReq) {
    std::string LOG_TAG = "RTN_SERVER";

    LOGD(LOG_TAG, "Request details:");
    LOGD(LOG_TAG, "reqType: " + std::to_string(clientReq->getRequestType()));
    LOGD(LOG_TAG, "handle: " + std::to_string(clientReq->getHandle()));
    LOGD(LOG_TAG, "Duration: " + std::to_string(clientReq->getDuration()));
    LOGD(LOG_TAG, "Priority: " + std::to_string(clientReq->getPriority()));
    LOGD(LOG_TAG, "client PID: " +std::to_string(clientReq->getClientPID()));
    LOGD(LOG_TAG, "client TID: " + std::to_string(clientReq->getClientTID()));
    LOGD(LOG_TAG, "Background Processing Enabled?: " + std::to_string((int32_t)clientReq->isBackgroundProcessingEnabled()));
    LOGD(LOG_TAG, "Number of Resources: " + std::to_string(clientReq->getResourcesCount()));

    LOGD(LOG_TAG, "Values for resources are as:");

    for(int32_t i = 0; i < clientReq->getResourcesCount(); i++) {
        Resource* res = clientReq->getResourceAt(i);
        LOGD(LOG_TAG, "Resource " + std::to_string(i + 1) + ":");
        LOGD(LOG_TAG, "Opcode ID: " + std::to_string(res->getOpCode()));
        LOGD(LOG_TAG, "Number of Values: " + std::to_string(res->getValuesCount()));
        LOGD(LOG_TAG, "-- Single Value: " + std::to_string(res->mConfigValue.singleValue));
    }
}

static void processIncomingRequest(Request* request, int8_t isValidated=false) {
    if(isValidated) {
        // Request is already validated, add it to RequestQueue directly.
        if(RequestManager::getInstance()->shouldRequestBeAdded(request)) {
            RequestManager::getInstance()->addRequest(request);

            // Add this request to the RequestQueue
            RequestQueue::getInstance()->addAndWakeup(request);

        } else {
            LOGD("RTN_SERVER_REQUESTS", "Duplicate found, dropping request.");
            Request::cleanUpRequest(request);
        }

        RequestQueue::getInstance()->addAndWakeup(request);
        return;
    }

    std::shared_ptr<RateLimiter> rateLimiter = RateLimiter::getInstance();

    // Perform a Global Rate Limit Check before Processing the Request
    // This is to check if the current Active Request count has hit the
    // Max Number of Concurrent Requests Allowed Threshold
    // If the Threshold has been hit, we don't process the Request any further.
    if(!rateLimiter->isGlobalRateLimitHonored()) {
        LOGE("RTN_SERVER_REQUESTS",
             "Max Concurrent Requests Count hit, "  \
             "Request with handle = " + std::to_string(request->getHandle()) + " Dropped.");

        // Free the Request Memory Block
        Request::cleanUpRequest(request);
        return;
    }

    if(request->getRequestType() == REQ_RESOURCE_TUNING) {
        if(!ClientDataManager::getInstance()->clientExists(request->getClientPID(), request->getClientTID())) {
            if(!ClientDataManager::getInstance()->createNewClient(request->getClientPID(), request->getClientTID())) {
                // Client Entry Could not be Created, don't Proceed further with the Request
                LOGE("RTN_SERVER_REQUESTS",
                     "Client Entry could not be created for handle = " +
                     std::to_string(request->getHandle()));

                // Free the Request Memory Block
                Request::cleanUpRequest(request);
                return;
            }
        }
    }

    if(request->getRequestType() == REQ_RESOURCE_UNTUNING ||
       request->getRequestType() == REQ_RESOURCE_RETUNING) {
        // Note: Client Data Manager initialisation is not necessary for Untune / Retune Requests,
        // since the client is expected to be allocated already (as part of the Tune Request)
        if(request->getRequestType() == REQ_RESOURCE_UNTUNING) {
            // Update the Processing Status for this handle to false
            // This handles the Edge Case where the Client sends the Untune Request immediately.
            // Edge cases are listed below:
            // 1. Client sends the Untune Request immediately after sending the Tune Request,
            //    before a Client Data Entry is even created for the Tune Request.
            // 2. Tune and Untune Requests are sent concurrently and the Untune Request is picked
            //    up for Processing in the RequestQueue before the Tune Request is added to the RequestManager.
            RequestManager::getInstance()->disableRequestProcessing(request->getHandle());
        } else {
            // Done for handling Edge Cases,
            // Refer the comment in the if-block for more explanation
            RequestManager::getInstance()->modifyRequestDuration(request->getHandle(), request->getDuration());
        }

        if(!ClientDataManager::getInstance()->clientExists(request->getClientPID(), request->getClientTID())) {
            // Client does not exist, drop the request
            Request::cleanUpRequest(request);
            return;
        }
    }

    if(!rateLimiter->isRateLimitHonored(request->getClientTID())) {
        LOGI("RTN_SERVER_REQUESTS", "ClientTID: " + std::to_string(request->getClientTID()) + " Rate Limited");
        return;
    }

    // For requests of type - retune / untune
    // - Check if request with specified handle exists or not
    // For requests of type tune
    // - Check for duplicate requests, using the RequestManager
    // - Check if any of the outstanding requests of this client matches the current request
    if(request->getRequestType() == REQ_RESOURCE_UNTUNING ||
       request->getRequestType() == REQ_RESOURCE_RETUNING) {
        if(!RequestManager::getInstance()->verifyHandle(request->getHandle())) {
            LOGD("RTN_SERVER_REQUESTS", "No existing request with this handle found, dropping the request");
            Request::cleanUpRequest(request);
        } else {
            if(request->getRequestType() == REQ_RESOURCE_UNTUNING) {
                // Update the Processing Status for this handle to false
                RequestManager::getInstance()->disableRequestProcessing(request->getHandle());
            }
            // Add it to request queue for further processing
            RequestQueue::getInstance()->addAndWakeup(request);
        }

        return;
    }

    if(VerifyIncomingRequest(request)) {
        TYPELOGV(VERIFIER_REQUEST_VALIDATED, request->getHandle());

        if(RequestManager::getInstance()->shouldRequestBeAdded(request)) {
            RequestManager::getInstance()->addRequest(request);

            // Add this request to the RequestQueue
            RequestQueue::getInstance()->addAndWakeup(request);

        } else {
            LOGD("RTN_SERVER_REQUESTS", "Duplicate found, dropping request.");
            Request::cleanUpRequest(request);
        }

    } else {
        TYPELOGV(VERIFIER_STATUS_FAILURE, request->getHandle());
        Request::cleanUpRequest(request);
    }
}

void submitResourceProvisioningRequest(Request* request, int8_t isValidated) {
    processIncomingRequest(request, isValidated);
}

void submitResourceProvisioningRequest(void* msg) {
    MsgForwardInfo* info = (MsgForwardInfo*) msg;
    if(info == nullptr) return;

    Request* request = nullptr;
    try {
        request = new (GetBlock<Request>()) Request();

    } catch(const std::bad_alloc& e) {
        TYPELOGV(REQUEST_MEMORY_ALLOCATION_FAILURE, e.what());
        return;
    }

    if(RC_IS_NOTOK(request->deserialize(info->buffer))) {
        Request::cleanUpRequest(request);
        return;
    }

    if(request->getRequestType() == REQ_RESOURCE_TUNING) {
        request->setHandle(info->handle);
    }

    processIncomingRequest(request);
}

void toggleDisplayModes() {
    if(ResourceTunerSettings::targetConfigs.currMode & MODE_DISPLAY_ON) {
        // Toggle to Display Off
        ResourceTunerSettings::targetConfigs.currMode &= ~MODE_DISPLAY_ON;
        ResourceTunerSettings::targetConfigs.currMode |= MODE_DISPLAY_OFF;

        // First drain out the CocoTable, and move Requests to Pending Queue (which
        // cannot be processed in Background)
        RequestManager::getInstance()->triggerDisplayOffOrDozeMode();

    } else {
        // Toggle to Display On
        ResourceTunerSettings::targetConfigs.currMode &= ~MODE_DISPLAY_OFF;
        ResourceTunerSettings::targetConfigs.currMode |= MODE_DISPLAY_ON;

        // First drain out the CocoTable, and move all Requests to the Active Queue
        // from the Pending Queue.
        RequestManager::getInstance()->triggerDisplayOnMode();
    }

    // Reset All Resource Nodes to their original values
    ResourceRegistry::getInstance()->restoreResourcesToDefaultValues();

    // Restart Request Processing
    RequestManager::getInstance()->floodInRequestsForProcessing();
}

void RequestQueue::orderedQueueConsumerHook() {
    std::shared_ptr<RequestManager> requestManager = RequestManager::getInstance();
    std::shared_ptr<CocoTable> cocoTable = CocoTable::getInstance();

    while(this->hasPendingTasks()) {
        Message* message = this->pop();
        if(message == nullptr) {
            continue;
        }

        // This is a custom Request used to clean up the Server.
        if(message->getPriority() == SERVER_CLEANUP_TRIGGER_PRIORITY) {
            LOGI("RTN_SERVER_REQUESTS", "Called Cleanup Request");
            return;
        }

        Request* req = dynamic_cast<Request*>(message);
        if(req == nullptr) {
            LOGD("RTN_SERVER_REQUESTS",
                 "Message is Malformed, Downcasting to Request Type Failed");
            continue;
        }

        // Check for System Mode and Request Compatability
        uint8_t currentMode = ResourceTunerSettings::targetConfigs.currMode;
        if((currentMode == MODE_DISPLAY_OFF || currentMode == MODE_DOZE) && !req->isBackgroundProcessingEnabled()) {
            // Cannot continue with this Request
            LOGD("RTN_SERVER_REQUESTS", "Request cannot be processed in current mode");
            continue;
        }

        if(req->getRequestType() == REQ_RESOURCE_TUNING) {
            int64_t requestProcessingStatus = RequestManager::getInstance()->getRequestProcessingStatus(req->getHandle());
            // Find better status code
            if(requestProcessingStatus == REQ_CANCELLED || requestProcessingStatus == REQ_COMPLETED) {
                // Request has already been untuned or expired (Edge Cases)
                // No need to process it again.
                continue;
            }

            RequestManager::getInstance()->markRequestAsComplete(req->getHandle());
            if(!cocoTable->insertRequest(req)) {
                // Request could not be inserted, clean it up.
                Request::cleanUpRequest(req);
            }

        } else {
            // For Tune and Untune Requests, get the Corresponding Tune Request from the RequestManager
            Request* correspondingTuneRequest = requestManager->getRequestFromMap(req->getHandle());

            if(correspondingTuneRequest == nullptr) {
                // Note by this point, the Client is ascertained to be in the Client Data Manager Table
                LOGD("RTN_SERVER_REQUESTS", "Corresponding Tune Request Not Found, Dropping");
                continue;
            }

            if(correspondingTuneRequest->getClientPID() != req->getClientPID()) {
                LOGI("RTN_SERVER_REQUESTS",
                     "Corresponding Tune Request issued by different Client, Dropping Request.");

                // Free Up the Request
                Request::cleanUpRequest(req);
                return;
            }

            if(req->getRequestType() == REQ_RESOURCE_UNTUNING) {
                cocoTable->removeRequest(correspondingTuneRequest);
                RequestManager::getInstance()->removeRequest(correspondingTuneRequest);

                // Free Up the Untune Request
                Request::cleanUpRequest(req);

                // Free up the Corresponding Tune Request Resources
                Request::cleanUpRequest(correspondingTuneRequest);

            } else if(req->getRequestType() == REQ_RESOURCE_RETUNING) {
                int64_t newDuration = req->getDuration();
                cocoTable->updateRequest(correspondingTuneRequest, newDuration);

                // Free Up the Retune Request
                Request::cleanUpRequest(req);
            }
        }
    }
}

void* TunerServerThread() {
    std::shared_ptr<RequestQueue> requestQueue = RequestQueue::getInstance();
    while(ResourceTunerSettings::isServerOnline()) {
        requestQueue->wait();
    }

    return nullptr;
}
