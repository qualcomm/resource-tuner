// Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
// SPDX-License-Identifier: BSD-3-Clause-Clear

#ifndef SIGNAL_QUEUE_H
#define SIGNAL_QUEUE_H

#include "Utils.h"
#include "Signal.h"
#include "OrderedQueue.h"

/**
 * @brief This class represents a mutex-protected multiple producer, single consumer priority queue. 
 * @details It stores the pointer to the request and compares their priorities. A server thread picks up
 *          these requests sequentially and processes them as appropriate.
 */
class SignalQueue : public OrderedQueue {
private:
    static std::shared_ptr<SignalQueue> mSignalQueueInstance;
    static std::mutex instanceProtectionLock;

    SignalQueue();

public:
    ~SignalQueue();

    void orderedQueueConsumerHook();

    static std::shared_ptr<SignalQueue> getInstance() {
        if(mSignalQueueInstance == nullptr) {
            instanceProtectionLock.lock();
            if(mSignalQueueInstance == nullptr) {
                try {
                    mSignalQueueInstance = std::shared_ptr<SignalQueue> (new SignalQueue());
                } catch(const std::bad_alloc& e) {
                    instanceProtectionLock.unlock();
                    return nullptr;
                }
            }
            instanceProtectionLock.unlock();
        }
        return mSignalQueueInstance;
    }
};

#endif
