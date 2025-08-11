// Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
// SPDX-License-Identifier: BSD-3-Clause-Clear

#include "Timer.h"
#include "Logger.h"

ThreadPool* Timer::mTimerThreadPool = nullptr;

Timer::Timer(std::function<void(void*)>callBack, int8_t isRecurring) {
    this->mStop.store(false);
    this->mIsRecurring = isRecurring;
    this->mCallBack = callBack;
}

void Timer::implementTimer() {
    try {
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
    } catch(const std::exception& e) {
        LOGE("RTN_TIMER", "Timer Could not be started, Error: " + std::string(e.what()));
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

    LOGD("RTN_TIMER", "Timer started with " + std::to_string(duration));
    return true;
}

void Timer::killTimer() {
    LOGD("RTN_TIMER", "Killing timer");
    mStop.store(true);
    mCv.notify_all();
}

Timer::~Timer() {
    killTimer();
}
