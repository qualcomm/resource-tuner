// Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
// SPDX-License-Identifier: BSD-3-Clause-Clear

#ifndef REQUEST_RECEIVER_H
#define REQUEST_RECEIVER_H

#include <cstdint>
#include <cstring>
#include <fstream>
#include <sstream>

#include "Logger.h"
#include "ResourceTunerSocketServer.h"
#include "ResourceTunerSettings.h"
#include "SysConfig.h"
#include "AuxRoutines.h"
#include "ComponentRegistry.h"

#include <memory>

/**
 * @brief RequestReceiver
 * @details Handles incoming client-requests, by forwarding the request to the
 *          appropriate module, using that module's registered callback.
 *          Note, the callback is invoked on a separate thread (taken from the ThreadPool)
 */
class RequestReceiver {
private:
    static std::shared_ptr<RequestReceiver> mRequestReceiverInstance;

    RequestReceiver();

public:
    static ThreadPool* mRequestsThreadPool;

    void forwardMessage(int32_t clientSocket, MsgForwardInfo* msgForwardInfo);

    static std::shared_ptr<RequestReceiver> getInstance() {
        if(mRequestReceiverInstance == nullptr) {
            mRequestReceiverInstance = std::shared_ptr<RequestReceiver>(new RequestReceiver());
        }
        return mRequestReceiverInstance;
    }
};

void OnResourceTunerMessageReceiverCallback(int32_t callbackID, char* message, uint64_t bufferSize);

void listenerThreadStartRoutine();

#endif
