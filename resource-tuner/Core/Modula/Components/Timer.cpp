// Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
// SPDX-License-Identifier: BSD-3-Clause-Clear

#include "Timer.h"

ThreadPool* Timer::mTimerThreadPool = nullptr;

Timer::Timer(std::function<void(void*)>callBack, int8_t isRecurring) {
    this->mTimerStop.store(false);
    this->mIsRecurring = isRecurring;
    this->mCallback = callBack;
}

void Timer::implementTimer() {
    try {
        do {
            std::unique_lock<std::mutex> lock(this->mTimerMutex);
            this->mTimerCond.wait_for(lock, std::chrono::milliseconds(this->mDuration),
                                     [this]{return mTimerStop.load();});
            lock.unlock();
            if(!this->mTimerStop.load()) {
                if(this->mCallback) {
                    this->mCallback(nullptr);
                }
            }
        } while(this->mIsRecurring && !this->mTimerStop.load());
    } catch(const std::exception& e) {
        LOGE("RESTUNE_TIMER", "Timer Could not be started, Error: " + std::string(e.what()));
    }
}

int8_t Timer::startTimer(int64_t duration) {
    if(duration == -1) {
        return true;
    }

    if(duration == 0 || duration < -1) {
        return false;
    }

    this->mDuration = duration;

    if(mTimerThreadPool == nullptr) {
        return false;
    }

    if(!mTimerThreadPool->enqueueTask(std::bind(&Timer::implementTimer, this), nullptr)) {
        return false;
    }

    LOGD("RESTUNE_TIMER", "Timer Event Created, duration: " + std::to_string(duration));
    return true;
}

void Timer::killTimer() {
    LOGD("RESTUNE_TIMER", "Killing timer");
    this->mTimerStop.store(true);
    this->mTimerCond.notify_all();
}

Timer::~Timer() {
    this->killTimer();
}
