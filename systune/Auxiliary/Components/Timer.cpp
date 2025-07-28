// Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
// SPDX-License-Identifier: BSD-3-Clause-Clear

#include "Timer.h"
#include "Logger.h"

ThreadPool* Timer::mTimerThreadPool = nullptr;

Timer::Timer(std::function<void(void*)>callBack, int8_t isRecurring) {
    mStop.store(false);
    mIsRecurring = isRecurring;
    this->mCallBack = callBack;
}

void Timer::implementTimer() {
    do {
        std::unique_lock<std::mutex> lock(mMutex);
        mCv.wait_for(lock, std::chrono::milliseconds(this->mDuration), [this]{return mStop.load();});
        lock.unlock();
        if(!mStop.load()) {
            if(this->mCallBack) {
                this->mCallBack(nullptr);
            }
        }
    } while(mIsRecurring && !mStop.load());
}

int8_t Timer::startTimer(int64_t duration) {
    if(duration <= 0) {
        return false;
    }

    this->mDuration = duration;

    if(mTimerThreadPool == nullptr) {
        return false;
    }

    if(!mTimerThreadPool->enqueueTask(std::bind(&Timer::implementTimer, this), nullptr)) {
        return false;
    }

    LOGD("URM_TIMER", "Timer started with " + std::to_string(duration));
    return true;
}

int8_t Timer::updateTimer(int64_t duration) {
    if(duration != - 1 && (duration < this->mDuration)) {
        return false;
    }

    LOGD("URM_TIMER", "Timer updated to: " + std::to_string(duration));
    killTimer();

    if(duration != -1) {
        // Re-Trigger the Timer
        this->mDuration = duration;
        mStop.store(false);

        if(mTimerThreadPool == nullptr) {
            return false;
        }

        if(!mTimerThreadPool->enqueueTask(std::bind(&Timer::implementTimer, this), nullptr)) {
            return false;
        }

        LOGD("URM_TIMER", "Timer started with " + std::to_string(duration));
    }

    return true;
}

void Timer::killTimer() {
    LOGD("URM_TIMER", "Killing timer");
    mStop.store(true);
    mCv.notify_all();
}

Timer::~Timer() {
    killTimer();
}
