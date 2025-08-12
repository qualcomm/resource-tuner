// Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
// SPDX-License-Identifier: BSD-3-Clause-Clear

#include "SignalServerPrivate.h"

static void dumpRequest(Signal* clientReq) {
    std::string LOG_TAG = "RTN_SERVER";
    LOGD(LOG_TAG, "Print Signal details:");

    LOGD(LOG_TAG, "Print Signal Request");
    LOGD(LOG_TAG, "Signal ID: " + std::to_string(clientReq->getSignalID()));
    LOGD(LOG_TAG, "Handle: " + std::to_string(clientReq->getHandle()));
    LOGD(LOG_TAG, "Duration: " + std::to_string(clientReq->getDuration()));
    LOGD(LOG_TAG, "App Name: " + std::string(clientReq->getAppName()));
    LOGD(LOG_TAG, "Scenario: " + std::string(clientReq->getScenario()));
    LOGD(LOG_TAG, "Num Args: " + std::to_string(clientReq->getNumArgs()));
    LOGD(LOG_TAG, "Priority: " + std::to_string(clientReq->getPriority()));
}

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

    // Basic sanity: Invalid resource Opcode
    if(signalInfo == nullptr) {
        LOGI("RTN_SERVER", "Invalid Opcode" + std::to_string(signal->getSignalID()));
        return false;
    }

    int8_t clientPermissions = ClientDataManager::getInstance()->getClientLevelByClientID(signal->getClientPID());
    if(clientPermissions == -1) {
        LOGI("RTN_SERVER", "Invalid Client Permissions");
        return false;
    }

    int8_t reqSpecifiedPriority = signal->getPriority();
    if(reqSpecifiedPriority > RequestPriority::REQ_PRIORITY_LOW ||
       reqSpecifiedPriority < RequestPriority::REQ_PRIORITY_HIGH) {
        LOGI("RTN_SERVER", "Priority Level = " +
             std::to_string(reqSpecifiedPriority) + "is not a valid value");
        return false;
    }

    int8_t allowedPriority = getRequestPriority(clientPermissions, reqSpecifiedPriority);
    if(allowedPriority == -1) return false;
    signal->setPriority(allowedPriority);

    // Check if the Signal is enabled for provisioning
    if(!signalInfo->mIsEnabled) {
        LOGI("RTN_SERVER", "Specified Signal is not enabled for provisioning");
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
        LOGI("RTN_SERVER",
             "Client does not have sufficient Permissions to provision Signal: " + std::to_string(signal->getSignalID()));
        return false;
    }

    // Target Compatability Checks
    std::string targetName = ResourceTunerSettings::targetConfigs.targetName;
    if(signalInfo->mTargetsEnabled != nullptr) {
        if(signalInfo->mTargetsEnabled->find(targetName) == signalInfo->mTargetsEnabled->end()) {
            LOGI("RTN_SERVER", "Specified Signal is not enabled for provisioning on this Target");
            return false;
        }
    } else if(signalInfo->mTargetsDisabled != nullptr) {
        if(signalInfo->mTargetsDisabled->find(targetName) != signalInfo->mTargetsDisabled->end()) {
            LOGI("RTN_SERVER", "Specified Signal is not enabled for provisioning on this Target");
            return false;
        }
    }

    if(signal->getRequestType() == SIGNAL_RELAY) {
        LOGI("RTN_SERVER", "Request Verified");
        return true;
    }

    // Perform Resource Level Checking, using the ResourceRegistry
    for(int32_t i = 0; i < signalInfo->mSignalResources->size(); i++) {
        Resource* resource = signalInfo->mSignalResources->at(i);
        ResourceConfigInfo* resourceConfig = ResourceRegistry::getInstance()->getResourceById(resource->getOpCode());

        // Basic sanity: Invalid resource Opcode
        if(resourceConfig == nullptr) {
            LOGE("RTN_SERVER","Invalid Opcode, Dropping Request");
            return false;
        }

        // Verify value is in the range [LT, HT]
        if(resource->getValuesCount() == 1) {
            int32_t configValue = resource->mConfigValue.singleValue;
            int32_t lowThreshold = resourceConfig->mLowThreshold;
            int32_t highThreshold = resourceConfig->mHighThreshold;

            if(configValue < lowThreshold || configValue > highThreshold) {
                LOGE("RTN_SERVER", "Range Checking Failed, Dropping Request");
                return false;
            }
        } else {
            // Note: Extend this verification for multiple values
        }

        // Verify tuning is supported for the resource in question
        if(!resourceConfig->mSupported) return false;

        // Check for Client permissions
        if(resourceConfig->mPermissions == PERMISSION_SYSTEM && clientPermissions == PERMISSION_THIRD_PARTY) {
            LOGI("RTN_SERVER", "Permission Check Failed, Dropping Request");
            return false;
        }
    }

    if(signal->getDuration() == 0) {
        // If the Client has not specified a duration to acquire the Signal for,
        // We use the default duration for the Signal, specified in the Signal
        // Configs (YAML) file.
        signal->setDuration(signalInfo->mTimeout);
    }

    LOGI("RTN_SERVER", "Request Verified");
    return true;
}

static void processIncomingRequest(Signal* signal) {
    if(signal->getRequestType() == SIGNAL_RELAY || signal->getRequestType() == SIGNAL_ACQ) {
        // Check if the client exists, if not create a new client tracking entry
        if(!ClientDataManager::getInstance()->clientExists(signal->getClientPID(), signal->getClientTID())) {
            ClientDataManager::getInstance()->createNewClient(signal->getClientPID(), signal->getClientTID());
        }
    } else {
        // SIGNAL_FREE request
        if(!ClientDataManager::getInstance()->clientExists(signal->getClientPID(), signal->getClientTID())) {
            // Client does not exist, drop the request
            FreeBlock<Signal>(static_cast<void*>(signal));
            return;
        }
    }

    // Rate Check the client
    std::shared_ptr<RateLimiter> rateLimiter = RateLimiter::getInstance();
    if(!rateLimiter->isRateLimitHonored(signal->getClientTID())) {
        LOGI("RTN_SERVER", "ClientTID: " + std::to_string(signal->getClientTID()) + " Rate Limited");
        return;
    }

    if(signal->getRequestType() == SIGNAL_ACQ || signal->getRequestType() == SIGNAL_RELAY) {
        if(!VerifyIncomingRequest(signal)) {
            LOGE("RTN_SERVER", "Signal Request verification failed, dropping request");
            FreeBlock<Signal>(static_cast<void*>(signal));
            return;
        }
    }

    SignalQueue::getInstance()->addAndWakeup(signal);
}

void submitSignalRequest(void* clientReq) {
    MsgForwardInfo* info = (MsgForwardInfo*) clientReq;
    if(info == nullptr) return;

    Signal* signal = nullptr;
    try {
        signal = new (GetBlock<Signal>()) Signal();

    } catch(const std::bad_alloc& e) {
        TYPELOGV(REQUEST_MEMORY_ALLOCATION_FAILURE, e.what());
        return;
    }

    if(RC_IS_NOTOK(signal->deserialize(info->buffer))) {
        Signal::cleanUpSignal(signal);
        return;
    }

    if(signal->getRequestType() == SIGNAL_ACQ) {
        signal->setHandle(info->handle);
    }

    processIncomingRequest(signal);
}

static Request* createResourceTuningRequest(Signal* signal) {
    Request* request = nullptr;

    try {
        request = new (GetBlock<Request>()) Request();

    } catch(const std::bad_alloc& e) {
        LOGE("RTN_SERVER",
             "Memory allocation for Request struct corresponding to Signal Req with handle: " +
             std::to_string(signal->getHandle()) +
             " failed with error: " + std::string(e.what()) + ". Dropping this Request");
        return nullptr;
    }

    request->setRequestType(REQ_RESOURCE_TUNING);
    request->setHandle(signal->getHandle());
    request->setDuration(signal->getDuration());
    request->setProperties(signal->getProperties());
    request->setClientPID(signal->getClientPID());
    request->setClientTID(signal->getClientTID());

    SignalInfo* signalInfo = SignalRegistry::getInstance()->getSignalConfigById(signal->getSignalID());

    if(signalInfo == nullptr) {
        FreeBlock<Request>(static_cast<void*>(request));
        return nullptr;
    }

    std::vector<Resource*>* signalLocks = signalInfo->mSignalResources;
    request->setNumResources(signalLocks->size() / 2);

    std::vector<Resource*>* resourceList =
        new (GetBlock<std::vector<Resource*>>()) std::vector<Resource*>;
    resourceList->resize(request->getResourcesCount());

    for(int32_t i = 0; i < signalLocks->size(); i++) {
        Resource* resource = (*signalLocks)[i];
        resourceList->push_back(resource);
    }

    request->setResources(resourceList);

    return request;
}

static Request* createResourceUntuneRequest(Signal* signal) {
    Request* request = nullptr;

    try {
        request = new(GetBlock<Request>()) Request();

    } catch(const std::bad_alloc& e) {
        LOGE("RTN_SERVER",
             "Memory allocation for Request struct corresponding to Signal Req with handle: " +
             std::to_string(signal->getHandle()) +
             " failed with error: " + std::string(e.what()) + ". Dropping this Request");
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
            LOGI("RTN_SERVER", "Called Cleanup Request");
            return;
        }

        Signal* signal = dynamic_cast<Signal*>(message);
        if(signal == nullptr) {
            LOGD("RTN_SERVER",
                 "Message is Malformed, Downcasting to Signal Type Failed");
            continue;
        }

        switch(signal->getRequestType()) {
            case SIGNAL_ACQ: {
                Request* request = createResourceTuningRequest(signal);
                FreeBlock<Signal>(static_cast<void*>(signal));

                // Submit the Resource Provisioning request for processing
                if(request != nullptr) {
                    submitResourceProvisioningRequest(request, true);
                }
                break;
            }
            case SIGNAL_FREE: {
                Request* request = createResourceUntuneRequest(signal);
                FreeBlock<Signal>(static_cast<void*>(signal));

                // Submit the Resource De-Provisioning request for processing
                if(request != nullptr) {
                    submitResourceProvisioningRequest(request, true);
                }
                break;
            }
            case SIGNAL_RELAY: {
                // Get all the subscribed Features
                std::vector<int32_t> subscribedFeatures;
                int8_t status = SignalExtFeatureMapper::getInstance()->getFeatures(signal->getSignalID(), subscribedFeatures);

                if(!status) {
                    LOGE("RTN_SERVER", "No signal with the specified ID exists, dropping the request");
                    break;
                }

                for(int32_t featureId: subscribedFeatures) {
                    // Fetch This Feature from ExtFeaturesRegistry
                    ExtFeatureInfo* featureInfo =
                        ExtFeaturesRegistry::getInstance()->getExtFeatureConfigById(featureId);
                    if(featureInfo == nullptr) {
                        continue;
                    }

                    // Relay the Signal to all subscribed features.
                }

                FreeBlock<Signal>(static_cast<void*>(signal));
                break;
            }
            default:
                break;
        }
    }
}

void* SignalsdServerThread() {
    LOGD("RTN_SERVER", "SysSignal Thread started");

    std::shared_ptr<SignalQueue> signalQueue = SignalQueue::getInstance();
    while(ResourceTunerSettings::isServerOnline()) {
        signalQueue->wait();
    }

    return nullptr;
}
