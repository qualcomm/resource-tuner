// Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
// SPDX-License-Identifier: BSD-3-Clause-Clear

#ifndef THREAD_POOL_H
#define THREAD_POOL_H

/*!
 * \file  ThreadPool.h
 */

/*!
 * \ingroup THREAD_POOL
 * \defgroup THREAD_POOL Thread Pool
 * \details Used to Pre-Allocate Worker Capacity. As part of the ThreadPool instance creation,
 *          User needs to specify 2 parameters:
 *          1. Desired Capacity: Number of threads to be created as part of the Pool.\n
 *          2. Max Pool Capacity: The size upto which the Thread Pool can scale, to accomodate growing demand.
 *
 *          - When a task is submitted (via the enqueueTask API), first we check if there
 *            any spare or free threads in the Pool, if there are we assign the task to one
 *            of those threads.\n\n
 *          - However if no threads are currently free, we check if the pool can be expanded (by adding
 *            additional threads to accomodate the request).\n\n
 *          - If even that is not possible, the request is dropped.\n\n
 *
 *          Any new threads create to scale up to increased demand shall be destroyed after some
 *          predefined interval of inactivity (i.e. scale down in response to decreased demand).\n\n
 *
 *          Internally the ThreadPool implementation makes use of Condition Variables. Initially
 *          when the pool is created, the threads put themselves to sleep by calling wait on this
 *          Condition Variable. Whenever a task comes in, one of these threads (considering there
 *          are threads available in the pool), will be woken up and it will pick up the new task.
 *
 * @{
 */

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
#include "SafeOps.h"

class TaskNode {
public:
    std::function<void(void*)>* taskCallback;
    void* args;
    TaskNode* next;

    TaskNode();
    ~TaskNode();
};

struct ThreadNode {
    std::thread* th;
    ThreadNode* next;
};

class TaskQueue {
private:
    int32_t size;
    TaskNode* head;
    TaskNode* tail;

public:
    TaskQueue();

    void add(TaskNode* taskNode);
    TaskNode* poll();
    int8_t isEmpty();
    int32_t getSize();

    ~TaskQueue();
};

static const int32_t maxLoadPerThread = 3;

/**
 * @brief ThreadPool
 * @details Pre-Allocate thread (Workers) capacity for future use, so as to prevent repeated Thread creation / destruction costs.
 */
class ThreadPool {
private:
    int32_t mDesiredPoolCapacity; //!< Desired or Base Thread Pool Capacity
    int32_t mMaxPoolCapacity; //!< Max Capacity upto which the Thread Pool can scale up.

    int32_t mCurrentThreadsCount;
    int32_t mTotalTasksCount;
    int8_t mTerminatePool;

    std::string mThreadPoolLabel;

    TaskQueue* mCurrentTasks;

    ThreadNode* mThreadQueueHead;
    ThreadNode* mThreadQueueTail;

    std::mutex mThreadPoolMutex;
    std::condition_variable mThreadPoolCond;

    TaskNode* createTaskNode(std::function<void(void*)> taskCallback, void* args);
    int8_t addNewThread(int8_t isCoreThread);
    int8_t threadRoutineHelper(int8_t isCoreThread);

public:
    ThreadPool(int32_t desiredCapacity, int32_t maxCapacity, std::string label="");
    ~ThreadPool();

    /**
     * @brief Enqueue a task for processing by one of ThreadPool's thread.
     * @param taskCallback function pointer to the task.
     * @param arg Pointer to the task arguments.
     * @return int8_t:\n
     *            - 1 if the request was successfully enqueued,
     *            - 0 otherwise.
     */
    int8_t enqueueTask(std::function<void(void*)> callBack, void* arg);
};

#endif

/*! @} */
