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

Timer* Request::getTimer() {
    return this->mTimer;
}

void Request::setNumResources(int32_t numResources) {
    this->mNumResources = numResources;
}

void Request::setNumCocoNodes(int32_t numCocoNodes) {
    this->mNumCocoNodes = numCocoNodes;
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

void Request::unsetTimer() {
    this->mTimer = nullptr;
}

// Use cleanpUpRequest for clearing a Request and it's associated components
Request::~Request() {}

void Request::populateUntuneRequest(Request* untuneRequest) {
    untuneRequest->mReqType= REQ_RESOURCE_UNTUNING;
    untuneRequest->mProperties = this->getProperties();
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
    retuneRequest->mProperties = this->getProperties();
    retuneRequest->mClientPID = this->getClientPID();
    retuneRequest->mClientTID = this->getClientTID();
    retuneRequest->mDuration = newDuration;
}

ErrCode Request::serialize(char* buf) {
    try {
        int8_t* ptr8 = (int8_t*)buf;
        ASSIGN_AND_INCR(ptr8, this->getRequestType());

        int64_t* ptr64 = (int64_t*)ptr8;
        ASSIGN_AND_INCR(ptr64, this->getHandle());

        ASSIGN_AND_INCR(ptr64, this->getDuration());

        int32_t* ptr = (int32_t*)ptr64;
        ASSIGN_AND_INCR(ptr, this->getResourcesCount());
        ASSIGN_AND_INCR(ptr, this->getProperties());
        ASSIGN_AND_INCR(ptr, this->getClientPID());
        ASSIGN_AND_INCR(ptr, this->getClientTID());

        ptr8 = (int8_t*)ptr;
        ASSIGN_AND_INCR(ptr8, this->isBackgroundProcessingEnabled());

        ptr = (int32_t*)ptr8;
        for(int32_t i = 0; i < this->getResourcesCount(); i++) {
            Resource* resource = this->getResourceAt(i);
            if(resource == nullptr) {
                return RC_INVALID_VALUE;
            }

            ASSIGN_AND_INCR(ptr, resource->mOpCode);
            ASSIGN_AND_INCR(ptr, resource->mOpInfo);
            ASSIGN_AND_INCR(ptr, resource->mOptionalInfo);
            ASSIGN_AND_INCR(ptr, resource->mNumValues);

            if(resource->mNumValues == 1) {
                ASSIGN_AND_INCR(ptr, resource->mConfigValue.singleValue);
            } else {
                for(int32_t j = 0; j < resource->mNumValues; j++) {
                    ASSIGN_AND_INCR(ptr, (*resource->mConfigValue.valueArray)[j]);
                }
            }
        }
    } catch(const std::invalid_argument& e) {
        return RC_REQUEST_PARSING_FAILED;

    } catch(const std::exception& e) {
        return RC_INVALID_VALUE;
    }

    return RC_SUCCESS;
}

ErrCode Request::deserialize(char* buf) {
     try {
        int8_t* ptr8 = (int8_t*)buf;
        this->mReqType = DEREF_AND_INCR(ptr8, int8_t);

        int64_t* ptr64 = (int64_t*)ptr8;
        this->mHandle = DEREF_AND_INCR(ptr64, int64_t);
        this->mDuration = DEREF_AND_INCR(ptr64, int64_t);

        int32_t* ptr = (int32_t*)ptr64;
        this->mNumResources = DEREF_AND_INCR(ptr, int32_t);
        this->mProperties = DEREF_AND_INCR(ptr, int32_t);
        this->mClientPID = DEREF_AND_INCR(ptr, int32_t);
        this->mClientTID = DEREF_AND_INCR(ptr, int32_t);

        ptr8 = (int8_t*)ptr;
        this->mBackgroundProcessing = DEREF_AND_INCR(ptr8, int8_t);

        if(this->mReqType == REQ_RESOURCE_TUNING) {
            this->mResources = new (GetBlock<std::vector<Resource*>>())
                                    std::vector<Resource*>;

            this->mResources->resize(this->getResourcesCount());

            ptr = (int32_t*)ptr8;
            for(int32_t i = 0; i < this->getResourcesCount(); i++) {
                Resource* resource = (Resource*) (GetBlock<Resource>());

                resource->mOpCode = DEREF_AND_INCR(ptr, int32_t);
                resource->mOpInfo = DEREF_AND_INCR(ptr, int32_t);
                resource->mOptionalInfo = DEREF_AND_INCR(ptr, int32_t);
                resource->mNumValues = DEREF_AND_INCR(ptr, int32_t);

                if(resource->mNumValues == 1) {
                    resource->mConfigValue.singleValue = DEREF_AND_INCR(ptr, int32_t);
                } else {
                    for(int32_t j = 0; j < resource->mNumValues; j++) {
                        resource->mConfigValue.valueArray->push_back(DEREF_AND_INCR(ptr, int32_t));
                    }
                }

                (*this->mResources)[i] = resource;
            }
        }

    } catch(const std::invalid_argument& e) {
        TYPELOGV(REQUEST_PARSING_FAILURE, "Error", e);
        return RC_REQUEST_PARSING_FAILED;

    } catch(const std::bad_alloc& e) {
        TYPELOGV(REQUEST_MEMORY_ALLOCATION_FAILURE, "Error", e);
        return RC_MEMORY_POOL_BLOCK_RETRIEVAL_FAILURE;

    } catch(const std::exception& e) {
        LOGE("URM_SYSTUNE_SERVER",
             "Request Deserialization Failed with error: " + std::string(e.what()));
        return RC_REQUEST_DESERIALIZATION_FAILURE;
    }

    return RC_SUCCESS;
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

    // Free timer block
    if(request->mTimer != nullptr) {
        FreeBlock<Timer>(static_cast<void*>(request->mTimer));
        request->mTimer = nullptr;
    }

    FreeBlock<Request>(static_cast<void*>(request));
}
