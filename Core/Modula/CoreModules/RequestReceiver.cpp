// Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
// SPDX-License-Identifier: BSD-3-Clause-Clear

#include "RequestReceiver.h"

std::mutex RequestReceiver::instanceProtectionLock {};
std::shared_ptr<RequestReceiver> RequestReceiver::mRequestReceiverInstance = nullptr;

RequestReceiver::RequestReceiver() {
    this->mRequestsThreadPool = nullptr;
    this->mTimerThreadPool = nullptr;
}

ThreadPool* RequestReceiver::getRequestThreadPool() {
    return this->mRequestsThreadPool;
}

ThreadPool* RequestReceiver::getTimerThreadPool() {
    return this->mTimerThreadPool;
}

ErrCode RequestReceiver::preAllocateWorkers() {
    int32_t desiredThreadCapacity = ResourceTunerSettings::desiredThreadCount;
    int32_t maxScalingCapacity = ResourceTunerSettings::maxScalingCapacity;

    try {
        this->mRequestsThreadPool = new ThreadPool(desiredThreadCapacity,
                                                   maxScalingCapacity);

        // Allocate 2 extra threads for Pulse Monitor and Garbage Collector
        this->mTimerThreadPool = new ThreadPool(desiredThreadCapacity + 2,
                                                maxScalingCapacity);

    } catch(const std::bad_alloc& e) {
        TYPELOGV(THREAD_POOL_CREATION_FAILURE, e.what());
        return RC_MODULE_INIT_FAILURE;
    }

    return RC_SUCCESS;
}

RequestReceiver::~RequestReceiver() {
    if(this->mRequestsThreadPool != nullptr) {
        delete this->mRequestsThreadPool;
    }

    if(this->mTimerThreadPool != nullptr) {
        delete this->mTimerThreadPool;
    }
}
