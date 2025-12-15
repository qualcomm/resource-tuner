// Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
// SPDX-License-Identifier: BSD-3-Clause-Clear

#include "OrderedQueue.h"

OrderedQueue::OrderedQueue() {
    this->mElementCount = 0;
}

int8_t OrderedQueue::addAndWakeup(Message* queueItem) {
    try {
        const std::unique_lock<std::mutex> lock(this->mOrderedQueueMutex);

        if(queueItem == nullptr) return false;
        if(queueItem->getPriority() < SERVER_CLEANUP_TRIGGER_PRIORITY) return false;

        this->mOrderedQueue.push(queueItem);
        this->mElementCount++;

        this->mOrderedQueueCondition.notify_one();
        return true;

    } catch(const std::exception& e) {
        TYPELOGV(GENERIC_CALL_FAILURE_LOG, e.what());
        return false;
    }

    return false;
}

void OrderedQueue::wait() {
    try {
        std::unique_lock<std::mutex> lock(this->mOrderedQueueMutex);

        while(this->mElementCount == 0) {
            this->mOrderedQueueCondition.wait(lock);
        }

        this->orderedQueueConsumerHook();
        lock.unlock();

    } catch(const std::system_error& e) {
        TYPELOGV(GENERIC_CALL_FAILURE_LOG, e.what());

    } catch(const std::exception& e) {
        TYPELOGV(GENERIC_CALL_FAILURE_LOG, e.what());
    }
}

int8_t OrderedQueue::hasPendingTasks() {
    return (this->mElementCount > 0);
}

Message* OrderedQueue::pop() {
    if(this->mElementCount == 0) {
        return nullptr;
    }

    // No need to acquire lock. Consumer should call the "pop" routine
    // while holding the lock acquired as part of "wait" routine.

    Message* queueItem = this->mOrderedQueue.top();
    this->mOrderedQueue.pop();
    this->mElementCount--;

    return queueItem;
}

void OrderedQueue::forcefulAwake() {
    Message* message = nullptr;
    try {
        message = MPLACED(Message);
    } catch(const std::bad_alloc& e) {
        return;
    }

    message->setPriority(SERVER_CLEANUP_TRIGGER_PRIORITY);
    this->addAndWakeup(message);
}

OrderedQueue::~OrderedQueue() {}
