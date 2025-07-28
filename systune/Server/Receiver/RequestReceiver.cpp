// Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
// SPDX-License-Identifier: BSD-3-Clause-Clear

#include "RequestReceiver.h"

std::shared_ptr<RequestReceiver> RequestReceiver::mRequestReceiverInstance = nullptr;
ThreadPool* RequestReceiver::mRequestsThreadPool = nullptr;
RequestReceiver::RequestReceiver() {}

int64_t RequestReceiver::forwardToProvisionerServer(Message* message) {
    if(!ComponentRegistry::isModuleEnabled(MOD_PROVISIONER)) return -1;

    int64_t handle = message->getHandle();

    // Generate and return Handle straightaway
    if(message->getRequestType() == REQ_RESOURCE_TUNING) {
        handle = SystuneSettings::generateUniqueHandle();
        if(handle < 0) {
            // Handle Generation Failure, indicating an Overflow
            Request::cleanUpRequest((Request*)message);
            return -1;
        }
        message->setHandle(handle);
    }

    // Enqueue the Request to the Thread Pool for async processing.
    if(this->mRequestsThreadPool != nullptr) {
        if(!this->mRequestsThreadPool->
            enqueueTask(ComponentRegistry::getModuleMessageHandlerCallback(MOD_PROVISIONER), message)) {
            LOGE("URM_REQUEST_RECEIVER",
                 "Failed to enqueue the Request to the Thread Pool");
            Request::cleanUpRequest((Request*)message);
        }
    } else {
        LOGE("URM_REQUEST_RECEIVER",
             "Thread pool not initialized, Dropping the Request");
        Request::cleanUpRequest((Request*)message);
    }

    return handle;
}

int64_t RequestReceiver::forwardToSysSignalServer(Message* message) {
    if(!ComponentRegistry::isModuleEnabled(MOD_SYSSIGNAL)) return -1;

    int64_t handle = message->getHandle();

    // Generate and return Handle straightaway
    if(message->getRequestType() == SIGNAL_ACQ) {
        handle = SystuneSettings::generateUniqueHandle();
        if(handle < 0) {
            // Handle Generation Failure, indicating an Overflow
            FreeBlock<Signal>(static_cast<void*>(message));
            return -1;
        }
        message->setHandle(handle);
    }

    // Enqueue the Request to the Thread Pool for async processing.
    if(this->mRequestsThreadPool != nullptr) {
        if(!this->mRequestsThreadPool->
            enqueueTask(ComponentRegistry::getModuleMessageHandlerCallback(MOD_SYSSIGNAL), message)) {
            LOGE("URM_REQUEST_RECEIVER",
                 "Failed to enqueue the Request to the Thread Pool");
            FreeBlock<Signal>(static_cast<void*>(message));
        }
    } else {
        LOGE("URM_REQUEST_RECEIVER",
                "Thread pool not initialized, Dropping the Request");
        FreeBlock<Signal>(static_cast<void*>(message));
    }

    return handle;
}

int64_t RequestReceiver::forwardMessage(int32_t callbackID, Message* message) {
    switch(callbackID) {
        case SERVER_ONLINE_CHECK_CALLBACK:
            return SystuneSettings::isServerOnline();
        case PROVISIONER_MESSAGE_RECEIVER_CALLBACK:
            return this->forwardToProvisionerServer(message);
        case SIGNAL_MESSAGE_RECEIVER_CALLBACK:
            return this->forwardToSysSignalServer(message);
        default:
            LOGE("URM_REQUEST_RECEIVER", "Invalid Callback Type = " +
                 std::to_string(callbackID));
            break;
    }

    return 0;
}

int64_t OnSysTuneMessageAsyncCallback(int32_t callbackID, Message* message) {
    std::shared_ptr<RequestReceiver> requestReceiver = RequestReceiver::getInstance();
    return requestReceiver->forwardMessage(callbackID, message);
}

int8_t OnSysTuneMessageSyncCallback(int8_t reqType, void* reqMsg, char* resultBuf, uint64_t bufferSize) {
    switch(reqType) {
        case REQ_SYSCONFIG_GET_PROP:
        case REQ_SYSCONFIG_SET_PROP: {
            SysConfig* config = (SysConfig*) reqMsg;
            if(ComponentRegistry::getModuleMessageHandlerCallback(MOD_SYS_CONFIG) != nullptr) {
                // return ComponentRegistry::getModuleMessageHandlerCallback(MOD_SYS_CONFIG)(reqMsg);
            } else {
                LOGE("URM_REQUEST_RECEIVER", "No Callback registered for SysConfig");
            }
            break;
        }
        default:
            break;
    }

    return -1;
}

void listenerThreadStartRoutine() {
    SystuneSocketServer* connection;
    try {
        connection = new SystuneSocketServer(12000,
                                             OnSysTuneMessageAsyncCallback,
                                             OnSysTuneMessageSyncCallback);
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
