// Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
// SPDX-License-Identifier: BSD-3-Clause-Clear

#ifndef RESOURCE_TUNER_TIMER_H
#define RESOURCE_TUNER_TIMER_H

#include <chrono>
#include <thread>
#include <atomic>
#include <functional>
#include <mutex>
#include <condition_variable>

#include "ThreadPool.h"

/**
 * @brief Timer
 */
class Timer {
private:
    int64_t mDuration; //!< Duration of the timer.
    int8_t mIsRecurring; //!< Flag to set a recurring timer. It is never modified. False by default.
    std::atomic<int8_t> mStop; //!< Flag to let the timer thread know it has been killed.
    std::condition_variable mCv; //!< Condition variable to stop the thread for the timer duration and wake up either after the duration has ended or the timer is killed.
    std::mutex mMutex; //!<Mutex to protect the condition variable.
    std::thread mThread; //!<Thread in which timer will run.
    std::function<void(void*)> mCallBack; //!< Callback function to be called after timer is over.

    void implementTimer();

public:
    // Create the ThreadPool as a static member so that all the Timer objects can share it.
    static ThreadPool* mTimerThreadPool;

    /**
     * @brief Initialize the Timer
     * @param callBack Function that needs to be invoked after the specified timer duration
     *                 (in milliseconds) has expired.\n
     * @param isRecurring Flag to indicate if a timer is recurring (false by default).
     *                    If set to true, the registered callback will be called after every
     *                    time interval window, say 5 seconds.
     *                    This flag is useful for daemon threads like Pulse Monitor.
     */
    Timer(std::function<void(void*)> callBack, int8_t isRecurring=false);
    ~Timer();

    /**
     * @brief Starts the timer for the given duration in milliseconds
     * @details As part of this routine, a task is submitted to the Thread Pool, this task forces
     *          the thread (which picked up the submitted task) to wait (on Condition Variable)
     *          for the given duration (wait_for). When woken up, the thread atomically checks if the timer was
     *          updated or not. Following which it it executes a pre-registered callback function.
     * @param duration Time Interval (in milliseconds) after which the Callback needs
     *                 needs to be triggered.
     * @return int8_t:\n
     *            - 1 if the timer was successfully started\n
     *            - 0 otherwise.
     */
    int8_t startTimer(int64_t duration);

    /**
     * @brief Invalidates current timer.
     */
    void killTimer();
};

#endif
