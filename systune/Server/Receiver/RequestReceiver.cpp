// Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
// SPDX-License-Identifier: BSD-3-Clause-Clear

#include "RequestReceiver.h"

std::shared_ptr<RequestReceiver> RequestReceiver::mRequestReceiverInstance = nullptr;
ThreadPool* RequestReceiver::mRequestsThreadPool = nullptr;
RequestReceiver::RequestReceiver() {}

void RequestReceiver::forwardMessage(int32_t clientSocket, MsgForwardInfo* msgForwardInfo) {
    int8_t requestType = *(int8_t*) msgForwardInfo->buffer;

    switch(requestType) {
        // Resource Provisioning Requests
        case REQ_RESOURCE_TUNING: {
            msgForwardInfo->handle = SystuneSettings::generateUniqueHandle();
            if(msgForwardInfo->handle < 0) {
                // Handle Generation Failure
                return;
            }
        }
        case REQ_RESOURCE_RETUNING:
        case REQ_RESOURCE_UNTUNING: {
            // Enqueue the Request to the Thread Pool for async processing.
            if(this->mRequestsThreadPool != nullptr) {
                if(!this->mRequestsThreadPool->
                    enqueueTask(ComponentRegistry::getModuleMessageHandlerCallback(MOD_PROVISIONER), msgForwardInfo)) {
                    LOGE("URM_REQUEST_RECEIVER",
                         "Failed to enqueue the Request to the Thread Pool");
                }
            } else {
                LOGE("URM_REQUEST_RECEIVER",
                     "Thread pool not initialized, Dropping the Request");
            }

            // Only in Case of Tune Requests, Write back the handle to the client.
            if(requestType == REQ_RESOURCE_TUNING) {
                if(write(clientSocket, (const void*)msgForwardInfo->handle, sizeof(int64_t)) == -1) {
                    TYPELOGV(ERRNO_LOG, "write", strerror(errno));
                }
            }
            break;
        }
        // SysConfig Requests
        case REQ_CLIENT_GET_REQUESTS:
        case REQ_SYSCONFIG_GET_PROP:
        case REQ_SYSCONFIG_SET_PROP: {
            break;
        }
        // SysSignal Requests
        case SIGNAL_ACQ: {
            msgForwardInfo->handle = SystuneSettings::generateUniqueHandle();
            if(msgForwardInfo->handle < 0) {
                // Handle Generation Failure
                return;
            }
        }
        case SIGNAL_FREE:
        case SIGNAL_RELAY: {
            // Enqueue the Request to the Thread Pool for async processing.
            if(this->mRequestsThreadPool != nullptr) {
                if(!this->mRequestsThreadPool->
                    enqueueTask(ComponentRegistry::getModuleMessageHandlerCallback(MOD_PROVISIONER), msgForwardInfo)) {
                    LOGE("URM_REQUEST_RECEIVER",
                         "Failed to enqueue the Request to the Thread Pool");
                }
            } else {
                LOGE("URM_REQUEST_RECEIVER",
                     "Thread pool not initialized, Dropping the Request");
            }

            // Only in Case of Tune Signals, Write back the handle to the client.
            if(requestType == SIGNAL_ACQ) {
                if(write(clientSocket, (const void*)msgForwardInfo->handle, sizeof(int64_t)) == -1) {
                    TYPELOGV(ERRNO_LOG, "write", strerror(errno));
                }
            }
            break;
        }
        default:
            LOGE("URM_SOCKET_SERVER", "Invalid Request Type");
            break;
    }
}

int8_t CheckServerOnlineStatus() {
    return SystuneSettings::isServerOnline();
}

int64_t OnSysTuneMessageAsyncCallback(int32_t clientSocket, MsgForwardInfo* msgForwardInfo) {
    std::shared_ptr<RequestReceiver> requestReceiver = RequestReceiver::getInstance();
    requestReceiver->forwardMessage(clientSocket, msgForwardInfo);

    return 0;
}

int8_t OnSystuneMessageSyncCallback(int8_t a, void* b, char* c, uint64_t d) {
    return 0;
}

void listenerThreadStartRoutine() {
    SystuneSocketServer* connection;
    try {
        connection = new SystuneSocketServer(12000,
                                             CheckServerOnlineStatus,
                                             OnSysTuneMessageAsyncCallback,
                                             OnSystuneMessageSyncCallback);
    } catch(const std::bad_alloc& e) {
        LOGE("URM_REQUEST_RECEIVER",
             "Failed to allocate memory for Systune Socket Server-Endpoint, Systune\
             Server startup failed: " + std::string(e.what()));

        return;
    } catch(const std::exception& e) {
        LOGE("URM_REQUEST_RECEIVER",
             "Failed to start the Systune Listener, error: " + std::string(e.what()));

        return;
    }

    if(RC_IS_NOTOK(connection->ListenForClientRequests())) {
        LOGE("URM_REQUEST_RECEIVER", "Server Socket Endpoint crashed");
    }

    if(connection != nullptr) {
        delete(connection);
    }
    return;
}
