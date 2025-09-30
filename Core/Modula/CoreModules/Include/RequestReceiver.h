// Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
// SPDX-License-Identifier: BSD-3-Clause-Clear

#ifndef REQUEST_RECEIVER_H
#define REQUEST_RECEIVER_H

#include <cstdint>
#include <memory>
#include <cstring>
#include <fstream>
#include <sstream>

#include "Logger.h"
#include "ResourceTunerSettings.h"
#include "AuxRoutines.h"
#include "ComponentRegistry.h"

/**
 * @brief RequestReceiver
 * @details Handles incoming client-requests, by forwarding the request to the
 *          appropriate module, using that module's registered callback.
 *          Note, the callback is invoked on a separate thread (taken from the ThreadPool)
 */
class RequestReceiver {
private:
    static std::shared_ptr<RequestReceiver> mRequestReceiverInstance;
    static std::mutex instanceProtectionLock;
    ThreadPool* mRequestsThreadPool;
    ThreadPool* mTimerThreadPool;

    RequestReceiver();

public:
    ~RequestReceiver();

    ThreadPool* getRequestThreadPool();
    ThreadPool* getTimerThreadPool();

    ErrCode preAllocateWorkers();

    static std::shared_ptr<RequestReceiver> getInstance() {
        if(mRequestReceiverInstance == nullptr) {
            instanceProtectionLock.lock();
            if(mRequestReceiverInstance == nullptr) {
                try {
                    mRequestReceiverInstance = std::shared_ptr<RequestReceiver> (new RequestReceiver());
                } catch(const std::bad_alloc& e) {
                    instanceProtectionLock.unlock();
                    return nullptr;
                }
            }
            instanceProtectionLock.unlock();
        }
        return mRequestReceiverInstance;
    }
};

void listenerThreadStartRoutine();

#endif
