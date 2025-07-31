// Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
// SPDX-License-Identifier: BSD-3-Clause-Clear

#ifndef SYSTUNE_TIMER_H
#define SYSTUNE_TIMER_H

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
    int64_t mDuration;//<! Duration of the timer.
    int8_t mIsRecurring; //<! Flag to set a recurring timer. It is never modified. False by default.
    std::atomic<int8_t> mStop; //<! Flag to let the timer thread know it has been killed.
    std::condition_variable mCv; //<! Condition variable to stop the thread for the timer duration and wake up either after the duration has ended or the timer is killed.
    std::mutex mMutex; //<!Mutex to protect the condition variable.
    std::thread mThread; //<!Thread in which timer will run.
    std::function<void(void*)> mCallBack; //<! Callback function to be called after timer is over.

    // Create the ThreadPool instance as static so that all the Timer objects can share it.

    void implementTimer();

    public:
    Timer(std::function<void(void*)> callBack, int8_t isRecurring=false);

    ~Timer();

    static ThreadPool* mTimerThreadPool;

    /**
    * @brief Starts the timer for the given duration in milliseconds
    * @details It spawns an independent thread that Waits (on Condition Variable)
    *          for the given duration. When woken up, it atomically checks if the timer was
    *          updated or not. Following which it it executes a pre-registered callback function.
    * @param duration
    */
    int8_t startTimer(int64_t duration);

    /**
    * @brief Updates the same timer duration to some higher value.
    * @details Updates the value of an atomic variable and acts the same way as
    *          starting a new timer while invalidating the old timer
    *
    * @param duration Updated duration of timer.
    */
    int8_t updateTimer(int64_t duration);

    /**
    * @brief Invalidates current timer.
    */
    void killTimer();
};

#endif
