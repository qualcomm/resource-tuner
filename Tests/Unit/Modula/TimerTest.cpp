// Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
// SPDX-License-Identifier: BSD-3-Clause-Clear

#include <gtest/gtest.h>
#include "Timer.h"

static std::shared_ptr<ThreadPool> tpoolInstance = std::shared_ptr<ThreadPool> (new ThreadPool(4, 4, 5));

class TimerTest : public testing::Test {
protected:
    Timer* timer;
    Timer* recurringTimer;
    std::atomic<int8_t> isFinished;

    void SetUp() override {
        static int8_t firstTest = true;
        if(firstTest) {
            Timer::mTimerThreadPool = tpoolInstance.get();
            MakeAlloc<Timer>(10);
            firstTest = false;
        }

        isFinished.store(false);
        timer = new Timer(std::bind(&TimerTest::afterTimer, this));
        recurringTimer = new Timer(std::bind(&TimerTest::afterTimer, this), true);
    }

    int32_t afterTimer() {
        isFinished.store(true);
        return 0;
    }

    void simulateWork() {
        while(!isFinished.load()){
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
    }

public:
    ~TimerTest() {}
};

TEST_F(TimerTest, BaseCase) {
    ASSERT_NE(timer, nullptr);
    auto start = std::chrono::high_resolution_clock::now();
    timer->startTimer(200);
    simulateWork();
    auto finish = std::chrono::high_resolution_clock::now();
    auto dur = std::chrono::duration_cast<std::chrono::milliseconds>(finish - start).count();

    EXPECT_NEAR(dur, 200, 25); //some tolerance
}

TEST_F(TimerTest, killBeforeCompletion) {
    ASSERT_NE(timer, nullptr);
    auto start = std::chrono::high_resolution_clock::now();
    timer->startTimer(200);
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    timer->killTimer();
    auto finish = std::chrono::high_resolution_clock::now();
    auto dur = std::chrono::duration_cast<std::chrono::milliseconds>(finish - start).count();

    EXPECT_NEAR(dur, 100, 25); //some tolerance
}

TEST_F(TimerTest, killAfterCompletion) {
    ASSERT_NE(timer, nullptr);
    auto start = std::chrono::high_resolution_clock::now();
    timer->startTimer(200);
    std::this_thread::sleep_for(std::chrono::milliseconds(300));
    timer->killTimer();
    auto finish = std::chrono::high_resolution_clock::now();
    auto dur = std::chrono::duration_cast<std::chrono::milliseconds>(finish - start).count();

    EXPECT_NEAR(dur, 300, 25); //some tolerance
}

TEST_F(TimerTest, RecurringTimer) {
    ASSERT_NE(recurringTimer, nullptr);

    auto start = std::chrono::high_resolution_clock::now();
    recurringTimer->startTimer(200);
    simulateWork();
    isFinished.store(false);
    simulateWork();
    isFinished.store(false);
    simulateWork();
    recurringTimer->killTimer();
    auto finish = std::chrono::high_resolution_clock::now();
    auto dur = std::chrono::duration_cast<std::chrono::milliseconds>(finish - start).count();

    EXPECT_NEAR(dur, 600, 25); //some tolerance
}

TEST_F(TimerTest, RecurringTimerPreMatureKill) {
    ASSERT_NE(recurringTimer, nullptr);

    auto start = std::chrono::high_resolution_clock::now();
    recurringTimer->startTimer(200);
    simulateWork();
    isFinished.store(false);
    simulateWork();
    isFinished.store(false);
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    recurringTimer->killTimer();
    auto finish = std::chrono::high_resolution_clock::now();
    auto dur = std::chrono::duration_cast<std::chrono::milliseconds>(finish - start).count();

    EXPECT_NEAR(dur, 500, 25); //some tolerance
}
