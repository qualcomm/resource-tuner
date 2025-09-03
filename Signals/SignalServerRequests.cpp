// Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
// SPDX-License-Identifier: BSD-3-Clause-Clear

#include "SignalInternal.h"

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
 * @brief Verifies the validity of an incoming Signal.
 *
 * @details This function checks the incoming Signal request against the Signal Registry.
 *          It ensures that the Signal exists in the registry and that the request is valid.
 *
 * @param req Pointer to the Request object.
 * @return int8_t True if the request is valid, false otherwise.
 */
static int8_t VerifyIncomingRequest(Signal* signal) {
    // Check if a Signal with the given ID exists in the Registry
    SignalInfo* signalInfo = SignalRegistry::getInstance()->getSignalConfigById(signal->getSignalID());

    // Basic sanity: Invalid ResCode
    if(signalInfo == nullptr) {
        TYPELOGV(VERIFIER_INVALID_OPCODE, signal->getSignalID());
        return false;
    }

    int8_t clientPermissions = ClientDataManager::getInstance()->getClientLevelByClientID(signal->getClientPID());
    if(clientPermissions == -1) {
        TYPELOGV(VERIFIER_INVALID_PERMISSION, signal->getClientPID(), signal->getClientTID());
        return false;
    }

    int8_t reqSpecifiedPriority = signal->getPriority();
    if(reqSpecifiedPriority > RequestPriority::REQ_PRIORITY_LOW ||
       reqSpecifiedPriority < RequestPriority::REQ_PRIORITY_HIGH) {
        TYPELOGV(VERIFIER_INVALID_PRIORITY, reqSpecifiedPriority);
        return false;
    }

    int8_t allowedPriority = getRequestPriority(clientPermissions, reqSpecifiedPriority);
    if(allowedPriority == -1) return false;
    signal->setPriority(allowedPriority);

    // Check if the Signal is enabled for provisioning
    if(!signalInfo->mIsEnabled) {
        TYPELOGV(VERIFIER_UNSUPPORTED_SIGNAL_TUNING, signal->getSignalID());
        return false;
    }

    int8_t permissionCheck = false;
    int8_t requestPermission = ClientDataManager::getInstance()->getClientLevelByClientID(signal->getClientPID());
    for(enum Permissions signalPermission: *signalInfo->mPermissions) {
        if(requestPermission == signalPermission) {
            permissionCheck = true;
            break;
        }
    }

    // Client does not have the necessary permissions to tune this Resource.
    if(!permissionCheck) {
        TYPELOGV(VERIFIER_NOT_SUFFICIENT_SIGNAL_ACQ_PERMISSION, signal->getSignalID());
        return false;
    }

    // Target Compatability Checks
    std::string targetName = ResourceTunerSettings::targetConfigs.targetName;
    if(signalInfo->mTargetsEnabled != nullptr) {
        if(signalInfo->mTargetsEnabled->find(targetName) == signalInfo->mTargetsEnabled->end()) {
            TYPELOGV(VERIFIER_TARGET_CHECK_FAILED, signal->getSignalID());
            return false;
        }
    } else if(signalInfo->mTargetsDisabled != nullptr) {
        if(signalInfo->mTargetsDisabled->find(targetName) != signalInfo->mTargetsDisabled->end()) {
            TYPELOGV(VERIFIER_TARGET_CHECK_FAILED, signal->getSignalID());
            return false;
        }
    }

    if(signal->getRequestType() == REQ_SIGNAL_RELAY) {
        TYPELOGV(VERIFIER_REQUEST_VALIDATED, signal->getHandle());
        return true;
    }

    // Perform Resource Level Checking, using the ResourceRegistry
    for(int32_t i = 0; i < signalInfo->mSignalResources->size(); i++) {
        Resource* resource = signalInfo->mSignalResources->at(i);
        ResourceConfigInfo* resourceConfig = ResourceRegistry::getInstance()->getResourceById(resource->getResCode());

        // Basic sanity: Invalid ResCode
        if(resourceConfig == nullptr) {
            TYPELOGV(VERIFIER_INVALID_OPCODE, resource->getResCode());
            return false;
        }

        // Verify value is in the range [LT, HT]
        if(resource->getValuesCount() == 1) {
            int32_t configValue = resource->mResValue.value;
            int32_t lowThreshold = resourceConfig->mLowThreshold;
            int32_t highThreshold = resourceConfig->mHighThreshold;

            if((lowThreshold != -1 && highThreshold != -1) &&
                (configValue < lowThreshold || configValue > highThreshold)) {
                TYPELOGV(VERIFIER_VALUE_OUT_OF_BOUNDS, configValue, resource->getResCode());
                return false;
            }
        } else {
            // Note: Extend this verification for multiple values
        }

        // Verify tuning is supported for the resource in question
        if(!resourceConfig->mSupported) return false;

        // Check for Client permissions
        if(resourceConfig->mPermissions == PERMISSION_SYSTEM && clientPermissions == PERMISSION_THIRD_PARTY) {
            TYPELOGV(VERIFIER_NOT_SUFFICIENT_PERMISSION, resource->getResCode());
            return false;
        }
    }

    if(signal->getDuration() == 0) {
        // If the Client has not specified a duration to tune the Signal for,
        // We use the default duration for the Signal, specified in the Signal
        // Configs (YAML) file.
        signal->setDuration(signalInfo->mTimeout);
    }

    TYPELOGV(VERIFIER_REQUEST_VALIDATED, signal->getHandle());
    return true;
}

// Fills in any optional fields in the Signal that are not specified in the Config file
// with the values specified in the tuneSignal API's list argument.
static int8_t fillDefaults(Signal* signal) {
    uint32_t signalID = signal->getSignalID();
    SignalInfo* signalInfo = SignalRegistry::getInstance()->getSignalConfigById(signalID);
    if(signalInfo == nullptr) return false;
    if(signalInfo->mSignalResources == nullptr) return true;

    int32_t listIndex = 0;
    for(Resource* resource : (*signalInfo->mSignalResources)) {
        int32_t valueCount = resource->getValuesCount();
        if(valueCount == 1) {
            if(resource->mResValue.value == -1) {
                if(signal->getListArgs() == nullptr) return false;
                resource->mResValue.value = signal->getListArgAt(listIndex);
                listIndex++;
            }
        } else {
            for(int32_t i = 0; i < valueCount; i++) {
                if((*resource->mResValue.values)[i] == -1) {
                    if(signal->getListArgs() == nullptr) return false;
                    if(listIndex >= 0 && listIndex < signal->getNumArgs()) {
                        (*resource->mResValue.values)[i] = signal->getListArgAt(listIndex);
                        listIndex++;
                    } else {
                        return false;
                    }
                }
            }
        }
    }

    return true;
}

static void processIncomingRequest(Signal* signal) {
    std::shared_ptr<RateLimiter> rateLimiter = RateLimiter::getInstance();
    std::shared_ptr<ClientDataManager> clientDataManager = ClientDataManager::getInstance();

    if(!rateLimiter->isGlobalRateLimitHonored()) {
        TYPELOGV(RATE_LIMITER_GLOBAL_RATE_LIMIT_HIT, signal->getHandle());
        // Free the Signal Memory Block
        Signal::cleanUpSignal(signal);
        return;
    }

    if(signal->getRequestType() == REQ_SIGNAL_RELAY || signal->getRequestType() == REQ_SIGNAL_TUNING) {
        // Check if the client exists, if not create a new client tracking entry
        if(!clientDataManager->clientExists(signal->getClientPID(), signal->getClientTID())) {
            if(!clientDataManager->createNewClient(signal->getClientPID(), signal->getClientTID())) {
                // Failed to create a tracking entry, drop the Request.
                TYPELOGV(CLIENT_ENTRY_CREATION_FAILURE, signal->getHandle());

                // Free the Signal Memory Block
                Signal::cleanUpSignal(signal);
                return;
            }
        }
    } else {
        // In case of untune Requests, the Client should already exist
        if(!clientDataManager->clientExists(signal->getClientPID(), signal->getClientTID())) {
            // Client does not exist, drop the request
            Signal::cleanUpSignal(signal);
            return;
        }
    }

    // Rate Check the client
    if(!rateLimiter->isRateLimitHonored(signal->getClientTID())) {
        TYPELOGV(RATE_LIMITER_RATE_LIMITED, signal->getClientTID(), signal->getHandle());
        Signal::cleanUpSignal(signal);
        return;
    }

    // Fill any Placeholders in the Signal Config
    if(!fillDefaults(signal)) {
        Signal::cleanUpSignal(signal);
        return;
    }

    if(signal->getRequestType() == REQ_SIGNAL_TUNING) {
        if(!VerifyIncomingRequest(signal)) {
            TYPELOGV(VERIFIER_STATUS_FAILURE, signal->getHandle());
            Signal::cleanUpSignal(signal);
            return;
        }
    }

    // Add the Signal to the SignalQueue
    SignalQueue::getInstance()->addAndWakeup(signal);
}

ErrCode submitSignalRequest(void* clientReq) {
    MsgForwardInfo* info = (MsgForwardInfo*) clientReq;
    if(info == nullptr) return RC_BAD_ARG;

    Signal* signal = nullptr;
    try {
        signal = new (GetBlock<Signal>()) Signal();

    } catch(const std::bad_alloc& e) {
        TYPELOGV(REQUEST_MEMORY_ALLOCATION_FAILURE, e.what());
        return RC_MEMORY_ALLOCATION_FAILURE;
    }

    if(RC_IS_NOTOK(signal->deserialize(info->buffer))) {
        Signal::cleanUpSignal(signal);
        return RC_REQUEST_DESERIALIZATION_FAILURE;
    }

    if(signal->getRequestType() == REQ_SIGNAL_TUNING) {
        signal->setHandle(info->handle);
    }

    processIncomingRequest(signal);
    return RC_SUCCESS;
}

static Request* createResourceTuningRequest(Signal* signal) {
    try {
        SignalInfo* signalInfo = SignalRegistry::getInstance()->getSignalConfigById(signal->getSignalID());
        if(signalInfo == nullptr) return nullptr;

        Request* request = new (GetBlock<Request>()) Request();

        request->setRequestType(REQ_RESOURCE_TUNING);
        request->setHandle(signal->getHandle());
        request->setDuration(signal->getDuration());
        request->setProperties(signal->getProperties());
        request->setClientPID(signal->getClientPID());
        request->setClientTID(signal->getClientTID());

        std::vector<Resource*>* signalLocks = signalInfo->mSignalResources;
        request->setNumResources(signalLocks->size());

        std::vector<Resource*>* resourceList =
            new (GetBlock<std::vector<Resource*>>()) std::vector<Resource*>;
        resourceList->resize(request->getResourcesCount());

        for(int32_t i = 0; i < signalLocks->size(); i++) {
            (*resourceList)[i] = (*signalLocks)[i];
        }

        request->setResources(resourceList);
        return request;

    } catch(const std::bad_alloc& e) {
        TYPELOGV(REQUEST_MEMORY_ALLOCATION_FAILURE_HANDLE, signal->getHandle(), e.what());
        return nullptr;
    }

    return nullptr;
}

static Request* createResourceUntuneRequest(Signal* signal) {
    Request* request = nullptr;

    try {
        request = new(GetBlock<Request>()) Request();

    } catch(const std::bad_alloc& e) {
        TYPELOGV(REQUEST_MEMORY_ALLOCATION_FAILURE_HANDLE, signal->getHandle(), e.what());
        return nullptr;
    }

    request->setRequestType(REQ_RESOURCE_UNTUNING);
    request->setHandle(signal->getHandle());
    request->setDuration(-1);
    request->setProperties(signal->getProperties());
    request->setClientPID(signal->getClientPID());
    request->setClientTID(signal->getClientTID());
    request->setNumResources(0);

    return request;
}

void SignalQueue::orderedQueueConsumerHook() {
    while(this->hasPendingTasks()) {
        Message* message = this->pop();
        if(message == nullptr) {
            continue;
        }

        // This is a custom Request used to clean up the Server.
        if(message->getPriority() == SERVER_CLEANUP_TRIGGER_PRIORITY) {
            LOGI("RESTUNE_SERVER", "Called Cleanup Request");
            return;
        }

        Signal* signal = dynamic_cast<Signal*>(message);
        if(signal == nullptr) {
            continue;
        }

        switch(signal->getRequestType()) {
            case REQ_SIGNAL_TUNING: {
                Request* request = createResourceTuningRequest(signal);
                FreeBlock<Signal>(static_cast<void*>(signal));

                // Submit the Resource Provisioning request for processing
                if(request != nullptr) {
                    submitResProvisionRequest(request, true);
                }
                break;
            }
            case REQ_SIGNAL_UNTUNING: {
                Request* request = createResourceUntuneRequest(signal);
                FreeBlock<Signal>(static_cast<void*>(signal));

                // Submit the Resource De-Provisioning request for processing
                if(request != nullptr) {
                    submitResProvisionRequest(request, true);
                }
                break;
            }
            case REQ_SIGNAL_RELAY: {
                // Get all the subscribed Features
                std::vector<uint32_t> subscribedFeatures;
                int8_t featuresExist = SignalExtFeatureMapper::getInstance()->getFeatures(
                    signal->getSignalID(),
                    subscribedFeatures
                );

                if(featuresExist == false) {
                    Signal::cleanUpSignal(signal);
                    continue;
                }

                for(uint32_t featureId: subscribedFeatures) {
                    ExtFeaturesRegistry::getInstance()->relayToFeature(featureId, signal);
                }

                Signal::cleanUpSignal(signal);
                break;
            }
            default:
                break;
        }
    }
}
