// Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
// SPDX-License-Identifier: BSD-3-Clause-Clear

#include "RequestReceiver.h"

std::shared_ptr<RequestReceiver> RequestReceiver::mRequestReceiverInstance = nullptr;
ThreadPool* RequestReceiver::mRequestsThreadPool = nullptr;

RequestReceiver::RequestReceiver() {}

void RequestReceiver::forwardMessage(int32_t clientSocket, MsgForwardInfo* msgForwardInfo) {
    int8_t moduleID = *(int8_t*) msgForwardInfo->buffer;
    int8_t requestType = *(int8_t*) ((unsigned char*) msgForwardInfo->buffer + sizeof(int8_t));

    if(!ComponentRegistry::isModuleEnabled(static_cast<ModuleID>(moduleID))) {
        return;
    }

    msgForwardInfo->mHandle = AuxRoutines::generateUniqueHandle();
    if(msgForwardInfo->mHandle < 0) {
        // Handle Generation Failure
        LOGE("RESTUNE_REQUEST_RECEIVER", "Failed to Generate Request handle");
        return;
    }

    ModuleInfo modInfo = ComponentRegistry::getModuleInfo(static_cast<ModuleID>(moduleID));
    if(modInfo.mOnEvent == nullptr) {
        // Module not enabled
        return;
    }

    // Enqueue the Request to the Thread Pool for async processing.
    if(this->mRequestsThreadPool != nullptr) {
        if(!this->mRequestsThreadPool->
            enqueueTask(modInfo.mOnEvent, msgForwardInfo)) {
            LOGE("RESTUNE_REQUEST_RECEIVER",
                    "Failed to enqueue the Request to the Thread Pool");
        }
    } else {
        LOGE("RESTUNE_REQUEST_RECEIVER",
                "Thread pool not initialized, Dropping the Request");
    }

    // Only in Case of Tune Requests, Write back the handle to the client.
    if(requestType == REQ_RESOURCE_TUNING || requestType == REQ_SIGNAL_TUNING) {
        if(write(clientSocket, (const void*)&msgForwardInfo->mHandle, sizeof(int64_t)) == -1) {
            TYPELOGV(ERRNO_LOG, "write", strerror(errno));
        }
    }
}

int8_t checkServerOnlineStatus() {
    return ResourceTunerSettings::isServerOnline();
}

void onMsgRecvCallback(int32_t clientSocket, MsgForwardInfo* msgForwardInfo) {
    std::shared_ptr<RequestReceiver> requestReceiver = RequestReceiver::getInstance();
    requestReceiver->forwardMessage(clientSocket, msgForwardInfo);
}

void listenerThreadStartRoutine() {
    SocketServer* connection;
    try {
        connection = new SocketServer(checkServerOnlineStatus, onMsgRecvCallback);
    } catch(const std::bad_alloc& e) {
        LOGE("RESTUNE_REQUEST_RECEIVER",
             "Failed to allocate memory for Resource Tuner Socket Server-Endpoint, Resource Tuner \
              Server startup failed: " + std::string(e.what()));

        return;

    } catch(const std::exception& e) {
        LOGE("RESTUNE_REQUEST_RECEIVER",
             "Failed to start the Resource Tuner Listener, error: " + std::string(e.what()));

        return;
    }

    if(RC_IS_NOTOK(connection->ListenForClientRequests())) {
        LOGE("RESTUNE_REQUEST_RECEIVER", "Server Socket Endpoint crashed");
    }

    if(connection != nullptr) {
        delete(connection);
    }
    return;
}
