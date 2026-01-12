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
            int8_t requestProcessingStatus = requestManager->getRequestProcessingStatus(req->getHandle());
            if((requestProcessingStatus & REQ_CANCELLED) || (requestProcessingStatus & REQ_COMPLETED)) {
                // Request has already been untuned or expired (Edge Cases)
                // No need to process it again.

                // Remove from RequestManager
                requestManager->removeRequest(req);
                Request::cleanUpRequest(req);
                continue;
            }

            requestManager->markRequestAsComplete(req->getHandle());

            if(!cocoTable->insertRequest(req)) {
                // Request could not be inserted, clean it up.
                requestManager->removeRequest(req);
                Request::cleanUpRequest(req);
                continue;
            }

        } else {
            // For Tune and Untune Requests, get the Corresponding Tune Request from the RequestManager
            RequestInfo matchingTuneReq = requestManager->getRequestFromMap(req->getHandle());

            int8_t processingStatus = matchingTuneReq.second;
            if(matchingTuneReq.first == nullptr || (processingStatus & REQ_NOT_FOUND)) {
                // Note by this point, the Client is ascertained to be in the Client Data Manager Table

                Request::cleanUpRequest(req);
                continue;
            }

            if(req->getRequestType() == REQ_RESOURCE_UNTUNING) {
                // Request is in RM, ensure it has entered Coco Table before issuing untune.
                if((processingStatus & REQ_COMPLETED) == 0) {
                    Request::cleanUpRequest(req);
                    continue;
                }

                cocoTable->removeRequest(matchingTuneReq.first);
                requestManager->removeRequest(matchingTuneReq.first);

                // Free Up the Untune Request
                Request::cleanUpRequest(req);
                Request::cleanUpRequest(matchingTuneReq.first);

            } else if(req->getRequestType() == REQ_RESOURCE_RETUNING) {
                int64_t newDuration = req->getDuration();
                cocoTable->updateRequest(matchingTuneReq.first, newDuration);

                // Free Up the Retune Request
                Request::cleanUpRequest(req);
            }
        }
    }
}

RequestQueue::~RequestQueue() {}
