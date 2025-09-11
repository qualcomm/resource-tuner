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
            LOGE("RESTUNE_REQUEST_RECEIVER",
                 "Incoming Request, handle generated = " + std::to_string(msgForwardInfo->handle));
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
                    enqueueTask(ComponentRegistry::getEventCallback(MOD_CORE_ON_MSG_RECV), msgForwardInfo)) {
                    LOGE("RESTUNE_REQUEST_RECEIVER",
                         "Failed to enqueue the Request to the Thread Pool");
                }
            } else {
                LOGE("RESTUNE_REQUEST_RECEIVER",
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
        case REQ_SYSCONFIG_GET_PROP: {
            // Decode Prop Fetch Request
            PropConfig propConfig;
            memset(&propConfig, 0, sizeof(propConfig));

            int8_t* ptr8 = (int8_t*)msgForwardInfo->buffer;
            (void) DEREF_AND_INCR(ptr8, int8_t);

            char* charIterator = (char*)ptr8;
            propConfig.mPropName = charIterator;

            while(*charIterator != '\0') {
                charIterator++;
            }
            charIterator++;

            uint64_t* ptr64 = (uint64_t*)charIterator;
            propConfig.mBufferSize = DEREF_AND_INCR(ptr64, uint64_t);

            ComponentRegistry::getEventCallback(PROP_ON_MSG_RECV)(&propConfig);
            std::string result = propConfig.mResult;

            size_t maxSafeSize = result.size() + 1;
            size_t bytesToWrite = std::min(propConfig.mBufferSize, maxSafeSize);

            if(write(clientSocket, (const void*)result.c_str(), bytesToWrite) == -1) {
                TYPELOGV(ERRNO_LOG, "write", strerror(errno));
            }
            break;
        }
        // Signal Requests
        case REQ_SIGNAL_TUNING: {
            if(!ComponentRegistry::isModuleEnabled(MOD_SIGNAL)) {
                TYPELOGV(NOTIFY_MODULE_NOT_ENABLED, "Signals");
                return;
            }
            msgForwardInfo->handle = AuxRoutines::generateUniqueHandle();
            if(msgForwardInfo->handle < 0) {
                // Handle Generation Failure
                LOGE("RESTUNE_REQUEST_RECEIVER",
                     "Failed to Generate Request handle");
                return;
            }
            LOGE("RESTUNE_REQUEST_RECEIVER",
                 "Incoming Request, handle generated = " + std::to_string(msgForwardInfo->handle));
        }
        case REQ_SIGNAL_UNTUNING:
        case REQ_SIGNAL_RELAY: {
            if(!ComponentRegistry::isModuleEnabled(MOD_SIGNAL)) {
                TYPELOGV(NOTIFY_MODULE_NOT_ENABLED, "Signals");
                return;
            }
            // Enqueue the Request to the Thread Pool for async processing.
            if(this->mRequestsThreadPool != nullptr) {
                if(!this->mRequestsThreadPool->
                    enqueueTask(ComponentRegistry::getEventCallback(MOD_SIGNAL_ON_MSG_RECV), msgForwardInfo)) {
                    LOGE("RESTUNE_REQUEST_RECEIVER",
                         "Failed to enqueue the Request to the Thread Pool");
                }
            } else {
                LOGE("RESTUNE_REQUEST_RECEIVER",
                     "Thread pool not initialized, Dropping the Request");
            }

            // Only in Case of Tune Signals, Write back the handle to the client.
            if(requestType == REQ_SIGNAL_TUNING) {
                if(write(clientSocket, (const void*)&msgForwardInfo->handle, sizeof(int64_t)) == -1) {
                    TYPELOGV(ERRNO_LOG, "write", strerror(errno));
                }
            }
            break;
        }
        default:
            LOGE("RESTUNE_SOCKET_SERVER", "Invalid Request Type");
            break;
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
    ResourceTunerSocketServer* connection;
    pthread_setname_np(pthread_self(), "listenerThread");
    try {
        connection = new ResourceTunerSocketServer(ResourceTunerSettings::metaConfigs.mListeningPort,
                                                   checkServerOnlineStatus,
                                                   onMsgRecvCallback);
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
