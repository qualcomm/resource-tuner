// Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
// SPDX-License-Identifier: BSD-3-Clause-Clear

#include <iostream>

#include "TestUtils.h"
#include "ThreadPool.h"

std::mutex taskLock;
std::condition_variable taskCV;

int32_t sharedVariable = 0;
std::string sharedString = "";
int8_t taskCondition = false;

void threadPoolTask(void* arg) {
	assert(arg != nullptr);
	*(int32_t*)arg = 64;
	return;
}

void threadPoolLongDurationTask(void* arg) {
	std::this_thread::sleep_for(std::chrono::seconds(*(int32_t*)arg));
}

static void TestThreadPoolTaskPickup1() {
	ThreadPool* threadPool = new ThreadPool(1, 1, 1);
	std::this_thread::sleep_for(std::chrono::seconds(1));

	int32_t* ptr = (int32_t*) malloc(sizeof(int32_t));
	*ptr = 49;

	int8_t status = threadPool->enqueueTask(threadPoolTask, (void*)ptr);
	C_ASSERT(status == true);
	std::this_thread::sleep_for(std::chrono::seconds(1));

	C_ASSERT(*ptr == 64);

	delete ptr;
	delete threadPool;
	threadPool = nullptr;
}

static void TestThreadPoolEnqueueStatus1() {
	ThreadPool* threadPool = new ThreadPool(2, 1, 2);
	std::this_thread::sleep_for(std::chrono::seconds(1));

	int32_t* ptr = (int32_t*) malloc(sizeof(int32_t));
	*ptr = 14;

	int8_t ret = threadPool->enqueueTask(threadPoolTask, (void*)ptr);
	C_ASSERT(ret == true);
	std::this_thread::sleep_for(std::chrono::seconds(1));

	C_ASSERT(*ptr == 64);

	delete ptr;
	delete threadPool;
	threadPool = nullptr;
}

static void TestThreadPoolEnqueueStatus2_1() {
	ThreadPool* threadPool = new ThreadPool(1, 1, 1);
	std::this_thread::sleep_for(std::chrono::seconds(1));

	int32_t* ptr = (int32_t*) malloc(sizeof(int32_t));
	*ptr = 2;

	int8_t ret1 = (threadPool->enqueueTask(threadPoolLongDurationTask, (void*)ptr));
	int8_t ret2 = (threadPool->enqueueTask(threadPoolLongDurationTask, (void*)ptr));

	C_ASSERT(ret1 == true);
	C_ASSERT(ret2 == true);

	std::this_thread::sleep_for(std::chrono::seconds(8));
	delete ptr;
	delete threadPool;
}

static void TestThreadPoolEnqueueStatus2_2() {
	ThreadPool* threadPool = new ThreadPool(2, 1, 2);
	std::this_thread::sleep_for(std::chrono::seconds(1));

	int32_t* ptr = (int32_t*) malloc(sizeof(int32_t));
	*ptr = 2;

	int8_t ret1 = (threadPool->enqueueTask(threadPoolLongDurationTask, (void*)ptr));
	int8_t ret2 = (threadPool->enqueueTask(threadPoolLongDurationTask, (void*)ptr));
	int8_t ret3 = (threadPool->enqueueTask(threadPoolLongDurationTask, (void*)ptr));

	C_ASSERT(ret1 == true);
	C_ASSERT(ret2 == true);
	C_ASSERT(ret3 == true);

	std::this_thread::sleep_for(std::chrono::seconds(8));
	delete ptr;
	delete threadPool;
}

// static void TestThreadPoolEnqueueStatus4() {
// 	ThreadPool* threadPool = new ThreadPool(2, 0, 2);
// 	std::this_thread::sleep_for(std::chrono::seconds(1));

// 	int32_t* ptr = (int32_t*) malloc(sizeof(int32_t));
// 	*ptr = 1;

// 	int8_t ret1 = (threadPool->enqueueTask(threadPoolLongDurationTask, (void*)ptr));
// 	int8_t ret2 = (threadPool->enqueueTask(threadPoolLongDurationTask, (void*)ptr));
// 	int8_t ret3 = (threadPool->enqueueTask(threadPoolLongDurationTask, (void*)ptr));
// 	std::this_thread::sleep_for(std::chrono::seconds(3));
// 	int8_t ret4 = (threadPool->enqueueTask(threadPoolLongDurationTask, (void*)ptr));

// 	C_ASSERT(ret1 == true);
// 	C_ASSERT(ret2 == true);
// 	C_ASSERT(ret3 == false);
// 	C_ASSERT(ret4 == true);

// 	delete ptr;
// 	delete threadPool;
// }

// void helperFunction(void* arg) {
//     for(int32_t i = 0; i < 1e7; i++) {
//         taskLock.lock();
//         sharedVariable++;
//         taskLock.unlock();
//     }
// }

// static void TestThreadPoolTaskProcessing1() {
// 	ThreadPool* threadPool = new ThreadPool(2, 0, 2);
// 	std::this_thread::sleep_for(std::chrono::seconds(1));

// 	int8_t ret1 = threadPool->enqueueTask(helperFunction, nullptr);
// 	int8_t ret2 = threadPool->enqueueTask(helperFunction, nullptr);

// 	C_ASSERT(ret1 == true);
// 	C_ASSERT(ret2 == true);

// 	// Wait for both tasks to complete
// 	std::this_thread::sleep_for(std::chrono::seconds(3));

// 	C_ASSERT(sharedVariable == 2e7);

// 	delete threadPool;
// }

// // Lambda Function
// static void TestThreadPoolTaskProcessing2() {
// 	ThreadPool* threadPool = new ThreadPool(1, 0, 1);
// 	std::this_thread::sleep_for(std::chrono::seconds(1));
// 	sharedVariable = 0;

// 	int8_t ret = threadPool->enqueueTask([&](void* arg) {
// 		for(int32_t i = 0; i < 1e7; i++) {
// 			sharedVariable++;
// 		}
// 	}, nullptr);

// 	C_ASSERT(ret == true);

// 	// Wait for both tasks to complete
// 	std::this_thread::sleep_for(std::chrono::seconds(3));

// 	C_ASSERT(sharedVariable == 1e7);

// 	delete threadPool;
// }

// static void TestThreadPoolTaskProcessing3() {
// 	ThreadPool* threadPool = new ThreadPool(1, 0, 1);
// 	std::this_thread::sleep_for(std::chrono::seconds(1));

// 	sharedVariable = 0;
// 	int32_t* callID = (int32_t*) malloc(sizeof(int32_t));
// 	*callID = 56;

// 	int8_t ret = threadPool->enqueueTask([&](void* arg) {
// 		C_ASSERT(arg != nullptr);
// 		sharedVariable = *(int32_t*)arg;
// 	}, (void*)callID);

// 	C_ASSERT(ret == true);

// 	// Wait for both tasks to complete
// 	std::this_thread::sleep_for(std::chrono::seconds(3));

// 	C_ASSERT(sharedVariable == *callID);

// 	delete callID;
// 	delete threadPool;
// }

// void taskAFunc(void* arg) {
//     std::unique_lock<std::mutex> uniqueLock(taskLock);
//     sharedString.push_back('A');
//     taskCondition = true;
//     taskCV.notify_one();
// }

// void taskBFunc(void* arg) {
//     std::unique_lock<std::mutex> uniqueLock(taskLock);
//     while(!taskCondition) {
//         taskCV.wait(uniqueLock);
//     }
//     sharedString.push_back('B');
// }

// static void TestThreadPoolTaskProcessing4() {
// 	ThreadPool* threadPool = new ThreadPool(2, 0, 2);
// 	std::this_thread::sleep_for(std::chrono::seconds(1));

// 	int8_t ret1 = threadPool->enqueueTask(taskAFunc, nullptr);
// 	int8_t ret2 = threadPool->enqueueTask(taskBFunc, nullptr);

// 	C_ASSERT(ret1 == true);
// 	C_ASSERT(ret2 == true);

// 	// Wait for both tasks to complete
// 	std::this_thread::sleep_for(std::chrono::seconds(1));

// 	C_ASSERT(sharedString == "AB");

// 	delete threadPool;
// }

// // Tests For Thread Pool Scaling
// static void TestThreadPoolEnqueueStatusWithExpansion1() {
// 	ThreadPool* threadPool = new ThreadPool(2, 0, 3);
// 	std::this_thread::sleep_for(std::chrono::seconds(1));

// 	int32_t* ptr = (int32_t*) malloc(sizeof(int32_t));
// 	*ptr = 3;

// 	int8_t ret1 = (threadPool->enqueueTask(threadPoolLongDurationTask, (void*)ptr));
// 	int8_t ret2 = (threadPool->enqueueTask(threadPoolLongDurationTask, (void*)ptr));
// 	int8_t ret3 = (threadPool->enqueueTask(threadPoolLongDurationTask, (void*)ptr));

// 	C_ASSERT(ret1 == true);
// 	C_ASSERT(ret2 == true);
// 	C_ASSERT(ret3 == true);

// 	std::this_thread::sleep_for(std::chrono::seconds(8));
// 	delete ptr;
// 	delete threadPool;
// }

// static void TestThreadPoolEnqueueStatusWithExpansion2() {
// 	ThreadPool* threadPool = new ThreadPool(2, 0, 3);
// 	std::this_thread::sleep_for(std::chrono::seconds(1));

// 	int32_t* ptr = (int32_t*) malloc(sizeof(int32_t));
// 	*ptr = 3;

// 	int8_t ret1 = (threadPool->enqueueTask(threadPoolLongDurationTask, (void*)ptr));
// 	int8_t ret2 = (threadPool->enqueueTask(threadPoolLongDurationTask, (void*)ptr));
// 	int8_t ret3 = (threadPool->enqueueTask(threadPoolLongDurationTask, (void*)ptr));
// 	int8_t ret4 = (threadPool->enqueueTask(threadPoolLongDurationTask, (void*)ptr));
// 	std::this_thread::sleep_for(std::chrono::seconds(5));
// 	int8_t ret5 = (threadPool->enqueueTask(threadPoolLongDurationTask, (void*)ptr));

// 	C_ASSERT(ret1 == true);
// 	C_ASSERT(ret2 == true);
// 	C_ASSERT(ret3 == true);
// 	C_ASSERT(ret4 == false);
// 	C_ASSERT(ret5 == true);

// 	std::this_thread::sleep_for(std::chrono::seconds(8));
// 	delete ptr;
// 	delete threadPool;
// }

// static void TestThreadPoolEnqueueStatusWithExpansion3() {
// 	ThreadPool* threadPool = new ThreadPool(2, 0, 4);
// 	std::this_thread::sleep_for(std::chrono::seconds(1));

// 	int32_t* ptr = (int32_t*) malloc(sizeof(int32_t));
// 	*ptr = 3;

// 	int8_t ret1 = (threadPool->enqueueTask(threadPoolLongDurationTask, (void*)ptr));
// 	int8_t ret2 = (threadPool->enqueueTask(threadPoolLongDurationTask, (void*)ptr));
// 	int8_t ret3 = (threadPool->enqueueTask(threadPoolLongDurationTask, (void*)ptr));
// 	int8_t ret4 = (threadPool->enqueueTask(threadPoolLongDurationTask, (void*)ptr));

// 	C_ASSERT(ret1 == true);
// 	C_ASSERT(ret2 == true);
// 	C_ASSERT(ret3 == true);
// 	C_ASSERT(ret4 == true);

// 	std::this_thread::sleep_for(std::chrono::seconds(8));
// 	delete ptr;
// 	delete threadPool;
// }


int32_t main() {
	std::cout<<"Running Test Suite: [ThreadPool Tests]\n"<<std::endl;

    RUN_TEST(TestThreadPoolTaskPickup1);
    RUN_TEST(TestThreadPoolEnqueueStatus1);
    RUN_TEST(TestThreadPoolEnqueueStatus2_1);
    RUN_TEST(TestThreadPoolEnqueueStatus2_2);
    // RUN_TEST(TestThreadPoolEnqueueStatus4);
    // RUN_TEST(TestThreadPoolTaskProcessing1);
    // RUN_TEST(TestThreadPoolTaskProcessing2);
    // RUN_TEST(TestThreadPoolTaskProcessing3);
    // RUN_TEST(TestThreadPoolTaskProcessing4);
    // RUN_TEST(TestThreadPoolEnqueueStatusWithExpansion1);
    // RUN_TEST(TestThreadPoolEnqueueStatusWithExpansion2);
    // RUN_TEST(TestThreadPoolEnqueueStatusWithExpansion3);

	std::cout<<"\nAll Tests from the suite: [ThreadPool Tests], executed successfully"<<std::endl;
    return 0;
}
