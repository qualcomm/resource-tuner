// Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
// SPDX-License-Identifier: BSD-3-Clause-Clear

#include "ThreadPool.h"

TaskQueue::TaskQueue() {
    this->size = 0;
    this->head = this->tail = nullptr;
}

void TaskQueue::add(TaskNode* taskNode) {
    if(taskNode == nullptr) return;

    if(this->head == nullptr) {
        this->head = taskNode;
        this->tail = taskNode;
    } else {
        this->tail->next = taskNode;
        this->tail = this->tail->next;
    }
    this->size++;
}

TaskNode* TaskQueue::poll() {
    TaskNode* taskNode = nullptr;
    if(this->size > 0) {
        if(this->head != nullptr) {
            taskNode = this->head;
            this->head = this->head->next;

            if(this->head == nullptr) {
                this->tail = nullptr;
            }
            this->size--;
        }
    }
    return taskNode;
}

int8_t TaskQueue::isEmpty() {
    return this->size == 0;
}

int32_t TaskQueue::getSize() {
    return this->size;
}

int8_t ThreadPool::threadRoutineHelper(int8_t isCoreThread, int8_t& firstTask) {
    try {
        std::unique_lock<std::mutex> threadPoolUniqueLock(this->mThreadPoolMutex);

        // Check for any Pending tasks
        if(!this->mTerminatePool && this->mTotalTasksCount == 0) {
            // Update the Count of threads in Use
            if(isCoreThread) {
                this->mCoreThreadsInUse = std::max(this->mCoreThreadsInUse - 1, 0);
                this->mThreadPoolCond.wait(threadPoolUniqueLock,
                                           [this]{return this->mTotalTasksCount > 0 ||
                                                         this->mTerminatePool;});
            } else {
                // Expandable Thread
                int8_t awakeStatus = this->mThreadPoolCond.wait_for(
                                            threadPoolUniqueLock,
                                            std::chrono::seconds(10 * 60),
                                            [this]{return this->mTotalTasksCount > 0 ||
                                                          this->mTerminatePool;});
                if(!awakeStatus) {
                    // If the Thread was woken up due to the Time Interval expiring, then
                    // Proceed with Thread Termination. As it indicates the Thread has been
                    // idle for the last 10 mins.
                    this->mCurrentThreadsCount--;
                    return true;
                }
            }
        }

        // Check if the pool has been terminated. If so, exit the thread.
        if(this->mTerminatePool == true) {
            return true;
        }

        if(!firstTask) {
            // Move a task from Pending List to CurrentList
            if(!this->mWaitingList->isEmpty()) {
                this->mCurrentTasks->add(this->mWaitingList->poll());
            }
        }

        TaskNode* taskNode = this->mCurrentTasks->poll();
        this->mTotalTasksCount--;
        firstTask = false;

        if(taskNode != nullptr && taskNode->taskCallback != nullptr) {
            std::function<void(void*)> task = *taskNode->taskCallback;
            void* args = taskNode->args;

            threadPoolUniqueLock.unlock();

            if(task != nullptr) {
                task(args);
            }
        }

        // Free the TaskNode, before proceeding to the next task
        FreeBlock<TaskNode>(taskNode);
        return false;

    } catch(const std::system_error& e) {
        TYPELOGV(THREAD_POOL_THREAD_TERMINATED, e.what());
        return true;

    } catch(const std::exception& e) {
        TYPELOGV(THREAD_POOL_THREAD_TERMINATED, e.what());
        return true;
    }
}

int8_t ThreadPool::addNewThread(int8_t isCoreThread) {
    // First Create a ThreadNode for this thread
    ThreadNode* thNode = nullptr;
    try {
        thNode = (ThreadNode*)(GetBlock<ThreadNode>());

    } catch(const std::bad_alloc& e) {
        return false;
    }

    thNode->next = nullptr;

    try {
        auto threadStartRoutine = ([this](int8_t isCoreThread) {
            while(true) {
                int8_t firstTask = true;
                if(threadRoutineHelper(isCoreThread, firstTask)) {
                    return;
                }
            }
        });

        try {
            thNode->th = new std::thread(threadStartRoutine, isCoreThread);

        } catch(const std::system_error& e) {
            FreeBlock<ThreadNode>(static_cast<void*>(thNode));
            throw;

        } catch(const std::bad_alloc& e) {
            FreeBlock<ThreadNode>(static_cast<void*>(thNode));
            throw;
        }

        // Add this ThreadNode to the ThreadList
        if(this->mThreadQueueHead == nullptr) {
            this->mThreadQueueHead = thNode;
            this->mThreadQueueTail = thNode;
        } else {
            this->mThreadQueueTail->next = thNode;
            this->mThreadQueueTail = this->mThreadQueueTail->next;
        }

    } catch(const std::exception& e) {
        TYPELOGV(THREAD_POOL_THREAD_CREATION_FAILURE, e.what());
    }

    return false;
}

ThreadPool::ThreadPool(int32_t desiredCapacity, int32_t maxPending, int32_t maxCapacity) {
    this->mThreadQueueHead = this->mThreadQueueTail = nullptr;

    this->mDesiredPoolCapacity = desiredCapacity;
    // make it iterative
    this->mCurrentThreadsCount = desiredCapacity;
    this->mMaxWaitingListCapacity = maxPending;

    if(maxCapacity < desiredCapacity) {
        maxCapacity = desiredCapacity;
    }
    this->mMaxPoolCapacity = maxCapacity;

    this->mCoreThreadsInUse = 0;
    this->mTotalTasksCount = 0;
    this->mTerminatePool = false;

    try {
        this->mCurrentTasks = new TaskQueue;
        this->mWaitingList = new TaskQueue;

        MakeAlloc<ThreadNode>(20);
        MakeAlloc<TaskNode>(50);

    } catch (const std::bad_alloc& e) {
        TYPELOGV(THREAD_POOL_INIT_FAILURE, e.what());
    }

    // Add desired number of Threads to the Pool
    for(int32_t i = 0; i < this->mDesiredPoolCapacity; i++) {
        this->addNewThread(true);
    }
}

TaskNode* ThreadPool::createTaskNode(std::function<void(void*)> taskCallback, void* args) {
    // Create a task Node
    TaskNode* taskNode = nullptr;
    try {
        taskNode = (TaskNode*)(GetBlock<TaskNode>());

    } catch(const std::bad_alloc& e) {
        return nullptr;
    }

    try {
        taskNode->taskCallback = new std::function<void(void*)>;
        *taskNode->taskCallback = std::move(taskCallback);

    } catch(const std::bad_alloc& e) {
        FreeBlock<TaskNode>(static_cast<void*>(taskNode));
        return nullptr;
    }

    taskNode->args = args;
    taskNode->next = nullptr;

    return taskNode;
}

int8_t ThreadPool::enqueueTask(std::function<void(void*)> taskCallback, void* args) {
    try {
        if(taskCallback == nullptr) return false;

        std::unique_lock<std::mutex> threadPoolUniqueLock(this->mThreadPoolMutex);
        int8_t taskAccepted = false;

        // First Check if any Thread in the Core Group is Available
        // If it is, assign the Task to that Thread.
        if(this->mCoreThreadsInUse < this->mDesiredPoolCapacity) {
            // Add the task to the Current List
            TaskNode* taskNode = createTaskNode(taskCallback, args);
            if(taskNode == nullptr) {
                throw std::bad_alloc();
            }

            this->mCurrentTasks->add(taskNode);
            this->mCoreThreadsInUse++;
            taskAccepted = true;
        }

        // If no Core Threads are available, Add the Task to the Pending Queue
        // If there are empty slots in the Pending Queue
        int32_t pendingQueueSize = this->mWaitingList->getSize();
        if(!taskAccepted && pendingQueueSize < this->mMaxWaitingListCapacity) {
            // Add the task to the Pending List
            TaskNode* taskNode = createTaskNode(taskCallback, args);
            if(taskNode == nullptr) {
                throw std::bad_alloc();
            }

            this->mWaitingList->add(taskNode);
            taskAccepted = true;
        }

        // Check if the Pool can be expanded to accomodate this Request
        if(!taskAccepted && this->mCurrentThreadsCount < this->mMaxPoolCapacity) {
            this->addNewThread(false);

            // Add the task to the current List
            TaskNode* taskNode = createTaskNode(taskCallback, args);
            if(taskNode == nullptr) {
                throw std::bad_alloc();
            }

            this->mCurrentTasks->add(taskNode);
            this->mCurrentThreadsCount++;
            taskAccepted = true;
        }

        if(taskAccepted) {
            this->mTotalTasksCount++;
            this->mThreadPoolCond.notify_one();
            return true;
        }

        TYPELOGD(THREAD_POOL_FULL_ALERT);
        return false;

    } catch(std::bad_alloc& e) {
        TYPELOGV(THREAD_POOL_ENQUEUE_TASK_FAILURE, e.what());
        return false;

    } catch(std::system_error& e) {
        TYPELOGV(THREAD_POOL_ENQUEUE_TASK_FAILURE, e.what());
        return false;
    }

    TYPELOGD(THREAD_POOL_FULL_ALERT);
    return false;
}

ThreadPool::~ThreadPool() {
    try {
        // Terminate all the threads
        this->mThreadPoolMutex.lock();
        this->mTerminatePool = true;
        this->mThreadPoolCond.notify_all();
        this->mThreadPoolMutex.unlock();

        ThreadNode* thNode = this->mThreadQueueHead;
        while(thNode != nullptr) {
            ThreadNode* nextNode = thNode->next;

            try {
                if(thNode->th != nullptr && thNode->th->joinable()) {
                    thNode->th->join();
                }
            } catch (const std::exception& e) {}

            if(thNode->th != nullptr) {
                delete thNode->th;
                thNode->th = nullptr;
            }

            thNode = nextNode;
        }

        thNode = this->mThreadQueueHead;
        while(thNode != nullptr) {
            ThreadNode* nextNode = thNode->next;
            FreeBlock<ThreadNode>(static_cast<void*>(thNode));
            thNode = nextNode;
        }

        while(!this->mCurrentTasks->isEmpty()) {
            TaskNode* taskNode = this->mCurrentTasks->poll();
            if(taskNode != nullptr) {
                FreeBlock<TaskNode>(static_cast<void*>(taskNode));
            }
        }

    } catch (const std::exception& e) {}
}
