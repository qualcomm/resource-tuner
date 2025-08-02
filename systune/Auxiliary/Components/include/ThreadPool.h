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
 *          2. Max Pending Queue Size: The size upto which the Pending Queue can grow.
 *
 *          - When a task is submitted (via the enqueueTask API), first we check if there
 *          any spare or free threads in the Pool, if there are we assign the task to one
 *          of those threads.\n\n
 *          - However if no threads are currently free, we check the size of the Waiting Queue,
 *          If the size is less than the Max Threshold then we add the task to the Waiting Queue.
 *          As soon as any of the threads is available, it will Poll this task from the Queue and
 *          Process it.\n\n
 *          - If even the Pending Queue is full, then the task is Dropped.\n\n
 *
 *          Internally the ThreadPool implementation makes use of Condition Variables. Initially
 *          when the pool is created, the threads put themselves to sleep by calling wait on this
 *          Condition Variable. Whenever a task comes in, One of these threads (considering there
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

/**
* @brief ThreadPool
* @details Pre-Allocate thread (Workers) capacity for future use, so as to prevent repeated Thread creation / destruction costs.
*/
class ThreadPool {
private:
    int32_t mMaxWaitingListCapacity;
    int32_t mDesiredPoolCapacity;
    int32_t mMaxPoolCapacity;

    int32_t mCoreThreadsInUse;
    int32_t mCurrentThreadsCount;
    int32_t mTotalTasksCount;
    int8_t mTerminatePool;

    TaskQueue* mCurrentTasks;
    TaskQueue* mWaitingList;

    ThreadNode* mThreadQueueHead;
    ThreadNode* mThreadQueueTail;

    std::mutex mThreadPoolMutex;
    std::condition_variable mThreadPoolCond;

    TaskNode* createTaskNode(std::function<void(void*)> taskCallback, void* args);
    int8_t addNewThread(int8_t isCoreThread);
    int8_t threadRoutineHelper(int8_t isCoreThread, int8_t& firstTask);

public:
    ThreadPool(int32_t desiredCapacity, int32_t maxPending, int32_t maxCapacity);
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

/*! @} */
