// Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
// SPDX-License-Identifier: BSD-3-Clause-Clear

#include "Request.h"

Request::Request() {
    this->mTimer = nullptr;
    this->mResourceList = new DLManager(REQUEST_DL_NR);
}

int32_t Request::getResourcesCount() {
    return this->mResourceList->getLen();
}

Timer* Request::getTimer() {
    return this->mTimer;
}

DLManager* Request::getResDlMgr() {
    return this->mResourceList;
}

void Request::addResource(CoreIterable* resIterable) {
    this->mResourceList->insert(resIterable);
}

// Define Methods to update the Request
void Request::setTimer(Timer* timer) {
    this->mTimer = timer;
}

void Request::unsetTimer() {
    this->mTimer = nullptr;
}

void Request::clearResources() {
    DL_ITERATE(this->mResourceList) {
        if(iter != nullptr && iter->mData != nullptr) {
            // Delete Resource struct
            FreeBlock<Resource>(iter->mData);
        }

        if(iter != nullptr) {
            // Delete CoreIterable itself
            FreeBlock<CoreIterable>(iter);
        }
    }
    this->mResourceList->destroy();
}

// Use cleanpUpRequest for clearing a Request and it's associated components
Request::~Request() {}

void Request::populateUntuneRequest(Request* untuneRequest) {
    untuneRequest->mReqType= REQ_RESOURCE_UNTUNING;
    untuneRequest->mProperties = this->getProperties();
    untuneRequest->mHandle = this->getHandle();
    untuneRequest->mClientPID = this->getClientPID();
    untuneRequest->mClientTID = this->getClientTID();
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

        DL_ITERATE(this->getResDlMgr()) {
            if(iter == nullptr || iter->mData == nullptr) {
                return RC_INVALID_VALUE;
            }

            Resource* resource = (Resource*) iter->mData;
            if(resource == nullptr) {
                return RC_INVALID_VALUE;
            }

            ASSIGN_AND_INCR(ptr, resource->getResCode());
            ASSIGN_AND_INCR(ptr, resource->getResInfo());
            ASSIGN_AND_INCR(ptr, resource->getOptionalInfo());
            ASSIGN_AND_INCR(ptr, resource->getValuesCount());

            if(resource->getValuesCount() == 1) {
                ASSIGN_AND_INCR(ptr, resource->mResValue.value);
            } else {
                for(int32_t j = 0; j < resource->getValuesCount(); j++) {
                    ASSIGN_AND_INCR(ptr, (*resource->mResValue.values)[j]);
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
        int32_t numResources = 0;
        int8_t* ptr8 = (int8_t*)buf;
        this->mReqType = DEREF_AND_INCR(ptr8, int8_t);

        int64_t* ptr64 = (int64_t*)ptr8;
        this->mHandle = DEREF_AND_INCR(ptr64, int64_t);
        this->mDuration = DEREF_AND_INCR(ptr64, int64_t);

        int32_t* ptr = (int32_t*)ptr64;
        numResources = DEREF_AND_INCR(ptr, int32_t);
        this->mProperties = DEREF_AND_INCR(ptr, int32_t);
        this->mClientPID = DEREF_AND_INCR(ptr, int32_t);
        this->mClientTID = DEREF_AND_INCR(ptr, int32_t);

        if(this->mReqType == REQ_RESOURCE_TUNING) {
            for(int32_t i = 0; i < numResources; i++) {
                CoreIterable* resIterable = MPLACED(CoreIterable);
                Resource* resource = MPLACED(Resource);

                resource->setResCode(DEREF_AND_INCR(ptr, int32_t));
                resource->setResInfo(DEREF_AND_INCR(ptr, int32_t));
                resource->setOptionalInfo(DEREF_AND_INCR(ptr, int32_t));
                resource->setNumValues(DEREF_AND_INCR(ptr, int32_t));

                if(resource->getValuesCount() == 1) {
                    resource->mResValue.value = DEREF_AND_INCR(ptr, int32_t);
                } else {
                    for(int32_t j = 0; j < resource->getValuesCount(); j++) {
                        if(resource->mResValue.values == nullptr) {
                            resource->mResValue.values = MPLACED(std::vector<int32_t>);
                        }
                        resource->mResValue.values->push_back(DEREF_AND_INCR(ptr, int32_t));
                    }
                }

                resIterable->mData = resource;
                this->addResource(resIterable);
            }
        }

    } catch(const std::invalid_argument& e) {
        TYPELOGV(REQUEST_PARSING_FAILURE, e.what());
        return RC_REQUEST_PARSING_FAILED;

    } catch(const std::bad_alloc& e) {
        TYPELOGV(REQUEST_MEMORY_ALLOCATION_FAILURE, e.what());
        return RC_MEMORY_POOL_BLOCK_RETRIEVAL_FAILURE;

    } catch(const std::exception& e) {
        LOGE("RESTUNE_SERVER",
             "Request Deserialization Failed with error: " + std::string(e.what()));
        return RC_REQUEST_DESERIALIZATION_FAILURE;
    }

    return RC_SUCCESS;
}

// Request Utils
void Request::cleanUpRequest(Request* request) {
    if(request == nullptr) return;
    request->clearResources();

    // Free timer block
    if(request->mTimer != nullptr) {
        FreeBlock<Timer>(static_cast<void*>(request->mTimer));
        request->mTimer = nullptr;
    }

    // Free the Request struct itself
    FreeBlock<Request>(static_cast<void*>(request));
}
