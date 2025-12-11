// Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
// SPDX-License-Identifier: BSD-3-Clause-Clear

#include "RequestQueue.h"

std::shared_ptr<RequestQueue> RequestQueue::mRequestQueueInstance = nullptr;
std::mutex RequestQueue::instanceProtectionLock{};

RequestQueue::RequestQueue() {}

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
            return;
        }

        Request* req = dynamic_cast<Request*>(message);
        if(req == nullptr) {
            continue;
        }

        if(req->getRequestType() == REQ_RESOURCE_TUNING) {
            int64_t requestProcessingStatus = requestManager->getRequestProcessingStatus(req->getHandle());
            // Find better status code
            if(requestProcessingStatus == REQ_CANCELLED || requestProcessingStatus == REQ_COMPLETED) {
                // Request has already been untuned or expired (Edge Cases)
                // No need to process it again.
                Request::cleanUpRequest(req);
                continue;
            }

            requestManager->markRequestAsComplete(req->getHandle());
            if(!cocoTable->insertRequest(req)) {
                // Request could not be inserted, clean it up.
                Request::cleanUpRequest(req);
                continue;
            }

        } else {
            // For Tune and Untune Requests, get the Corresponding Tune Request from the RequestManager
            Request* correspondingTuneRequest = requestManager->getRequestFromMap(req->getHandle());

            if(correspondingTuneRequest == nullptr) {
                // Note by this point, the Client is ascertained to be in the Client Data Manager Table
                LOGD("RESTUNE_SERVER_REQUESTS", "Corresponding Tune Request Not Found, Dropping");

                Request::cleanUpRequest(req);
                continue;
            }

            if(correspondingTuneRequest->getClientPID() != req->getClientPID()) {
                LOGI("RESTUNE_SERVER_REQUESTS",
                     "Corresponding Tune Request issued by different Client, Dropping Request.");

                // Free Up the Request
                Request::cleanUpRequest(req);
                return;
            }

            if(req->getRequestType() == REQ_RESOURCE_UNTUNING) {
                cocoTable->removeRequest(correspondingTuneRequest);
                // If this is an untune request issued during SUSPEND mode transition
                // Then don't cleanup the corresponding Tune Request, as it is still valid.
                requestManager->removeRequest(correspondingTuneRequest);
                if(req->getPriority() != HIGH_TRANSFER_PRIORITY) {
                    // Free up the Corresponding Tune Request Resources
                    Request::cleanUpRequest(correspondingTuneRequest);
                }

                // Free Up the Untune Request
                Request::cleanUpRequest(req);

            } else if(req->getRequestType() == REQ_RESOURCE_RETUNING) {
                int64_t newDuration = req->getDuration();
                cocoTable->updateRequest(correspondingTuneRequest, newDuration);

                // Free Up the Retune Request
                Request::cleanUpRequest(req);
            }
        }
    }
}

RequestQueue::~RequestQueue() {}
