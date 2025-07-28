// Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
// SPDX-License-Identifier: BSD-3-Clause-Clear

#include "Request.h"

Request::Request() {
    this->mResources = nullptr;
    this->mCocoNodes = nullptr;
    this->mTimer = nullptr;
}

int32_t Request::getResourcesCount() {
    return this->mNumResources;
}

int32_t Request::getCocoNodesCount() {
    return this->mNumCocoNodes;
}

int8_t Request::isBackgroundProcessingEnabled() {
    return this->mBackgroundProcessing;
}

std::vector<Resource*>* Request::getResources() {
    return this->mResources;
}

Resource* Request::getResourceAt(int32_t index) {
    if(index < 0 || index >= mNumResources || this->mResources == nullptr) {
        return nullptr;
    }
    return (*this->mResources)[index];
}

std::vector<CocoNode*>* Request::getCocoNodes() {
    return this->mCocoNodes;
}

CocoNode* Request::getCocoNodeAt(int32_t index) {
    if(index < 0 || index >= mNumCocoNodes) {
        return nullptr;
    }

    return (*this->mCocoNodes)[index];
}

void Request::setNumResources(int32_t numResources) {
    this->mNumResources = numResources;
}

void Request::setNumCocoNodes(int32_t numCocoNodes) {
    this->mNumCocoNodes = numCocoNodes;
}

void Request::setBackgroundProcessing(int8_t isBackgroundProcessingEnabled) {
    this->mBackgroundProcessing = isBackgroundProcessingEnabled;
}

void Request::setResources(std::vector<Resource*>* resources) {
    this->mResources = resources;
}

void Request::setCocoNodes(std::vector<CocoNode*>* cocoNodes) {
    this->mCocoNodes = cocoNodes;
}

// Define Methods to update the Request
void Request::setTimer(Timer* timer) {
    this->mTimer = timer;
}

void Request::updateTimer(int64_t newDuration) {
    this->mTimer->updateTimer(newDuration);
}

// Use cleanpUpRequest for clearing a Request and it's associated components
Request::~Request() {}

void Request::populateUntuneRequest(Request* untuneRequest) {
    untuneRequest->mReqType= REQ_RESOURCE_UNTUNING;
    untuneRequest->mPriority = this->getPriority();
    untuneRequest->mHandle = this->getHandle();
    untuneRequest->mClientPID = this->getClientPID();
    untuneRequest->mClientTID = this->getClientTID();
    untuneRequest->mNumResources = 0;
    untuneRequest->mResources = nullptr;
    untuneRequest->mCocoNodes = nullptr;
    untuneRequest->mTimer = nullptr;
}

void Request::populateRetuneRequest(Request* retuneRequest, int64_t newDuration) {
    retuneRequest->mReqType= REQ_RESOURCE_RETUNING;
    retuneRequest->mHandle = this->getHandle();
    retuneRequest->mPriority = this->getPriority();
    retuneRequest->mClientPID = this->getClientPID();
    retuneRequest->mClientTID = this->getClientTID();
    retuneRequest->mDuration = newDuration;
}

// Request Utils
void Request::cleanUpRequest(Request* request) {
    if(request == nullptr) return;
    // Note: Resources and CocoNodes are expected to be allocated via the MemoryPool.

    for(int32_t i = 0; i < request->getResourcesCount(); i++) {
        Resource* resource = request->getResourceAt(i);
        FreeBlock<Resource>(static_cast<void*>(resource));
    }

    // Note: For CocoNodes strictly use the member mNumCocoNodes vector for iteration, instead of relying
    // on request->getResourcesCount() or request->getCocoNodes()->size(). Since it is possible that for a Request no CocoNodes
    // were allocated, and hence mCocoNodes vector is empty.
    for(int32_t i = 0; i < request->getCocoNodesCount(); i++) {
        CocoNode* cocoNode = request->getCocoNodeAt(i);
        if(cocoNode != nullptr) {
            // Safe Check, should not be needed
            FreeBlock<CocoNode>(static_cast<void*>(cocoNode));
        }
    }

    if(request->mResources != nullptr) {
        FreeBlock<std::vector<Resource*>>
                (static_cast<void*>(request->mResources));
        request->mResources = nullptr;
    }

    if(request->mCocoNodes != nullptr) {
        FreeBlock<std::vector<CocoNode*>>
                (static_cast<void*>(request->mCocoNodes));
        request->mCocoNodes = nullptr;
    }

    FreeBlock<Request>(static_cast<void*>(request));
}
