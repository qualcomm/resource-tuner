// Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
// SPDX-License-Identifier: BSD-3-Clause-Clear

#include "RequestQueue.h"
#include "TestUtils.h"

static void Init() {
    MakeAlloc<Message> (30);
}

static void TestRequestQueueTaskEnqueue() {
    std::shared_ptr<RequestQueue> queue = RequestQueue::getInstance();
    int32_t requestCount = 8;
    int32_t requestsProcessed = 0;

    for(int32_t count = 0; count < requestCount; count++) {
        Message* message = new (GetBlock<Message>()) Message;
        message->setDuration(9000);

        queue->addAndWakeup(message);
    }

    while(queue->hasPendingTasks()) {
        requestsProcessed++;
        Message* message = (Message*)queue->pop();

        FreeBlock<Message>(static_cast<void*>(message));
    }

    C_ASSERT(requestsProcessed == requestCount);
}

static void TestRequestQueueSingleTaskPickup1() {
    std::shared_ptr<RequestQueue> requestQueue = RequestQueue::getInstance();

    // Consumer
    std::thread consumerThread([&]{
        requestQueue->wait();

        while(requestQueue->hasPendingTasks()) {
            Request* req = (Request*)requestQueue->pop();

            C_ASSERT(req->getRequestType() == REQ_RESOURCE_TUNING);
            C_ASSERT(req->getHandle() == 200);
            C_ASSERT(req->getClientPID() == 321);
            C_ASSERT(req->getClientTID() == 2445);
            C_ASSERT(req->getDuration() == -1);

            delete req;
        }
    });

    // Producer
    // Enqueue a Request
    Request* req = new Request();
    req->setRequestType(REQ_RESOURCE_TUNING);
    req->setHandle(200);
    req->setDuration(-1);
    req->setNumResources(0);
    req->setClientPID(321);
    req->setClientTID(2445);
    req->setProperties(0);
    req->setResources(nullptr);
    requestQueue->addAndWakeup(req);

    consumerThread.join();
}

static void TestRequestQueueSingleTaskPickup2() {
    std::shared_ptr<RequestQueue> requestQueue = RequestQueue::getInstance();
    std::atomic<int32_t> requestsProcessed(0);
    int8_t taskCondition = false;
    std::mutex taskLock;
    std::condition_variable taskCV;

    // Consumer
    std::thread consumerThread([&]{
        std::unique_lock<std::mutex> uniqueLock(taskLock);
        int8_t terminateProducer = false;

        while(true) {
            if(terminateProducer) {
                return;
            }

            while(!taskCondition) {
                taskCV.wait(uniqueLock);
            }

            while(requestQueue->hasPendingTasks()) {
                Request* req = (Request*)requestQueue->pop();
                requestsProcessed.fetch_add(1);

                if(req->getHandle() == 0) {
                    delete req;
                    terminateProducer = true;
                    break;
                }

                delete req;
            }
        }
    });

    // producer
    std::thread producerThread([&]{
        const std::unique_lock<std::mutex> uniqueLock(taskLock);

        Request* req = new Request;
        req->setRequestType(REQ_RESOURCE_TUNING);
        req->setDuration(-1);
        req->setHandle(0);
        req->setProperties(0);
        req->setNumResources(0);
        req->setClientPID(321);
        req->setClientTID(2445);
        req->setResources(nullptr);
        requestQueue->addAndWakeup(req);

        taskCondition = true;
        taskCV.notify_one();
    });

    producerThread.join();
    consumerThread.join();

    C_ASSERT(requestsProcessed.load() == 1);
}

static void TestRequestQueueMultipleTaskPickup() {
    std::shared_ptr<RequestQueue> requestQueue = RequestQueue::getInstance();
    std::atomic<int32_t> requestsProcessed(0);
    int8_t taskCondition = false;
    std::mutex taskLock;
    std::condition_variable taskCV;

    // Consumer
    std::thread consumerThread([&]{
        std::unique_lock<std::mutex> uniqueLock(taskLock);
        int8_t terminateProducer = false;

        while(true) {
            if(terminateProducer) {
                return;
            }

            while(!taskCondition) {
                taskCV.wait(uniqueLock);
            }

            while(requestQueue->hasPendingTasks()) {
                Request* req = (Request*)requestQueue->pop();
                requestsProcessed.fetch_add(1);

                if(req->getHandle() == -1) {
                    delete req;
                    terminateProducer = true;
                    break;
                }

                delete req;
            }
        }
        return;
    });

    std::thread producerThread([&]{
        // Producer will enqueue multiple requests
        std::unique_lock<std::mutex> uniqueLock(taskLock);

        Request* req1 = new Request();
        req1->setRequestType(REQ_RESOURCE_TUNING);
        req1->setHandle(200);
        req1->setDuration(-1);
        req1->setNumResources(0);
        req1->setResources(nullptr);
        req1->setProperties(0);
        req1->setClientPID(321);
        req1->setClientTID(2445);

        Request* req2 = new Request();
        req2->setRequestType(REQ_RESOURCE_TUNING);
        req2->setHandle(112);
        req2->setDuration(-1);
        req2->setNumResources(0);
        req2->setResources(nullptr);
        req2->setProperties(0);
        req2->setClientPID(344);
        req2->setClientTID(2378);

        Request* req3 = new Request();
        req3->setRequestType(REQ_RESOURCE_TUNING);
        req3->setHandle(44);
        req3->setDuration(6500);
        req3->setNumResources(0);
        req3->setResources(nullptr);
        req3->setProperties(0);
        req3->setClientPID(32180);
        req3->setClientTID(67770);

        requestQueue->addAndWakeup(req1);
        requestQueue->addAndWakeup(req2);
        requestQueue->addAndWakeup(req3);

        // Instrumented request to force the taskThread to terminate
        Request* exitReq = new Request();
        exitReq->setRequestType(REQ_RESOURCE_TUNING);
        exitReq->setHandle(-1);
        exitReq->setDuration(-1);
        exitReq->setProperties(1);
        exitReq->setNumResources(0);
        exitReq->setClientPID(554);
        exitReq->setClientTID(3368);
        exitReq->setResources(nullptr);
        requestQueue->addAndWakeup(exitReq);

        taskCondition = true;
        taskCV.notify_one();
    });

    consumerThread.join();
    producerThread.join();

    C_ASSERT(requestsProcessed.load() == 4);
}

static void TestRequestQueueMultipleTaskAndProducersPickup() {
    std::shared_ptr<RequestQueue> requestQueue = RequestQueue::getInstance();
    std::atomic<int32_t> requestsProcessed(0);
    int32_t totalNumberOfThreads = 10;
    int8_t taskCondition = false;
    std::mutex taskLock;
    std::condition_variable taskCV;

    std::thread consumerThread([&]{
        std::unique_lock<std::mutex> uniqueLock(taskLock);
        int8_t terminateProducer = false;

        while(true) {
            if(terminateProducer) {
                return;
            }

            while(!taskCondition) {
                taskCV.wait(uniqueLock);
            }

            while(requestQueue->hasPendingTasks()) {
                Request* req = (Request*)requestQueue->pop();
                requestsProcessed.fetch_add(1);

                if(req->getHandle() == -1) {
                    delete req;
                    terminateProducer = true;
                    break;
                }

                delete req;
            }
        }
    });

    std::vector<std::thread> producerThreads;

    // Create multiple producer threads
    for(int32_t count = 0; count < totalNumberOfThreads; count++) {
        auto threadStartRoutine = [&]{
            Request* req = new Request();
            req->setRequestType(REQ_RESOURCE_TUNING);
            req->setHandle(count);
            req->setDuration(-1);
            req->setNumResources(0);
            req->setProperties(0);
            req->setClientPID(321 + count);
            req->setClientTID(2445 + count);
            req->setResources(nullptr);
            requestQueue->addAndWakeup(req);
        };
        producerThreads.push_back(std::thread(threadStartRoutine));
    }

    for(int32_t i = 0; i < producerThreads.size(); i++) {
        producerThreads[i].join();
    }

    sleep(1);
    // Instrumented request to force the consumerThread to terminate
    std::thread terminateThread([&]{
        std::unique_lock<std::mutex> uniqueLock(taskLock);

        Request* exitReq = new Request();
        exitReq->setRequestType(REQ_RESOURCE_TUNING);
        exitReq->setHandle(-1);
        exitReq->setDuration(-1);
        exitReq->setProperties(1);
        exitReq->setNumResources(0);
        exitReq->setClientPID(100);
        exitReq->setClientTID(1155);
        exitReq->setResources(nullptr);
        requestQueue->addAndWakeup(exitReq);

        taskCondition = true;
        taskCV.notify_one();
    });

    consumerThread.join();
    terminateThread.join();

    C_ASSERT(requestsProcessed.load() == totalNumberOfThreads + 1);
}

static void TestRequestQueueEmptyPoll() {
    std::shared_ptr<RequestQueue> requestQueue = RequestQueue::getInstance();
    C_ASSERT(requestQueue->pop() == nullptr);
}

static void TestRequestQueuePollingPriority1() {
    std::shared_ptr<RequestQueue> requestQueue = RequestQueue::getInstance();
    int8_t taskCondition = false;
    std::mutex taskLock;
    std::condition_variable taskCV;

    // Create some sample requests with varying priorities
    std::vector<Request*> requests = {
        new Request(), new Request(), new Request(), new Request(), new Request(), new Request()
    };

    requests[0]->setRequestType(REQ_RESOURCE_TUNING);
    requests[0]->setHandle(11);
    requests[0]->setDuration(-1);
    requests[0]->setNumResources(0);
    requests[0]->setProperties(SYSTEM_HIGH);
    requests[0]->setClientPID(321);
    requests[0]->setClientTID(2445);
    requests[0]->setResources(nullptr);

    requests[1]->setRequestType(REQ_RESOURCE_TUNING);
    requests[1]->setHandle(23);
    requests[1]->setDuration(-1);
    requests[1]->setNumResources(0);
    requests[1]->setProperties(SYSTEM_LOW);
    requests[1]->setClientPID(234);
    requests[1]->setClientTID(5566);
    requests[1]->setResources(nullptr);

    requests[2]->setRequestType(REQ_RESOURCE_TUNING);
    requests[2]->setHandle(38);
    requests[2]->setDuration(-1);
    requests[2]->setNumResources(0);
    requests[2]->setProperties(THIRD_PARTY_HIGH);
    requests[2]->setClientPID(522);
    requests[2]->setClientTID(8889);
    requests[2]->setResources(nullptr);

    requests[3]->setRequestType(REQ_RESOURCE_TUNING);
    requests[3]->setHandle(55);
    requests[3]->setDuration(-1);
    requests[3]->setNumResources(0);
    requests[3]->setProperties(THIRD_PARTY_LOW);
    requests[3]->setClientPID(455);
    requests[3]->setClientTID(2547);
    requests[3]->setResources(nullptr);

    requests[4]->setRequestType(REQ_RESOURCE_TUNING);
    requests[4]->setHandle(87);
    requests[4]->setDuration(-1);
    requests[4]->setNumResources(0);
    requests[4]->setProperties(10);
    requests[4]->setClientPID(770);
    requests[4]->setClientTID(7774);
    requests[4]->setResources(nullptr);

    requests[5]->setRequestType(REQ_RESOURCE_TUNING);
    requests[5]->setHandle(-1);
    requests[5]->setDuration(-1);
    requests[5]->setProperties(15);
    requests[5]->setNumResources(0);
    requests[5]->setClientPID(115);
    requests[5]->setClientTID(1211);
    requests[5]->setResources(nullptr);

    // Suppose all the clients corresponding to the above requests have system permission
    // So the processsing order will be determined by the value of priority in the Request
    // - Sort the Requests in the above vector by priority

    sort(requests.begin(), requests.end(), [&](Request* first, Request* second) {
        return first->getPriority() < second->getPriority();
    });

    // Note, we haven't registered the consumer yet, instead we'll first add
    // all the requests to the RequestQueue.
    std::thread producerThread([&]{
        std::unique_lock<std::mutex> uniqueLock(taskLock);

        for(Request* request: requests) {
            requestQueue->addAndWakeup(request);
        }

        taskCondition = true;
        taskCV.notify_one();
    });


    sleep(1);
    int32_t requestsIndex = 0;

    // Next create the Consumer
    std::thread consumerThread([&]{
        std::unique_lock<std::mutex> uniqueLock(taskLock);
        int8_t terminateProducer = false;

        while(true) {
            if(terminateProducer) {
                return;
            }

            while(!taskCondition) {
                taskCV.wait(uniqueLock);
            }

            while(requestQueue->hasPendingTasks()) {
                Request* req = (Request*)requestQueue->pop();

                if(req->getHandle() == -1) {
                    delete req;
                    terminateProducer = true;
                    break;
                }

                C_ASSERT(req->getClientPID() == requests[requestsIndex]->getClientPID());
                C_ASSERT(req->getClientTID() == requests[requestsIndex]->getClientTID());
                C_ASSERT(req->getPriority() == requests[requestsIndex]->getPriority());
                requestsIndex++;

                delete req;
            }
        }
    });

    producerThread.join();
    consumerThread.join();
}

static void TestRequestQueuePollingPriority2() {
    std::shared_ptr<RequestQueue> requestQueue = RequestQueue::getInstance();
    int8_t taskCondition = false;
    std::mutex taskLock;
    std::condition_variable taskCV;

    // Create some sample requests with varying priorities
    Request* systemPermissionRequest = new Request();
    systemPermissionRequest->setRequestType(REQ_RESOURCE_TUNING);
    systemPermissionRequest->setHandle(11);
    systemPermissionRequest->setDuration(-1);
    systemPermissionRequest->setNumResources(0);
    systemPermissionRequest->setProperties(SYSTEM_HIGH);
    systemPermissionRequest->setClientPID(321);
    systemPermissionRequest->setClientTID(2445);
    systemPermissionRequest->setResources(nullptr);

    Request* thirdPartyPermissionRequest = new Request();
    thirdPartyPermissionRequest->setRequestType(REQ_RESOURCE_TUNING);
    thirdPartyPermissionRequest->setHandle(23);
    thirdPartyPermissionRequest->setDuration(-1);
    thirdPartyPermissionRequest->setNumResources(0);
    thirdPartyPermissionRequest->setProperties(THIRD_PARTY_HIGH);
    thirdPartyPermissionRequest->setClientPID(234);
    thirdPartyPermissionRequest->setClientTID(5566);
    thirdPartyPermissionRequest->setResources(nullptr);

    Request* exitReq = new Request();
    exitReq->setRequestType(REQ_RESOURCE_TUNING);
    exitReq->setHandle(-1);
    exitReq->setDuration(-1);
    exitReq->setProperties(THIRD_PARTY_LOW);
    exitReq->setNumResources(0);
    exitReq->setClientPID(102);
    exitReq->setClientTID(1220);
    exitReq->setResources(nullptr);

    // Note, we haven't registered the consumer yet, instead we'll first add
    // both the requests to the RequestQueue.
    std::thread producerThread([&]{
        std::unique_lock<std::mutex> uniqueLock(taskLock);

        requestQueue->addAndWakeup(systemPermissionRequest);
        requestQueue->addAndWakeup(thirdPartyPermissionRequest);
        requestQueue->addAndWakeup(exitReq);

        taskCondition = true;
        taskCV.notify_one();
    });

    sleep(1);
    int32_t requestsIndex = 0;

    // Next create the Consumer
    std::thread consumerThread([&]{
        std::unique_lock<std::mutex> uniqueLock(taskLock);
        int8_t terminateProducer = false;

        while(true) {
            if(terminateProducer) {
                return;
            }

            while(!taskCondition) {
                taskCV.wait(uniqueLock);
            }

            while(requestQueue->hasPendingTasks()) {
                Request* req = (Request*)requestQueue->pop();

                if(req->getHandle() == -1) {
                    delete req;
                    terminateProducer = true;
                    break;
                }

                if(requestsIndex == 0) {
                    C_ASSERT(req->getClientPID() == 321);
                    C_ASSERT(req->getClientTID() == 2445);
                    C_ASSERT(req->getHandle() == 11);
                } else if(requestsIndex == 1) {
                    C_ASSERT(req->getClientPID() == 234);
                    C_ASSERT(req->getClientTID() == 5566);
                    C_ASSERT(req->getHandle() == 23);
                }
                requestsIndex++;

                delete req;
            }
        }
    });

    consumerThread.join();
    producerThread.join();
}

static void TestRequestQueueInvalidPriority() {
    std::shared_ptr<RequestQueue> requestQueue = RequestQueue::getInstance();

    Request* invalidRequest = new Request();
    invalidRequest->setRequestType(REQ_RESOURCE_TUNING);
    invalidRequest->setHandle(11);
    invalidRequest->setDuration(-1);
    invalidRequest->setNumResources(0);
    invalidRequest->setProperties(-15);
    invalidRequest->setClientPID(321);
    invalidRequest->setClientTID(2445);
    invalidRequest->setResources(nullptr);

    C_ASSERT(requestQueue->addAndWakeup(invalidRequest) == false);
}

int32_t main() {
    std::cout<<"Running Test Suite: [RequestQueueTests]\n"<<std::endl;

    Init();
    RUN_TEST(TestRequestQueueTaskEnqueue);
    RUN_TEST(TestRequestQueueSingleTaskPickup1);
    RUN_TEST(TestRequestQueueSingleTaskPickup2);
    RUN_TEST(TestRequestQueueMultipleTaskPickup);
    RUN_TEST(TestRequestQueueMultipleTaskAndProducersPickup);
    RUN_TEST(TestRequestQueueEmptyPoll);
    RUN_TEST(TestRequestQueuePollingPriority1);
    RUN_TEST(TestRequestQueuePollingPriority2);
    RUN_TEST(TestRequestQueueInvalidPriority);

    std::cout<<"\nAll Tests from the suite: [RequestQueueTests], executed successfully"<<std::endl;
    return 0;
}
