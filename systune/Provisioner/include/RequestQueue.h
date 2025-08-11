// Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
// SPDX-License-Identifier: BSD-3-Clause-Clear

#ifndef REQUEST_QUEUE_H
#define REQUEST_QUEUE_H

#include "Utils.h"
#include "Request.h"
#include "OrderedQueue.h"

class RequestQueue : public OrderedQueue {
private:
    static std::shared_ptr<RequestQueue> mRequestQueueInstance;
    static std::mutex instanceProtectionLock;

    RequestQueue();

public:
    ~RequestQueue();

    void orderedQueueConsumerHook();

    static std::shared_ptr<RequestQueue> getInstance() {
        if(mRequestQueueInstance == nullptr) {
            instanceProtectionLock.lock();
            if(mRequestQueueInstance == nullptr) {
                try {
                    mRequestQueueInstance = std::shared_ptr<RequestQueue> (new RequestQueue());
                } catch(const std::bad_alloc& e) {
                    instanceProtectionLock.unlock();
                    return nullptr;
                }
            }
            instanceProtectionLock.unlock();
        }
        return mRequestQueueInstance;
    }
};

#endif
