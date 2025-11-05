// Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
// SPDX-License-Identifier: BSD-3-Clause-Clear

#include "SignalInternal.h"

static int8_t performPhysicalMapping(int32_t& coreValue, int32_t& clusterValue) {
    std::shared_ptr<TargetRegistry> targetRegistry = TargetRegistry::getInstance();
    if(targetRegistry == nullptr) return false;

    int32_t physicalClusterValue = targetRegistry->getPhysicalClusterId(clusterValue);
    int32_t physicalCoreValue = 0;

    // For resources with ApplyType == "core":
    // A coreValue of 0, indicates apply the config value to all the cores part of the physical
    // cluster corresponding to the specified logical cluster ID.
    if(coreValue != 0) {
        // if a non-zero coreValue is provided, translate it and apply the config value
        // only to that physical core's resource node.
        physicalCoreValue = targetRegistry->getPhysicalCoreId(clusterValue, coreValue);
    }

    if(physicalCoreValue != -1 && physicalClusterValue != -1) {
        coreValue = physicalCoreValue;
        clusterValue = physicalClusterValue;
    } else {
        return false;
    }

    return true;
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
 * @return int8_t:\n
 *            - 1: if the request is valid.
 *            - 0: otherwise.
 */
static int8_t VerifyIncomingRequest(Signal* signal) {
    // Check if a Signal with the given ID exists in the Registry
    SignalInfo* signalInfo = SignalRegistry::getInstance()->getSignalConfigById(signal->getSignalCode());

    // Basic sanity: Invalid ResCode
    if(signalInfo == nullptr) {
        TYPELOGV(VERIFIER_INVALID_OPCODE, signal->getSignalCode());
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
        TYPELOGV(VERIFIER_UNSUPPORTED_SIGNAL_TUNING, signal->getSignalCode());
        return false;
    }

    int8_t permissionCheck = false;
    for(enum Permissions signalPermission: *signalInfo->mPermissions) {
        if(clientPermissions == signalPermission) {
            permissionCheck = true;
            break;
        }
    }

    // Client does not have the necessary permissions to tune this Resource.
    if(!permissionCheck) {
        TYPELOGV(VERIFIER_NOT_SUFFICIENT_SIGNAL_ACQ_PERMISSION, signal->getSignalCode());
        return false;
    }

    // Target Compatability Checks
    std::string targetName = ResourceTunerSettings::targetConfigs.targetName;
    if(signalInfo->mTargetsEnabled != nullptr) {
        if(signalInfo->mTargetsEnabled->find(targetName) == signalInfo->mTargetsEnabled->end()) {
            TYPELOGV(VERIFIER_TARGET_CHECK_FAILED, signal->getSignalCode());
            return false;
        }
    } else if(signalInfo->mTargetsDisabled != nullptr) {
        if(signalInfo->mTargetsDisabled->find(targetName) != signalInfo->mTargetsDisabled->end()) {
            TYPELOGV(VERIFIER_TARGET_CHECK_FAILED, signal->getSignalCode());
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
        ResConfInfo* resourceConfig = ResourceRegistry::getInstance()->getResConf(resource->getResCode());

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

        // If ApplyType for the Resource is set to Core or Cluster, then perform Logical to Physical Translation
        if(resourceConfig->mApplyType == ResourceApplyType::APPLY_CORE) {
            // Check for invalid Core / cluster values, these are the logical values
            int32_t coreValue = resource->getCoreValue();
            int32_t clusterValue = resource->getClusterValue();

            if(coreValue < 0) {
                TYPELOGV(VERIFIER_INVALID_LOGICAL_CORE, coreValue);
                return false;
            }

            if(clusterValue < 0) {
                TYPELOGV(VERIFIER_INVALID_LOGICAL_CLUSTER, clusterValue);
                return false;
            }

            // Perform logical to physical mapping here, as part of which verification can happen
            // Replace mResInfo with the Physical values here:
            if(!performPhysicalMapping(coreValue, clusterValue)) {
                TYPELOGV(VERIFIER_LOGICAL_TO_PHYSICAL_MAPPING_FAILED, resource->getResCode());
                return false;
            }

            resource->setCoreValue(coreValue);
            resource->setClusterValue(clusterValue);

        } else if(resourceConfig->mApplyType == ResourceApplyType::APPLY_CLUSTER) {
            // Check for invalid Core / cluster values, these are the logical values
            int32_t clusterValue = resource->getClusterValue();

            if(clusterValue < 0) {
                TYPELOGV(VERIFIER_INVALID_LOGICAL_CLUSTER, clusterValue);
                return false;
            }

            // Perform logical to physical mapping here, as part of which verification can happen
            // Replace mResInfo with the Physical values here:
            int32_t physicalClusterID = TargetRegistry::getInstance()->getPhysicalClusterId(clusterValue);
            if(physicalClusterID == -1) {
                TYPELOGV(VERIFIER_LOGICAL_TO_PHYSICAL_MAPPING_FAILED, resource->getResCode());
                return false;
            }

            resource->setClusterValue(physicalClusterID);
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

ErrCode submitSignalRequest(void* msg) {
    if(msg == nullptr) return RC_BAD_ARG;

    ErrCode opStatus = RC_SUCCESS;
    MsgForwardInfo* info = (MsgForwardInfo*) msg;
    Signal* signal = nullptr;

    if(RC_IS_OK(opStatus)) {
        if(info == nullptr) {
            opStatus = RC_BAD_ARG;
        }
    }

    if(RC_IS_OK(opStatus)) {
        try {
            signal = MPLACED(Signal);
            opStatus = signal->deserialize(info->buffer);
            if(RC_IS_NOTOK(opStatus)) {
                Signal::cleanUpSignal(signal);
            }

        } catch(const std::bad_alloc& e) {
            TYPELOGV(REQUEST_MEMORY_ALLOCATION_FAILURE, e.what());
            opStatus = RC_MEMORY_ALLOCATION_FAILURE;
        }
    }

    if(RC_IS_OK(opStatus)) {
        if(signal->getRequestType() == REQ_SIGNAL_TUNING) {
            signal->setHandle(info->handle);
        }

        processIncomingRequest(signal);
    }

    if(info != nullptr) {
        FreeBlock<char[REQ_BUFFER_SIZE]>(info->buffer);
        FreeBlock<MsgForwardInfo>(info);
    }

    return RC_SUCCESS;
}
