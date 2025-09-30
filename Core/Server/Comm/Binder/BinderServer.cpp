// Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
// SPDX-License-Identifier: BSD-3-Clause-Clear

#include "BinderServer.h"

Restune::tuneResources(int64_t duration, int32_t prop, int32_t numRes, SysResource* resourceList) {
    if(!ComponentRegistry::isModuleEnabled(MOD_CORE)) {
        TYPELOGV(NOTIFY_MODULE_NOT_ENABLED, "Core");
        return;
    }
    msgForwardInfo->handle = AuxRoutines::generateUniqueHandle();
    if(msgForwardInfo->handle < 0) {
        // Handle Generation Failure
        LOGE("RESTUNE_REQUEST_RECEIVER",
                "Failed to Generate Request handle");
        return;
    }
    LOGD("RESTUNE_REQUEST_RECEIVER",
         "Incoming Request, handle generated = " + std::to_string(msgForwardInfo->handle));

    if(this->mRequestsThreadPool != nullptr) {
        if(!this->mRequestsThreadPool->
            enqueueTask(ComponentRegistry::getEventCallback(MOD_CORE_ON_MSG_RECV), msgForwardInfo)) {
            LOGE("RESTUNE_REQUEST_RECEIVER",
                    "Failed to enqueue the Request to the Thread Pool");
        }
    } else {
        LOGE("RESTUNE_REQUEST_RECEIVER",
                "Thread pool not initialized, Dropping the Request");
    }

    if(write(clientSocket, (const void*)&msgForwardInfo->handle, sizeof(int64_t)) == -1) {
        TYPELOGV(ERRNO_LOG, "write", strerror(errno));
    }
}
