#include <gtest/gtest.h>
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

TEST(ThreadPoolTaskPickupTests, TestThreadPoolTaskPickup1) {
	ThreadPool* threadPool = new ThreadPool(1, 1, 1);
	std::this_thread::sleep_for(std::chrono::seconds(1));

	int32_t* ptr = (int32_t*) malloc(sizeof(int32_t));
	*ptr = 49;

	int8_t status = threadPool->enqueueTask(threadPoolTask, (void*)ptr);
	ASSERT_EQ(status, true);
	std::this_thread::sleep_for(std::chrono::seconds(1));

	ASSERT_EQ(*ptr, 64);

	delete ptr;
	delete threadPool;
	threadPool = nullptr;
}

TEST(ThreadPoolTaskPickupTests, TestThreadPoolEnqueueStatus1) {
	ThreadPool* threadPool = new ThreadPool(2, 1, 2);
	std::this_thread::sleep_for(std::chrono::seconds(1));

	int32_t* ptr = (int32_t*) malloc(sizeof(int32_t));
	*ptr = 14;

	int8_t ret = threadPool->enqueueTask(threadPoolTask, (void*)ptr);
	ASSERT_EQ(ret, true);
	std::this_thread::sleep_for(std::chrono::seconds(1));

	ASSERT_EQ(*ptr, 64);

	delete ptr;
	delete threadPool;
	threadPool = nullptr;
}

TEST(ThreadPoolTaskPickupTests, TestThreadPoolEnqueueStatus2) {
	ThreadPool* threadPool = new ThreadPool(2, 1, 2);
	std::this_thread::sleep_for(std::chrono::seconds(1));

	int32_t* ptr = (int32_t*) malloc(sizeof(int32_t));
	*ptr = 2;

	int8_t ret1 = (threadPool->enqueueTask(threadPoolLongDurationTask, (void*)ptr));
	int8_t ret2 = (threadPool->enqueueTask(threadPoolLongDurationTask, (void*)ptr));
	int8_t ret3 = (threadPool->enqueueTask(threadPoolLongDurationTask, (void*)ptr));

	ASSERT_EQ(ret1, true);
	ASSERT_EQ(ret2, true);
	ASSERT_EQ(ret3, true);

	std::this_thread::sleep_for(std::chrono::seconds(8));
	delete ptr;
	delete threadPool;
}

TEST(ThreadPoolTaskPickupTests, TestThreadPoolEnqueueStatus3) {
	ThreadPool* threadPool = new ThreadPool(2, 1, 2);
	std::this_thread::sleep_for(std::chrono::seconds(1));

	int32_t* ptr = (int32_t*) malloc(sizeof(int32_t));
	*ptr = 3;

	int8_t ret1 = (threadPool->enqueueTask(threadPoolLongDurationTask, (void*)ptr));
	int8_t ret2 = (threadPool->enqueueTask(threadPoolLongDurationTask, (void*)ptr));
	int8_t ret3 = (threadPool->enqueueTask(threadPoolLongDurationTask, (void*)ptr));
	std::this_thread::sleep_for(std::chrono::seconds(1));
	int8_t ret4 = (threadPool->enqueueTask(threadPoolLongDurationTask, (void*)ptr));

	ASSERT_EQ(ret1, true);
	ASSERT_EQ(ret2, true);
	ASSERT_EQ(ret3, true);
	ASSERT_EQ(ret4, false);

	delete ptr;
	delete threadPool;
}

TEST(ThreadPoolTaskPickupTests, TestThreadPoolEnqueueStatus4) {
	ThreadPool* threadPool = new ThreadPool(2, 0, 2);
	std::this_thread::sleep_for(std::chrono::seconds(1));

	int32_t* ptr = (int32_t*) malloc(sizeof(int32_t));
	*ptr = 1;

	int8_t ret1 = (threadPool->enqueueTask(threadPoolLongDurationTask, (void*)ptr));
	int8_t ret2 = (threadPool->enqueueTask(threadPoolLongDurationTask, (void*)ptr));
	int8_t ret3 = (threadPool->enqueueTask(threadPoolLongDurationTask, (void*)ptr));
	std::this_thread::sleep_for(std::chrono::seconds(3));
	int8_t ret4 = (threadPool->enqueueTask(threadPoolLongDurationTask, (void*)ptr));

	ASSERT_EQ(ret1, true);
	ASSERT_EQ(ret2, true);
	ASSERT_EQ(ret3, false);
	ASSERT_EQ(ret4, true);

	delete ptr;
	delete threadPool;
}

void helperFunction(void* arg) {
    for(int32_t i = 0; i < 1e7; i++) {
        taskLock.lock();
        sharedVariable++;
        taskLock.unlock();
    }
}

TEST(ThreadPoolTaskProcessingTests, TestThreadPoolTaskProcessing1) {
	ThreadPool* threadPool = new ThreadPool(2, 0, 2);
	std::this_thread::sleep_for(std::chrono::seconds(1));

	int8_t ret1 = threadPool->enqueueTask(helperFunction, nullptr);
	int8_t ret2 = threadPool->enqueueTask(helperFunction, nullptr);

	ASSERT_EQ(ret1, true);
	ASSERT_EQ(ret2, true);

	// Wait for both tasks to complete
	std::this_thread::sleep_for(std::chrono::seconds(3));

	ASSERT_EQ(sharedVariable, 2e7);

	delete threadPool;
}

// Lambda Function
TEST(ThreadPoolTaskProcessingTests, TestThreadPoolTaskProcessing2) {
	ThreadPool* threadPool = new ThreadPool(1, 0, 1);
	std::this_thread::sleep_for(std::chrono::seconds(1));
	sharedVariable = 0;

	int8_t ret = threadPool->enqueueTask([&](void* arg) {
		for(int32_t i = 0; i < 1e7; i++) {
			sharedVariable++;
		}
	}, nullptr);

	ASSERT_EQ(ret, true);

	// Wait for both tasks to complete
	std::this_thread::sleep_for(std::chrono::seconds(3));

	ASSERT_EQ(sharedVariable, 1e7);

	delete threadPool;
}

TEST(ThreadPoolTaskProcessingTests, TestThreadPoolTaskProcessing3) {
	ThreadPool* threadPool = new ThreadPool(1, 0, 1);
	std::this_thread::sleep_for(std::chrono::seconds(1));

	sharedVariable = 0;
	int32_t* callID = (int32_t*) malloc(sizeof(int32_t));
	*callID = 56;

	int8_t ret = threadPool->enqueueTask([&](void* arg) {
		ASSERT_NE(arg, nullptr);
		sharedVariable = *(int32_t*)arg;
	}, (void*)callID);

	ASSERT_EQ(ret, true);

	// Wait for both tasks to complete
	std::this_thread::sleep_for(std::chrono::seconds(3));

	ASSERT_EQ(sharedVariable, *callID);

	delete callID;
	delete threadPool;
}

void taskAFunc(void* arg) {
    std::unique_lock<std::mutex> uniqueLock(taskLock);
    sharedString.push_back('A');
    taskCondition = true;
    taskCV.notify_one();
}

void taskBFunc(void* arg) {
    std::unique_lock<std::mutex> uniqueLock(taskLock);
    while(!taskCondition) {
        taskCV.wait(uniqueLock);
    }
    sharedString.push_back('B');
}

TEST(ThreadPoolTaskProcessingTests, TestThreadPoolTaskProcessing4) {
	ThreadPool* threadPool = new ThreadPool(2, 0, 2);
	std::this_thread::sleep_for(std::chrono::seconds(1));

	int8_t ret1 = threadPool->enqueueTask(taskAFunc, nullptr);
	int8_t ret2 = threadPool->enqueueTask(taskBFunc, nullptr);

	ASSERT_EQ(ret1, true);
	ASSERT_EQ(ret2, true);

	// Wait for both tasks to complete
	std::this_thread::sleep_for(std::chrono::seconds(1));

	ASSERT_EQ(sharedString, "AB");

	delete threadPool;
}

// Tests For Thread Pool Scaling
TEST(ThreadPoolScalingTests, TestThreadPoolEnqueueStatusWithExpansion1) {
	ThreadPool* threadPool = new ThreadPool(2, 0, 3);
	std::this_thread::sleep_for(std::chrono::seconds(1));

	int32_t* ptr = (int32_t*) malloc(sizeof(int32_t));
	*ptr = 3;

	int8_t ret1 = (threadPool->enqueueTask(threadPoolLongDurationTask, (void*)ptr));
	int8_t ret2 = (threadPool->enqueueTask(threadPoolLongDurationTask, (void*)ptr));
	int8_t ret3 = (threadPool->enqueueTask(threadPoolLongDurationTask, (void*)ptr));

	ASSERT_EQ(ret1, true);
	ASSERT_EQ(ret2, true);
	ASSERT_EQ(ret3, true);

	std::this_thread::sleep_for(std::chrono::seconds(8));
	delete ptr;
	delete threadPool;
}

TEST(ThreadPoolScalingTests, TestThreadPoolEnqueueStatusWithExpansion2) {
	ThreadPool* threadPool = new ThreadPool(2, 0, 3);
	std::this_thread::sleep_for(std::chrono::seconds(1));

	int32_t* ptr = (int32_t*) malloc(sizeof(int32_t));
	*ptr = 3;

	int8_t ret1 = (threadPool->enqueueTask(threadPoolLongDurationTask, (void*)ptr));
	int8_t ret2 = (threadPool->enqueueTask(threadPoolLongDurationTask, (void*)ptr));
	int8_t ret3 = (threadPool->enqueueTask(threadPoolLongDurationTask, (void*)ptr));
	int8_t ret4 = (threadPool->enqueueTask(threadPoolLongDurationTask, (void*)ptr));
	std::this_thread::sleep_for(std::chrono::seconds(5));
	int8_t ret5 = (threadPool->enqueueTask(threadPoolLongDurationTask, (void*)ptr));

	ASSERT_EQ(ret1, true);
	ASSERT_EQ(ret2, true);
	ASSERT_EQ(ret3, true);
	ASSERT_EQ(ret4, false);
	ASSERT_EQ(ret5, true);

	std::this_thread::sleep_for(std::chrono::seconds(8));
	delete ptr;
	delete threadPool;
}

TEST(ThreadPoolScalingTests, TestThreadPoolEnqueueStatusWithExpansion3) {
	ThreadPool* threadPool = new ThreadPool(2, 0, 4);
	std::this_thread::sleep_for(std::chrono::seconds(1));

	int32_t* ptr = (int32_t*) malloc(sizeof(int32_t));
	*ptr = 3;

	int8_t ret1 = (threadPool->enqueueTask(threadPoolLongDurationTask, (void*)ptr));
	int8_t ret2 = (threadPool->enqueueTask(threadPoolLongDurationTask, (void*)ptr));
	int8_t ret3 = (threadPool->enqueueTask(threadPoolLongDurationTask, (void*)ptr));
	int8_t ret4 = (threadPool->enqueueTask(threadPoolLongDurationTask, (void*)ptr));

	ASSERT_EQ(ret1, true);
	ASSERT_EQ(ret2, true);
	ASSERT_EQ(ret3, true);
	ASSERT_EQ(ret4, true);

	std::this_thread::sleep_for(std::chrono::seconds(8));
	delete ptr;
	delete threadPool;
}
