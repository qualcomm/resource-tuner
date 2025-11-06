// Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
// SPDX-License-Identifier: BSD-3-Clause-Clear

#include "SignalQueue.h"

static Request* createResourceTuningRequest(Signal* signal) {
    try {
        SignalInfo* signalInfo = SignalRegistry::getInstance()->getSignalConfigById(signal->getSignalCode());
        if(signalInfo == nullptr) return nullptr;

        Request* request = MPLACED(Request);

        request->setRequestType(REQ_RESOURCE_TUNING);
        request->setHandle(signal->getHandle());
        request->setDuration(signal->getDuration());
        request->setProperties(signal->getProperties());
        request->setClientPID(signal->getClientPID());
        request->setClientTID(signal->getClientTID());

        std::vector<Resource*>* signalLocks = signalInfo->mSignalResources;

        int32_t listIndex = 0;
        for(int32_t i = 0; i < signalLocks->size(); i++) {
            if((*signalLocks)[i] == nullptr) {
                continue;
            }

            // Copy
            Resource* resource = MPLACEV(Resource, (*((*signalLocks)[i])));

            // fill placeholders if any
            int32_t valueCount = resource->getValuesCount();
            if(valueCount == 1) {
                if(resource->mResValue.value == -1) {
                    if(signal->getListArgs() == nullptr) return nullptr;
                    if(listIndex < signal->getNumArgs()) {
                        resource->mResValue.value = signal->getListArgAt(listIndex);
                        listIndex++;
                    } else {
                        return nullptr;
                    }
                }
            } else {
                for(int32_t i = 0; i < valueCount; i++) {
                    if((*resource->mResValue.values)[i] == -1) {
                        if(signal->getListArgs() == nullptr) return nullptr;
                        if(listIndex >= 0 && listIndex < signal->getNumArgs()) {
                            (*resource->mResValue.values)[i] = signal->getListArgAt(listIndex);
                            listIndex++;
                        } else {
                            return nullptr;
                        }
                    }
                }
            }

            CoreIterable* resIterable = MPLACED(CoreIterable);
            resIterable->mData = resource;
            request->addResource(resIterable);
        }

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
        request = MPLACED(Request);

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

    return request;
}

std::shared_ptr<SignalQueue> SignalQueue::mSignalQueueInstance = nullptr;
std::mutex SignalQueue::instanceProtectionLock{};

SignalQueue::SignalQueue() {}

void SignalQueue::orderedQueueConsumerHook() {
    while(this->hasPendingTasks()) {
        Message* message = this->pop();
        if(message == nullptr) {
            continue;
        }

        // This is a custom Request used to clean up the Server.
        if(message->getPriority() == SERVER_CLEANUP_TRIGGER_PRIORITY) {
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
                } else {
                    LOGE("RESTUNE_SIGNAL_QUEUE", "Malformd Signal Request");
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
                int8_t featureExist = SignalExtFeatureMapper::getInstance()->getFeatures(
                    signal->getSignalCode(),
                    subscribedFeatures
                );

                if(featureExist) {
                    // Relay
                    for(uint32_t featureId: subscribedFeatures) {
                        ExtFeaturesRegistry::getInstance()->relayToFeature(featureId, signal);
                    }
                }

                Signal::cleanUpSignal(signal);
                break;
            }
            default:
                break;
        }
    }
}

SignalQueue::~SignalQueue() {}
