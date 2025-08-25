// Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
// SPDX-License-Identifier: BSD-3-Clause-Clear

#include <cmath>

#include "Timer.h"

#define RUN_TEST(test)                                              \
do {                                                                \
    std::cout<<"Running Test: "<<#test<<std::endl;                  \
    test();                                                         \
    std::cout<<#test<<": Run Successful"<<std::endl;                \
    std::cout<<"-------------------------------------"<<std::endl;  \
} while(false);                                                     \

#define C_ASSERT(cond)                                                               \
    if(cond == false) {                                                              \
        std::cerr<<"Condition Check on line:["<<__LINE__<<"]  failed"<<std::endl;    \
        std::cerr<<"Test: ["<<__func__<<"] Failed, Terminating Suite\n"<<std::endl;  \
        exit(EXIT_FAILURE);                                                          \
    }                                                                                \

#define C_ASSERT_NEAR(val1, val2, tol)                                               \
    if (std::fabs((val1) - (val2)) > (tol)) {                                        \
        std::cerr<<"Condition Check on line:["<<__LINE__<<"]  failed"<<std::endl;    \
        std::cerr<<"Test: ["<<__func__<<"] Failed, Terminating Suite\n"<<std::endl;  \
        exit(EXIT_FAILURE);                                                          \
    }                                                                                \

static std::shared_ptr<ThreadPool> tpoolInstance = std::shared_ptr<ThreadPool> (new ThreadPool(4, 4, 5));

static Timer* timer;
static Timer* recurringTimer;
static std::atomic<int8_t> isFinished;

static void afterTimer(void*) {
    isFinished.store(true);
}

static void Init() {
    Timer::mTimerThreadPool = tpoolInstance.get();
    MakeAlloc<Timer>(10);

    isFinished.store(false);
    timer = new Timer(afterTimer);
    recurringTimer = new Timer(afterTimer, true);
}

static void simulateWork() {
    while(!isFinished.load()){
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
}

static void BaseCase() {
    C_ASSERT(timer != nullptr);
    auto start = std::chrono::high_resolution_clock::now();
    timer->startTimer(200);
    simulateWork();
    auto finish = std::chrono::high_resolution_clock::now();
    auto dur = std::chrono::duration_cast<std::chrono::milliseconds>(finish - start).count();

    C_ASSERT_NEAR(dur, 200, 25); //some tolerance
}

static void killBeforeCompletion() {
    C_ASSERT(timer != nullptr);
    auto start = std::chrono::high_resolution_clock::now();
    timer->startTimer(200);
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    timer->killTimer();
    auto finish = std::chrono::high_resolution_clock::now();
    auto dur = std::chrono::duration_cast<std::chrono::milliseconds>(finish - start).count();

    C_ASSERT_NEAR(dur, 100, 25); //some tolerance
}

static void killAfterCompletion() {
    C_ASSERT(timer != nullptr);
    auto start = std::chrono::high_resolution_clock::now();
    timer->startTimer(200);
    std::this_thread::sleep_for(std::chrono::milliseconds(300));
    timer->killTimer();
    auto finish = std::chrono::high_resolution_clock::now();
    auto dur = std::chrono::duration_cast<std::chrono::milliseconds>(finish - start).count();

    C_ASSERT_NEAR(dur, 300, 25); //some tolerance
}

static void RecurringTimer() {
    C_ASSERT(recurringTimer != nullptr);

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

    C_ASSERT_NEAR(dur, 600, 25); //some tolerance
}

static void RecurringTimerPreMatureKill() {
    C_ASSERT(recurringTimer != nullptr);

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

    C_ASSERT_NEAR(dur, 500, 25); //some tolerance
}

int main() {
    std::cout<<"Running Test Suite: [TimerTest]\n"<<std::endl;

    RUN_TEST(BaseCase);
    RUN_TEST(killBeforeCompletion);
    RUN_TEST(killAfterCompletion);
    RUN_TEST(RecurringTimer);
    RUN_TEST(RecurringTimerPreMatureKill);

    std::cout<<"\nAll Tests from the suite: [TimerTest], executed successfully"<<std::endl;
    return 0;
}
