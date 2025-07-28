// Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
// SPDX-License-Identifier: BSD-3-Clause-Clear

#ifndef THREAD_POOL_H
#define THREAD_POOL_H

#include <assert.h>
#include <iostream>
#include <thread>
#include <vector>
#include <queue>
#include <functional>
#include <unistd.h>
#include <mutex>
#include <condition_variable>
#include <exception>

#include "Utils.h"
#include "Logger.h"
#include "MemoryPool.h"

struct TaskNode {
    std::function<void(void*)>* taskCallback;
    void* args;
    TaskNode* next;
};

struct ThreadNode {
    std::thread* th;
    struct ThreadNode* next;
};

/**
* @brief ThreadPool
* @details Pre-Allocate thread capacity for future use, so as to prevent repeated Thread creation / destruction costs.
*/
class ThreadPool {
private:
    int32_t mMaxPending;
    int32_t mDesiredCapacity;

    int8_t mTerminatePool;

    ThreadNode* mThreadQueueHead;
    ThreadNode* mThreadQueueTail;

    // Pending WAIT queue List: RENAME
    TaskNode* mTasksQueueHead;
    TaskNode* mTasksQueueTail;
    int32_t mTasksQueueSize;

    std::mutex mThreadPoolMutex;
    std::condition_variable mThreadPoolCond;

    void addNewThreads(int32_t mThreadCount);
    int8_t threadRoutineHelper();

public:
    ThreadPool(int32_t mDesiredCapacity, int32_t mMaxPending);
    ~ThreadPool();

    /**
     * @brief Enqueue a task for processing by one of ThreadPool's thread.
     * @param taskCallback function pointer to the task.
     * @param arg Pointer to the task arguments.
     * @return int8_t:
     *            1 if the request was successfully enqueued,
     *            0 otherwise.
     */
    int8_t enqueueTask(std::function<void(void*)> callBack, void* arg);
};

#endif
