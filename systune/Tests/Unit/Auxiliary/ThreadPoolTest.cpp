/*! \file */
#include <gtest/gtest.h>
#include "ThreadPool.h"

std::mutex taskLock;
std::condition_variable taskCV;

int32_t sharedVariable = 0;
std::string sharedString = "";
int8_t taskCondition = false;

#if !defined(UNIT_TESTS_BUILD)
// Define a test within a test group
#define TEST(testGroup, testName, brief, details)                        \
class TEST_##testGroup## {                                               \
public:                                                                  \
    /** ##brief##\n                                                      \
       ##details## */                                                    \
    ##testName##(##testGroup##, ##testName##) {}                         \
};
#endif   

void threadPoolTask(void* arg) {
	assert(arg != nullptr);
	*(int32_t*)arg = 64;
	return;
}

/*! \test */
void threadPoolLongDurationTask(void* arg) {
	sleep(*(int32_t*)arg);
}

#if !defined(UNIT_TESTS_BUILD)
/*! \test */
TEST(ThreadPoolTaskPickupTests, TestThreadPoolTaskPickup1,
"Objective: Enqueue a task to the ThreadPool and verify that the task is correctly picked up and processed.",
"Here we enqueue the task "threadPoolTask" to the ThreadPool with an IN/OUT arg. As part of this routine we modify the value of this argument. Ensure that the value of the argument was correctly updated as part of the threadPoolTask routine.") 
#else
TEST(ThreadPoolTaskPickupTests, TestThreadPoolTaskPickup1)
#endif 
{
	ThreadPool* threadPool = new ThreadPool(1, 1);
    sleep(1);

	int32_t arg = 49;

	threadPool->enqueueTask(threadPoolTask, (void*)&arg);
	sleep(1);
	
	ASSERT_EQ(arg, 64);

    delete threadPool;
}

#if !defined(UNIT_TESTS_BUILD)
TEST(ThreadPoolTaskPickupTests, TestThreadPoolEnqueueStatus1,
"Objective: Enqueue a task to the ThreadPool and verify that the task is correctly picked up and processed.",
"Here we enqueue the task "threadPoolTask" to the ThreadPool with an IN/OUT arg. As part of this routine we modify the value of this argument. Ensure that the task was accepted by the ThreadPool, i.e. the call to enqueueTask returns a value of 1. Ensure that the value of the argument was correctly updated as part of the threadPoolTask routine.") 
#else
TEST(ThreadPoolTaskPickupTests, TestThreadPoolEnqueueStatus1)
#endif 
{
	ThreadPool* threadPool = new ThreadPool(2, 1);
	sleep(1);
	
	int32_t arg = 14;
	
	int8_t ret = threadPool->enqueueTask(threadPoolTask, (void*)&arg);
	sleep(1);

	ASSERT_EQ(ret, 1);
    ASSERT_EQ(arg, 64);

    delete threadPool;
}

#if !defined(UNIT_TESTS_BUILD)
TEST(ThreadPoolTaskPickupTests, TestThreadPoolEnqueueStatus2,
"Objective: Verify that the ThreadPool accepts new tasks upto it's max capacity, beyond which no further tasks should be accepted.",
"Here we enqueue the task "threadPoolTask" to the ThreadPool. We specify a desired capacity of 1, and a max pending queue size of 1. So only a maximum of 2 concurrent tasks can be accepted at a time. Here we try to enqueue 3 tasks to the pool. Ensure that the first two tasks are accepted (i.e. enqueueTask returns true), while the third task should not be enqueued (i.e. enqueuTask returns false)") 
#else
TEST(ThreadPoolTaskPickupTests, TestThreadPoolEnqueueStatus2)
#endif 
{
	ThreadPool* threadPool = new ThreadPool(1, 1);
	sleep(1);
	
	int32_t taskLength = 2;

	int8_t ret1 = (threadPool->enqueueTask(threadPoolTask, (void*)&taskLength));
	int8_t ret2 = (threadPool->enqueueTask(threadPoolTask, (void*)&taskLength));
	int8_t ret3 = (threadPool->enqueueTask(threadPoolTask, (void*)&taskLength));

    ASSERT_EQ(ret1, true);
    ASSERT_EQ(ret2, true);
    ASSERT_EQ(ret3, false);

    delete threadPool;
}

#if !defined(UNIT_TESTS_BUILD)
TEST(ThreadPoolTaskPickupTests, TestThreadPoolEnqueueStatus3,
"Objective: Verify that the ThreadPool accepts new tasks upto it's max capacity.",
"Here we enqueue the task "threadPoolTask" to the ThreadPool. We specify a desired capacity of 2, and a max pending queue size of 1. So only a maximum of 3 concurrent tasks can be accepted at a time. Here we try to enqueue 3 tasks to the pool. Ensure that all the 3 tasks are accepted (i.e. enqueueTask returns true)") 
#else
TEST(ThreadPoolTaskPickupTests, TestThreadPoolEnqueueStatus3)
#endif 
{
	ThreadPool* threadPool = new ThreadPool(2, 1);
	sleep(1);
	
	int32_t taskLength = 2;

	int8_t ret1 = (threadPool->enqueueTask(threadPoolLongDurationTask, (void*)&taskLength));
	int8_t ret2 = (threadPool->enqueueTask(threadPoolLongDurationTask, (void*)&taskLength));
	int8_t ret3 = (threadPool->enqueueTask(threadPoolLongDurationTask, (void*)&taskLength));

    ASSERT_EQ(ret1, true);
    ASSERT_EQ(ret2, true);
    ASSERT_EQ(ret3, true);

    delete threadPool;
}

#if !defined(UNIT_TESTS_BUILD)
TEST(ThreadPoolTaskPickupTests, TestThreadPoolEnqueueStatus4,
"Objective: Verify that the ThreadPool accepts new tasks upto it's max capacity, beyond which no further tasks should be accepted.",
"Here we enqueue the task "threadPoolLongDurationTask" to the ThreadPool. We specify a desired capacity of 2, and a max pending queue size of 1. So only a maximum of 3 concurrent tasks can be accepted at a time. Here we try to enqueue 4 tasks to the pool. Ensure that the first three tasks are accepted (i.e. enqueueTask returns true), while the fourth task should not be enqueued (i.e. enqueuTask returns false)") 
#else
TEST(ThreadPoolTaskPickupTests, TestThreadPoolEnqueueStatus4)
#endif 
{
	ThreadPool* threadPool = new ThreadPool(2, 1);
	sleep(1);
	
	int32_t taskLength = 2;

	int8_t ret1 = (threadPool->enqueueTask(threadPoolLongDurationTask, (void*)&taskLength));
	int8_t ret2 = (threadPool->enqueueTask(threadPoolLongDurationTask, (void*)&taskLength));
	int8_t ret3 = (threadPool->enqueueTask(threadPoolLongDurationTask, (void*)&taskLength));
    sleep(1);
    int8_t ret4 = (threadPool->enqueueTask(threadPoolLongDurationTask, (void*)&taskLength));

    ASSERT_EQ(ret1, true);
    ASSERT_EQ(ret2, true);
    ASSERT_EQ(ret3, true);
    ASSERT_EQ(ret4, false);

    delete threadPool;
}

#if !defined(UNIT_TESTS_BUILD)
TEST(ThreadPoolTaskPickupTests, TestThreadPoolEnqueueStatus5,
"Objective: Verify that the ThreadPool accepts new tasks upto it's max capacity, beyond which no further tasks should be accepted.",
"Here we enqueue the task "threadPoolLongDurationTask" to the ThreadPool. We specify a desired capacity of 2, and a max pending queue size of 0. So only a maximum of 2 concurrent tasks can be accepted at a time. Here we try to enqueue 4 tasks to the pool. Ensure that the first two tasks are accepted (i.e. enqueueTask returns true), while the third task should not be enqueued (i.e. enqueuTask returns false) Wait for a span of 1 second, by this point the earlier enqueued tasks would have been completely processed. Now try to add another task to the pool, the task should be successfully enqueued.") 
#else
TEST(ThreadPoolTaskPickupTests, TestThreadPoolEnqueueStatus5)
#endif 
{
    ThreadPool* threadPool = new ThreadPool(2, 0);
	sleep(1);
	
	int32_t taskLength = 1;

	int8_t ret1 = (threadPool->enqueueTask(threadPoolLongDurationTask, (void*)&taskLength));
	int8_t ret2 = (threadPool->enqueueTask(threadPoolLongDurationTask, (void*)&taskLength));
	int8_t ret3 = (threadPool->enqueueTask(threadPoolLongDurationTask, (void*)&taskLength));
    sleep(2);
    int8_t ret4 = (threadPool->enqueueTask(threadPoolLongDurationTask, (void*)&taskLength));

    ASSERT_EQ(ret1, true);
    ASSERT_EQ(ret2, true);
    ASSERT_EQ(ret3, false);
    ASSERT_EQ(ret4, true);

    delete threadPool;
}

void helperFunction(void* arg) {
    for(int32_t i = 0; i < 1e7; i++) {
        taskLock.lock();
        sharedVariable++;
        taskLock.unlock();
    }
}

#if !defined(UNIT_TESTS_BUILD)
/*! \test */
TEST(ThreadPoolTaskProcessingTests, TestThreadPoolTaskProcessing1,
"Objective: Verify that the tasks enqueued are correctly processed, even if these tasks internally use synchronization primitives.",
"Here we create a thread pool with desired capacity of 2 and enqueue 2 thread-tasks that access a shared variable (sharedVariable). Ensure that the variable "sharedVariable" is correctly updated and there are no race conditions.") 
#else
TEST(ThreadPoolTaskProcessingTests, TestThreadPoolTaskProcessing1)
#endif 
{
    ThreadPool* threadPool = new ThreadPool(2, 0);
	sleep(1);

    int8_t ret1 = threadPool->enqueueTask(helperFunction, nullptr);
	int8_t ret2 = threadPool->enqueueTask(helperFunction, nullptr);

    ASSERT_EQ(ret1, true);
    ASSERT_EQ(ret2, true);

    // Wait for both tasks to complete
    sleep(3);

    ASSERT_EQ(sharedVariable, 2e7);

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

#if !defined(UNIT_TESTS_BUILD)
TEST(ThreadPoolTaskProcessingTests, TestThreadPoolTaskProcessing2,
"Objective: Verify that the tasks enqueued are correctly processed, even if these tasks internally use synchronization primitives.",
"Here we create a thread pool with desired capacity of 2 and enqueue 2 thread-tasks that access a shared variable (sharedString) which must be updated in a specific order, here thread B must wait for thread A to finish execution. Ensure that the variable "sharedString" is updated correctly using condition variable.") 
#else
TEST(ThreadPoolTaskProcessingTests, TestThreadPoolTaskProcessing2)
#endif 
{
    ThreadPool* threadPool = new ThreadPool(2, 0);
	sleep(1);

    int8_t ret1 = threadPool->enqueueTask(taskAFunc, nullptr);
	int8_t ret2 = threadPool->enqueueTask(taskBFunc, nullptr);

    ASSERT_EQ(ret1, true);
    ASSERT_EQ(ret2, true);

    // Wait for both tasks to complete
    sleep(1);

    ASSERT_EQ(sharedString, "AB");

    delete threadPool;
}
