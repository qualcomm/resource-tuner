// Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
// SPDX-License-Identifier: BSD-3-Clause-Clear

#include "RequestReceiver.h"
#include "SysConfigInternal.h"

std::shared_ptr<RequestReceiver> RequestReceiver::mRequestReceiverInstance = nullptr;
ThreadPool* RequestReceiver::mRequestsThreadPool = nullptr;
RequestReceiver::RequestReceiver() {}

void RequestReceiver::forwardMessage(int32_t clientSocket, MsgForwardInfo* msgForwardInfo) {
    int8_t requestType = *(int8_t*) msgForwardInfo->buffer;

    switch(requestType) {
        // Resource Provisioning Requests
        case REQ_RESOURCE_TUNING: {
            if(!ComponentRegistry::isModuleEnabled(MOD_CORE)) {
                TYPELOGV(NOTIFY_MODULE_NOT_ENABLED, "Core");
                return;
            }
            msgForwardInfo->handle = ResourceTunerSettings::generateUniqueHandle();
            if(msgForwardInfo->handle < 0) {
                // Handle Generation Failure
                return;
            }
        }
        case REQ_RESOURCE_RETUNING:
        case REQ_RESOURCE_UNTUNING: {
            if(!ComponentRegistry::isModuleEnabled(MOD_CORE)) {
                TYPELOGV(NOTIFY_MODULE_NOT_ENABLED, "Core");
                return;
            }
            // Enqueue the Request to the Thread Pool for async processing.
            if(this->mRequestsThreadPool != nullptr) {
                if(!this->mRequestsThreadPool->
                    enqueueTask(ComponentRegistry::getModuleMessageHandlerCallback(MOD_CORE), msgForwardInfo)) {
                    LOGE("RTN_REQUEST_RECEIVER",
                         "Failed to enqueue the Request to the Thread Pool");
                }
            } else {
                LOGE("RTN_REQUEST_RECEIVER",
                     "Thread pool not initialized, Dropping the Request");
            }

            // Only in Case of Tune Requests, Write back the handle to the client.
            if(requestType == REQ_RESOURCE_TUNING) {
                if(write(clientSocket, (const void*)&msgForwardInfo->handle, sizeof(int64_t)) == -1) {
                    TYPELOGV(ERRNO_LOG, "write", strerror(errno));
                }
            }
            break;
        }
        // SysConfig Requests
        case REQ_CLIENT_GET_REQUESTS:
        case REQ_SYSCONFIG_GET_PROP:
        case REQ_SYSCONFIG_SET_PROP: {
            // Deserialize to SysConfig Request
            SysConfig* sysConfig = nullptr;
            try {
                sysConfig = new (GetBlock<SysConfig>()) SysConfig();

            } catch(const std::bad_alloc& e) {
                TYPELOGV(REQUEST_MEMORY_ALLOCATION_FAILURE, e.what());
                return;
            }

            if(RC_IS_NOTOK(sysConfig->deserialize(msgForwardInfo->buffer))) {
                FreeBlock<SysConfig>(sysConfig);
                return;
            }

            std::string result;
            int8_t status = submitSysConfigRequest(result, sysConfig);

            if(requestType == REQ_SYSCONFIG_GET_PROP) {
                if(write(clientSocket, (const void*)result.c_str(), sizeof(sysConfig->getBufferSize())) == -1) {
                    TYPELOGV(ERRNO_LOG, "write", strerror(errno));
                }
            }

            FreeBlock<SysConfig>(sysConfig);
        }
        // SysSignal Requests
        case SIGNAL_ACQ: {
            if(!ComponentRegistry::isModuleEnabled(MOD_SYSSIGNAL)) {
                TYPELOGV(NOTIFY_MODULE_NOT_ENABLED, "Signals");
                return;
            }
            msgForwardInfo->handle = ResourceTunerSettings::generateUniqueHandle();
            if(msgForwardInfo->handle < 0) {
                // Handle Generation Failure
                return;
            }
        }
        case SIGNAL_FREE:
        case SIGNAL_RELAY: {
            if(!ComponentRegistry::isModuleEnabled(MOD_SYSSIGNAL)) {
                TYPELOGV(NOTIFY_MODULE_NOT_ENABLED, "Signals");
                return;
            }
            // Enqueue the Request to the Thread Pool for async processing.
            if(this->mRequestsThreadPool != nullptr) {
                if(!this->mRequestsThreadPool->
                    enqueueTask(ComponentRegistry::getModuleMessageHandlerCallback(MOD_SYSSIGNAL), msgForwardInfo)) {
                    LOGE("RTN_REQUEST_RECEIVER",
                         "Failed to enqueue the Request to the Thread Pool");
                }
            } else {
                LOGE("RTN_REQUEST_RECEIVER",
                     "Thread pool not initialized, Dropping the Request");
            }

            // Only in Case of Tune Signals, Write back the handle to the client.
            if(requestType == SIGNAL_ACQ) {
                if(write(clientSocket, (const void*)&msgForwardInfo->handle, sizeof(int64_t)) == -1) {
                    TYPELOGV(ERRNO_LOG, "write", strerror(errno));
                }
            }
            break;
        }
        default:
            LOGE("RTN_SOCKET_SERVER", "Invalid Request Type");
            break;
    }
}

int8_t CheckServerOnlineStatus() {
    return ResourceTunerSettings::isServerOnline();
}

void OnResourceTunerMessageReceiverCallback(int32_t clientSocket, MsgForwardInfo* msgForwardInfo) {
    std::shared_ptr<RequestReceiver> requestReceiver = RequestReceiver::getInstance();
    requestReceiver->forwardMessage(clientSocket, msgForwardInfo);
}

void listenerThreadStartRoutine() {
    ResourceTunerSocketServer* connection;
    try {
        connection = new ResourceTunerSocketServer(12000,
                                             CheckServerOnlineStatus,
                                             OnResourceTunerMessageReceiverCallback);
    } catch(const std::bad_alloc& e) {
        LOGE("RTN_REQUEST_RECEIVER",
             "Failed to allocate memory for Resource Tuner Socket Server-Endpoint, Resource Tuner\
              Server startup failed: " + std::string(e.what()));

        return;
    } catch(const std::exception& e) {
        LOGE("RTN_REQUEST_RECEIVER",
             "Failed to start the Resource Tuner Listener, error: " + std::string(e.what()));

        return;
    }

    if(RC_IS_NOTOK(connection->ListenForClientRequests())) {
        LOGE("RTN_REQUEST_RECEIVER", "Server Socket Endpoint crashed");
    }

    if(connection != nullptr) {
        delete(connection);
    }
    return;
}
