// Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
// SPDX-License-Identifier: BSD-3-Clause-Clear

#include "Restune.h"

namespace aidl {
namespace vendor {
namespace qti {
namespace hardware {
namespace restune {

ndk::ScopedAStatus Restune::tuneResources(int64_t duration,
                                          int32_t prop,
                                          const std::vector<SysResource>& resourceList,
                                          int64_t* _aidl_return) {

    if(!ComponentRegistry::isModuleEnabled(MOD_CORE)) {
        TYPELOGV(NOTIFY_MODULE_NOT_ENABLED, "Core");
        return ndk::ScopedAStatus::ok();
    }

    int64_t handle = AuxRoutines::generateUniqueHandle();
    if(handle < 0) {
        // Handle Generation Failure
        LOGE("RESTUNE_REQUEST_RECEIVER", "Failed to Generate Request handle");
        return ndk::ScopedAStatus::ok();
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

ndk::ScopedAStatus Restune::retuneResources(int64_t handle, int64_t duration, int8_t* _aidl_return) {
    if(!ComponentRegistry::isModuleEnabled(MOD_CORE)) {
        TYPELOGV(NOTIFY_MODULE_NOT_ENABLED, "Core");
        return ndk::ScopedAStatus::ok();
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

ndk::ScopedAStatus Restune::untuneResources(int64_t handle, int8_t* _aidl_return) {
    if(!ComponentRegistry::isModuleEnabled(MOD_CORE)) {
        TYPELOGV(NOTIFY_MODULE_NOT_ENABLED, "Core");
        return ndk::ScopedAStatus::ok();
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

ndk::ScopedAStatus Restune::getProp(const std::string& propName,
                                    const std::string& defaultVal,
                                    std::string *_aidl_return) {
    return ndk::ScopedAStatus::ok();
}

ndk::ScopedAStatus Restune::tuneSignal(int64_t signalCode,
                                       int64_t duration,
                                       int32_t properties,
                                       const std::string& appName,
                                       const std::string& scenario,
                                       const std::vector<int64_t>& list,
                                       int64_t* _aidl_return) {

    if(!ComponentRegistry::isModuleEnabled(MOD_SIGNAL)) {
        TYPELOGV(NOTIFY_MODULE_NOT_ENABLED, "Signals");
        return ndk::ScopedAStatus::ok();
    }

    return ndk::ScopedAStatus::ok();
}

ndk::ScopedAStatus Restune::relaySignal(int64_t signalCode,
                                        int64_t duration,
                                        int32_t properties,
                                        const std::string& appName,
                                        const std::string& scenario,
                                        const std::vector<int64_t>& list,
                                        int8_t* _aidl_return) {

    if(!ComponentRegistry::isModuleEnabled(MOD_SIGNAL)) {
        TYPELOGV(NOTIFY_MODULE_NOT_ENABLED, "Signals");
        return ndk::ScopedAStatus::ok();
    }

    return ndk::ScopedAStatus::ok();
}

ndk::ScopedAStatus Restune::untuneSignal(int64_t handle, int8_t* _aidl_return) {
    if(!ComponentRegistry::isModuleEnabled(MOD_SIGNAL)) {
        TYPELOGV(NOTIFY_MODULE_NOT_ENABLED, "Signals");
        return ndk::ScopedAStatus::ok();
    }

    return ndk::ScopedAStatus::ok();
}

}
}
}
}
}

void listenerThreadStartRoutine() {
   
}
