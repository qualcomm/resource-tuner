// Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
// SPDX-License-Identifier: BSD-3-Clause-Clear

#include "BinderServer.h"

ndk::ScopedAStatus Restune::tuneResources(int64_t duration,
                                          int32_t prop,
                                          int32_t numRes,
                                          SysResource* resourceList) {

    if(!ComponentRegistry::isModuleEnabled(MOD_CORE)) {
        TYPELOGV(NOTIFY_MODULE_NOT_ENABLED, "Core");
        return;
    }

    int64_t handle = AuxRoutines::generateUniqueHandle();
    if(handle < 0) {
        // Handle Generation Failure
        LOGE("RESTUNE_REQUEST_RECEIVER", "Failed to Generate Request handle");
        return;
    }

    // // Enqueue the Request to the Thread Pool for async processing.
    // if(this->mRequestsThreadPool != nullptr) {
    //     if(!this->mRequestsThreadPool->
    //         enqueueTask(ComponentRegistry::getEventCallback(MOD_CORE_ON_MSG_RECV), msgForwardInfo)) {
    //         TYPELOGD(INCOMING_REQUEST_THREAD_POOL_ENQUEUE_FAILURE);
    //     }
    // } else {
    //     TYPELOGD(INCOMING_REQUEST_THREAD_POOL_INIT_NOT_DONE);
    // }

    return ndk::ScopedAStatus::ok();
}

ndk::ScopedAStatus Restune::retuneResources(int64_t handle, int64_t duration) {
    if(!ComponentRegistry::isModuleEnabled(MOD_CORE)) {
        TYPELOGV(NOTIFY_MODULE_NOT_ENABLED, "Core");
        return;
    }

    // // Enqueue the Request to the Thread Pool for async processing.
    // if(this->mRequestsThreadPool != nullptr) {
    //     if(!this->mRequestsThreadPool->
    //         enqueueTask(ComponentRegistry::getEventCallback(MOD_CORE_ON_MSG_RECV), msgForwardInfo)) {
    //         TYPELOGD(INCOMING_REQUEST_THREAD_POOL_ENQUEUE_FAILURE);
    //     }
    // } else {
    //     TYPELOGD(INCOMING_REQUEST_THREAD_POOL_INIT_NOT_DONE);
    // }

    return ndk::ScopedAStatus::ok();
}

ndk::ScopedAStatus Restune::untuneResources(int64_t handle) {
    if(!ComponentRegistry::isModuleEnabled(MOD_CORE)) {
        TYPELOGV(NOTIFY_MODULE_NOT_ENABLED, "Core");
        return;
    }

    // // Enqueue the Request to the Thread Pool for async processing.
    // if(this->mRequestsThreadPool != nullptr) {
    //     if(!this->mRequestsThreadPool->
    //         enqueueTask(ComponentRegistry::getEventCallback(MOD_CORE_ON_MSG_RECV), msgForwardInfo)) {
    //         TYPELOGD(INCOMING_REQUEST_THREAD_POOL_ENQUEUE_FAILURE);
    //     }
    // } else {
    //     TYPELOGD(INCOMING_REQUEST_THREAD_POOL_INIT_NOT_DONE);
    // }

    return ndk::ScopedAStatus::ok();
}

ndk::ScopedAStatus Restune::getProp(const char* prop,
                                    char* buffer,
                                    size_t bufferSize,
                                    const char* defValue) {
    return ndk::ScopedAStatus::ok();
}

ndk::ScopedAStatus Restune::tuneSignal(uint32_t signalCode,
                                       int64_t duration,
                                       int32_t properties,
                                       const char* appName,
                                       const char* scenario,
                                       int32_t numArgs,
                                       uint32_t* list) {

    if(!ComponentRegistry::isModuleEnabled(MOD_SIGNAL)) {
        TYPELOGV(NOTIFY_MODULE_NOT_ENABLED, "Signals");
        return;
    }

    return ndk::ScopedAStatus::ok();
}

ndk::ScopedAStatus Restune::relaySignal(uint32_t signalCode,
                                        int64_t duration,
                                        int32_t properties,
                                        const char* appName,
                                        const char* scenario,
                                        int32_t numArgs,
                                        uint32_t* list) {

    if(!ComponentRegistry::isModuleEnabled(MOD_SIGNAL)) {
        TYPELOGV(NOTIFY_MODULE_NOT_ENABLED, "Signals");
        return;
    }

    return ndk::ScopedAStatus::ok();
}

ndk::ScopedAStatus Restune::untuneSignal(int64_t handle) {
    if(!ComponentRegistry::isModuleEnabled(MOD_SIGNAL)) {
        TYPELOGV(NOTIFY_MODULE_NOT_ENABLED, "Signals");
        return;
    }

    return ndk::ScopedAStatus::ok();
}

void listenerThreadStartRoutine() {
   
}
