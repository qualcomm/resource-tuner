// Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
// SPDX-License-Identifier: BSD-3-Clause-Clear

#include "ThreadPool.h"

int8_t ThreadPool::threadRoutineHelper() {
    std::unique_lock<std::mutex> threadPoolUniqueLock(this->mThreadPoolMutex, std::defer_lock);
    threadPoolUniqueLock.lock();

    if(this->mTerminatePool == true) {
        threadPoolUniqueLock.unlock();
        return true;
    }

    while(this->mTasksQueueSize == 0) {
        this->mThreadPoolCond.wait(threadPoolUniqueLock);

        if(this->mTerminatePool == true) {
            threadPoolUniqueLock.unlock();
            return true;
        }
    }

    if(this->mTerminatePool == true && this->mTasksQueueSize == 0) {
        threadPoolUniqueLock.unlock();
        return true;
    }

    TaskNode* taskNode = nullptr;
    if(this->mTasksQueueHead != nullptr) {
        taskNode = this->mTasksQueueHead;
        this->mTasksQueueHead = this->mTasksQueueHead->next;

        if(this->mTasksQueueHead == nullptr) {
            this->mTasksQueueTail = nullptr;
        }

        this->mTasksQueueSize--;
    }

    if(taskNode != nullptr && taskNode->taskCallback != nullptr) {
        std::function<void(void*)> task = *taskNode->taskCallback;
        void* args = taskNode->args;

        threadPoolUniqueLock.unlock();

        if(task != nullptr) {
            task(args);
        }
    }

    return false;
}

void ThreadPool::addNewThreads(int32_t mThreadCount) {
    for(int32_t i = 0; i < mThreadCount; i++) {
        try {
            auto threadStartRoutine = ([this] {
                while(true) {
                    try {
                        if(threadRoutineHelper()) {
                            return;
                        }

                    } catch(const std::system_error& e) {
                        TYPELOGV(THREAD_POOL_THREAD_TERMINATED, e.what());
                        return;

                    } catch(const std::exception& e) {
                        TYPELOGV(THREAD_POOL_THREAD_TERMINATED, e.what());
                        return;
                    }
                }
            });

            ThreadNode* thNode = (ThreadNode*)(GetBlock<ThreadNode>());

            try {
                thNode->th = new std::thread(threadStartRoutine);

            } catch (const std::system_error& e) {
                FreeBlock<ThreadNode>(static_cast<void*>(thNode));
                throw;

            } catch (const std::exception& e) {
                FreeBlock<ThreadNode>(static_cast<void*>(thNode));
                throw;
            }

            thNode->next = nullptr;

            if(this->mThreadQueueHead == nullptr) {
                this->mThreadQueueHead = thNode;
                this->mThreadQueueTail = thNode;
            } else {
                this->mThreadQueueTail->next = thNode;
                this->mThreadQueueTail = this->mThreadQueueTail->next;
            }

        } catch(const std::bad_alloc& e) {
            TYPELOGV(THREAD_POOL_THREAD_ALLOCATION_FAILURE, e.what());

        } catch (const std::exception& e) {
            TYPELOGV(THREAD_POOL_THREAD_CREATION_FAILURE, e.what());
        }
    }
}

ThreadPool::ThreadPool(int32_t mDesiredCapacity, int32_t mMaxPending) {
    this->mThreadQueueHead = this->mThreadQueueTail = nullptr;
    this->mTasksQueueHead = this->mTasksQueueTail = nullptr;

    this->mDesiredCapacity = mDesiredCapacity;
    this->mMaxPending = mMaxPending;
    this->mTerminatePool = false;
    this->mTasksQueueSize = 0;

    try {
        MakeAlloc<ThreadNode>(this->mDesiredCapacity);
        MakeAlloc<TaskNode>(this->mDesiredCapacity + this->mMaxPending);

    } catch (const std::bad_alloc& e) {
        TYPELOGV(THREAD_POOL_INIT_FAILURE, e.what());
        throw;
    }

    addNewThreads(mDesiredCapacity);
}

int8_t ThreadPool::enqueueTask(std::function<void(void*)> taskCallback, void* args) {
    try {
        if(taskCallback == nullptr) return false;

        this->mThreadPoolMutex.lock();

        // Create a task Node
        TaskNode* taskNode = (TaskNode*)(GetBlock<TaskNode>());
        if(taskNode == nullptr) {
            TYPELOGD(THREAD_POOL_FULL_ALERT);
            this->mThreadPoolMutex.unlock();

            return false;
        }

        taskNode->taskCallback = new std::function<void(void*)>;
        *taskNode->taskCallback = std::move(taskCallback);

        taskNode->args = args;
        taskNode->next = nullptr;

        if(this->mTasksQueueHead == nullptr) {
            this->mTasksQueueHead = taskNode;
            this->mTasksQueueTail = taskNode;
        } else {
            this->mTasksQueueTail->next = taskNode;
            this->mTasksQueueTail = this->mTasksQueueTail->next;
        }

        this->mTasksQueueSize++;

        this->mThreadPoolMutex.unlock();
        this->mThreadPoolCond.notify_one();

        return true;

    } catch(std::bad_alloc& e) {
        TYPELOGV(THREAD_POOL_ENQUEUE_TASK_FAILURE, e.what());
        return false;

    } catch(std::system_error& e) {
        TYPELOGV(THREAD_POOL_ENQUEUE_TASK_FAILURE, e.what());
        return false;
    }

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
            }

            thNode = nextNode;
        }

        thNode = this->mThreadQueueHead;
        while(thNode != nullptr) {
            ThreadNode* nextNode = thNode->next;
            FreeBlock<ThreadNode>(static_cast<void*>(thNode));
            thNode = nextNode;
        }

        TaskNode* taskNode = this->mTasksQueueHead;
        while(taskNode != nullptr) {
            TaskNode* nextNode = taskNode->next;
            FreeBlock<TaskNode>(static_cast<void*>(taskNode));
            taskNode = nextNode;
        }

    } catch (const std::exception& e) {}
}
