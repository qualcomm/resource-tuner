// Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
// SPDX-License-Identifier: BSD-3-Clause-Clear

#ifndef ORDERED_QUEUE_H
#define ORDERED_QUEUE_H

#include <queue>
#include <vector>
#include <mutex>
#include <condition_variable>

#include "Message.h"
#include "MemoryPool.h"
#include "Utils.h"

/**
 * @brief This class represents a mutex-protected multiple producer, single consumer priority queue.
 * @details The Queue items are ordered by their Priority, so that the Queue Item with the highest
 *          Priority is always served first.
 */
class OrderedQueue {
protected:
    int32_t mElementCount;
    std::mutex mOrderedQueueMutex;
    std::condition_variable mOrderedQueueCondition;
    int8_t lockStatus;

    struct QueueOrdering {
        int8_t operator() (Message* &a,  Message* &b) {
            return a->getPriority() > b->getPriority();
        }
    };

    /**
     * @brief Core OrderedQueue Data Structure, to store the Requests pushed by the Publisher threads.
     *        Makes use of std::priority_queue, which is ordered here based on the Request Priorities.
     */
    std::priority_queue<Message*, std::vector<Message*>, QueueOrdering> mOrderedQueue;

public:
    OrderedQueue();
    ~OrderedQueue();

    /**
     * @brief Used by the producers to add a new request to the OrderedQueue.
     * @details This routine will wake up the consumer end to process the task.
     * @param req Pointer to the Request
     * @return int8_t:\n
     *            - 1: If the Request was successfully added to the Request Queue
     *            - 0: otherwise
     */
    int8_t addAndWakeup(Message* req);

    /**
     * @brief Provides a mechanism, to hook or plug-in the Consumer Code.
     * @details Using this routine, the consumer can safely (lock-protected) extract
     *          and process Requests enqueued by the Producers.
     */
    virtual void orderedQueueConsumerHook() = 0;

    /**
     * @brief Used by the consumer end to poll a request from the OrderedQueue
     * @details This routine will return the Request with the highest priority to the consumer
     *          and remove it from the OrderedQueue.
     *          If the OrderedQueue is empty this function returns a null pointer.
     * @return void*:\n
     *           - Pointer to the request polled
     */
    Message* pop();

    /**
     * @brief Used by the Consumer end to wait for Requests.
     * @details This routine will put the consumer to sleep.
     */
    void wait();

    /**
     * @brief Used by the consumer to check if there are any pending requests in the OrderedQueue
     * @return int8_t:\n
     *            - 1: If there are pending Requests
     *            - 0: otherwise
     */
    int8_t hasPendingTasks();

    void forcefulAwake();
};

#endif
