// Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
// SPDX-License-Identifier: BSD-3-Clause-Clear

#ifndef REQUEST_RECEIVER_H
#define REQUEST_RECEIVER_H

#include <cstdint>
#include <cstring>
#include <fstream>
#include <sstream>

#include "Logger.h"
#include "SystuneSocketServer.h"
#include "SystuneSettings.h"
#include "SysConfig.h"
#include "ComponentRegistry.h"

#include <memory>

class RequestReceiver {
private:
    static std::shared_ptr<RequestReceiver> mRequestReceiverInstance;
    int64_t forwardToProvisionerServer(Message* message);
    int64_t forwardToSysSignalServer(Message* message);

    RequestReceiver();

public:
    static ThreadPool* mRequestsThreadPool;

    int64_t forwardMessage(int32_t callbackID, Message* message);

    static std::shared_ptr<RequestReceiver> getInstance() {
        if(mRequestReceiverInstance == nullptr) {
            mRequestReceiverInstance = std::shared_ptr<RequestReceiver>(new RequestReceiver());
        }
        return mRequestReceiverInstance;
    }
};

int64_t OnSysTuneMessageReceiveCallback(int32_t callbackID, Message* reqMsg, char* resultBuf);

void listenerThreadStartRoutine();

#endif
