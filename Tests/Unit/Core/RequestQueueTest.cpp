// Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
// SPDX-License-Identifier: BSD-3-Clause-Clear

// #include <gtest/gtest.h>

// #include "Request.h"
// #include "RequestQueue.h"

// class RequestQueueTests: public::testing::Test {
// protected:
//     void SetUp() override {
//         static int8_t firstTest = true;
//         if(firstTest == true) {
//             MakeAlloc<ClientInfo> (30);
//             MakeAlloc<ClientTidData> (30);
//             MakeAlloc<std::unordered_set<int64_t>> (30);
//             MakeAlloc<Resource> (30);
//             MakeAlloc<std::vector<Resource*>> (30);
//             MakeAlloc<Request> (30);
//             firstTest = false;
//         }

//         ResourceTunerSettings::metaConfigs.mDelta = 5000;
//         ResourceTunerSettings::metaConfigs.mPenaltyFactor = 2.0;
//         ResourceTunerSettings::metaConfigs.mRewardFactor = 0.4;
//     }
// };

// TEST(RequestQueueTaskProcessingTests, TestRequestQueueTaskEnqueue) {
//     std::shared_ptr<RequestQueue> requestQueue = RequestQueue::getInstance();
//     int32_t requestCount = 8;
//     int32_t requestsProcessed = 0;

//     for(int32_t count = 0; count < requestCount; count++) {
//         Resource* resource = (Resource*) GetBlock<Resource>();

//         resource->setResCode(16);
//         resource->mNumValues = 1;
//         resource->mResValue.value = 8;

//         std::vector<Resource*>* resources =
//             new (GetBlock<std::vector<Resource*>>())std::vector<Resource*>;
//         resources->push_back(resource);

//         Request* request = new (GetBlock<Request>()) Request;
//         request->setRequestType(REQ_RESOURCE_TUNING);
//         request->setHandle(25);
//         request->setDuration(-1);
//         request->setPriority(0);
//         request->setNumResources(1);
//         request->setClientPID(321);
//         request->setClientTID(2445);
//         request->setResources(resources);
//         request->setBackgroundProcessing(false);

//         requestQueue->addAndWakeup(request);
//     }

//     while(requestQueue->hasPendingTasks()) {
//         requestsProcessed++;
//         Request* request = (Request*)requestQueue->pop();

//         delete request->getResources();
//         delete request;
//     }

//     ASSERT_EQ(requestsProcessed, requestCount);
// }

// TEST(RequestQueueTaskProcessingTests, TestRequestQueueSingleTaskPickup1) {
//     std::shared_ptr<RequestQueue> requestQueue = RequestQueue::getInstance();

//     // Consumer
//     std::thread consumerThread([&]{
//         requestQueue->registerAsConsumer();
//         requestQueue->wait();

//         while(requestQueue->hasPendingTasks()) {
//             Request* req = (Request*)requestQueue->pop();

//             ASSERT_EQ(req->getRequestType(), REQ_RESOURCE_TUNING);
//             ASSERT_EQ(req->getHandle(), 200);
//             ASSERT_EQ(req->getClientPID(), 321);
//             ASSERT_EQ(req->getClientTID(), 2445);
//             ASSERT_EQ(req->getDuration(), -1);

//             delete req;
//         }

//         requestQueue->unRegisterAsConsumer();
//     });

//     // Producer
//     // Enqueue a Request
//     Request* req = new Request(REQ_RESOURCE_TUNING, 200, -1, 1, 0, 321, 2445, nullptr);
//     requestQueue->addAndWakeup(req);

//     consumerThread.join();
// }

// TEST(RequestQueueTaskProcessingTests, TestRequestQueueSingleTaskPickup2) {
//     std::shared_ptr<RequestQueue> requestQueue = RequestQueue::getInstance();
//     std::atomic<int32_t> requestsProcessed(0);

//     // Consumer
//     std::thread consumerThread([&]{
//         requestQueue->registerAsConsumer();
//         int8_t terminateProducer = false;

//         while(true) {
//             if(terminateProducer) {
//                 requestQueue->unRegisterAsConsumer();
//                 return;
//             }

//             requestQueue->wait();

//             while(requestQueue->hasPendingTasks()) {
//                 Request* req = (Request*)requestQueue->pop();
//                 requestsProcessed.fetch_add(1);

//                 if(req->getHandle() == -1) {
//                     delete req;
//                     terminateProducer = true;
//                     break;
//                 }

//                 delete req;
//             }
//         }
//     });

//     // producer
//     std::thread producerThread([&]{
//         Request* req = new Request(REQ_RESOURCE_TUNING, -1, -1, 1, 0, 321, 2445, nullptr);
//         requestQueue->addAndWakeup(req);
//     });

//     producerThread.join();
//     consumerThread.join();

//     ASSERT_EQ(requestsProcessed.load(), 1);
// }

// TEST(RequestQueueTaskProcessingTests, TestRequestQueueMultipleTaskPickup) {
//     std::shared_ptr<RequestQueue> requestQueue = RequestQueue::getInstance();
//     std::atomic<int32_t> requestsProcessed(0);

//     // Consumer
//     std::thread taskThread([&]{
//         requestQueue->registerAsConsumer();
//         int8_t terminateProducer = false;

//         while(true) {
//             if(terminateProducer) {
//                 requestQueue->unRegisterAsConsumer();
//                 return;
//             }

//             requestQueue->wait();

//             while(requestQueue->hasPendingTasks()) {
//                 Request* req = (Request*)requestQueue->pop();
//                 requestsProcessed.fetch_add(1);

//                 if(req->getHandle() == -1) {
//                     delete req;
//                     terminateProducer = true;
//                     break;
//                 }

//                 delete req;
//             }
//         }
//     });

//     // Producer will enqueue multiple requests
//     Request* req1 = new Request(REQ_RESOURCE_TUNING, 200, -1, 1, 0, 321, 2445, nullptr);
//     Request* req2 = new Request(REQ_RESOURCE_TUNING, 200, -1, 1, 0, 321, 2445, nullptr);
//     Request* req3 = new Request(REQ_RESOURCE_TUNING, 200, -1, 1, 0, 321, 2445, nullptr);

//     requestQueue->addAndWakeup(req1);
//     requestQueue->addAndWakeup(req2);
//     requestQueue->addAndWakeup(req3);

//     // Instrumented request to force the taskThread to terminate
//     Request* exitReq = new Request(REQ_RESOURCE_TUNING, -1, -1, 1, 4, 321, 2445, nullptr);
//     requestQueue->addAndWakeup(exitReq);

//     taskThread.join();

//     ASSERT_EQ(requestsProcessed.load(), 4);
// }


// TEST(RequestQueueTaskProcessingTests, TestRequestQueueMultipleTaskAndProducersPickup) {
//     std::shared_ptr<RequestQueue> requestQueue = RequestQueue::getInstance();
//     std::atomic<int32_t> requestsProcessed(0);
//     int32_t totalNumberOfThreads = 10;

//     std::thread consumerThread([&]{
//         int8_t terminateProducer = false;
//         requestQueue->registerAsConsumer();

//         while(true) {
//             if(terminateProducer) {
//                 requestQueue->unRegisterAsConsumer();
//                 return;
//             }

//             requestQueue->wait();

//             while(requestQueue->hasPendingTasks()) {
//                 Request* req = (Request*)requestQueue->pop();
//                 requestsProcessed.fetch_add(1);

//                 if(req->getHandle() == -1) {
//                     delete req;
//                     terminateProducer = true;
//                     break;
//                 }

//                 delete req;
//             }
//         }
//     });

//     std::vector<std::thread> producerThreads;

//     // Create multiple producer threads
//     for(int32_t count = 0; count < totalNumberOfThreads; count++) {
//         auto threadStartRoutine = [&]{
//             Request* req = new Request(REQ_RESOURCE_TUNING, count, -1, 1, 0, 321, 2445, nullptr);
//             requestQueue->addAndWakeup(req);
//         };
//         producerThreads.push_back(std::thread(threadStartRoutine));
//     }

//     for(int32_t i = 0; i < producerThreads.size(); i++) {
//         producerThreads[i].join();
//     }

//     // Instrumented request to force the consumerThread to terminate
//     Request* exitReq = new Request(REQ_RESOURCE_TUNING, -1, -1, 1, 4, 321, 2445, nullptr);
//     requestQueue->addAndWakeup(exitReq);

//     consumerThread.join();

//     ASSERT_EQ(requestsProcessed.load(), totalNumberOfThreads + 1);
// }


// TEST(RequestQueuePollingTests, TestRequestQueueEmptyPoll) {
//     std::shared_ptr<RequestQueue> requestQueue = RequestQueue::getInstance();
//     ASSERT_EQ(requestQueue->pop(), nullptr);
// }


// TEST(RequestQueuePollingTests, TestRequestQueuePollingPriority1) {
//     std::shared_ptr<RequestQueue> requestQueue = RequestQueue::getInstance();

//     // Create some sample requests with varying priorities
//     std::vector<Request*> requests = {
//         new Request(REQ_RESOURCE_TUNING, 11, -1, 1, 15, 321, 2445, nullptr),
//         new Request(REQ_RESOURCE_TUNING, 23, -1, 1, 21, 234, 5566, nullptr),
//         new Request(REQ_RESOURCE_TUNING, 38, -1, 1, 11, 522, 8889, nullptr),
//         new Request(REQ_RESOURCE_TUNING, 55, -1, 1, 48, 455, 2547, nullptr),
//         new Request(REQ_RESOURCE_TUNING, 87, -1, 1, 37, 770, 7774, nullptr)
//     };

//     // Suppose all the clients corresponding to the above requests have system permission
//     // So the processsing order will be determined by the value of priority in the Request
//     // - Sort the Requests in the above vector by priority

//     sort(requests.begin(), requests.end(), [&](Request* first, Request* second) {
//         return first->getPriority() > second->getPriority();
//     });

//     // Note, we haven't registered the consumer yet, instead we'll first add
//     // all the requests to the RequestQueue.
//     for(Request* request: requests) {
//         requestQueue->addAndWakeup(request);
//     }

//     sleep(1);
//     int32_t requestsIndex = 0;

//     // Next create the Consumer
//     std::thread consumerThread([&]{
//         int8_t terminateProducer = false;
//         requestQueue->registerAsConsumer();

//         while(true) {
//             if(terminateProducer) {
//                 requestQueue->unRegisterAsConsumer();
//                 return;
//             }

//             requestQueue->wait();

//             while(requestQueue->hasPendingTasks()) {
//                 Request* req = (Request*)requestQueue->pop();

//                 if(req->getHandle() == -1) {
//                     delete req;
//                     terminateProducer = true;
//                     break;
//                 }

//                 ASSERT_EQ(req->getClientPID(), requests[requestsIndex]->getClientPID());
//                 ASSERT_EQ(req->getClientTID(), requests[requestsIndex]->getClientTID());
//                 ASSERT_EQ(req->getPriority(), requests[requestsIndex]->getPriority());
//                 requestsIndex++;

//                 delete req;
//             }
//         }
//     });

//     Request* exitReq = new Request(REQ_RESOURCE_TUNING, -1, -1, 1, 4, 321, 2445, nullptr);
//     requestQueue->addAndWakeup(exitReq);

//     consumerThread.join();
// }

// TEST(RequestQueuePollingTests, TestRequestQueuePollingPriority2) {
//     std::shared_ptr<RequestQueue> requestQueue = RequestQueue::getInstance();

//     // Create some sample requests with varying priorities
//     Request* systemPermissionRequest =
//         new Request(REQ_RESOURCE_TUNING, 11, -1, 1, 0, 321, 2445, nullptr);
//     Request* thirdPartyPermissionRequest =
//         new Request(REQ_RESOURCE_TUNING, 23, -1, 1, 1, 234, 5566, nullptr);

//     // Note, we haven't registered the consumer yet, instead we'll first add
//     // both the requests to the RequestQueue.
//     requestQueue->addAndWakeup(systemPermissionRequest);
//     requestQueue->addAndWakeup(thirdPartyPermissionRequest);

//     sleep(1);
//     int32_t requestsIndex = 0;

//     // Next create the Consumer
//     std::thread consumerThread([&]{
//         int8_t terminateProducer = false;
//         requestQueue->registerAsConsumer();

//         while(true) {
//             if(terminateProducer) {
//                 requestQueue->unRegisterAsConsumer();
//                 return;
//             }

//             requestQueue->wait();

//             while(requestQueue->hasPendingTasks()) {
//                 Request* req = (Request*)requestQueue->pop();

//                 if(req->getHandle() == -1) {
//                     delete req;
//                     terminateProducer = true;
//                     break;
//                 }

//                 if(requestsIndex == 0) {
//                     ASSERT_EQ(req->getClientPID(), 321);
//                     ASSERT_EQ(req->getClientTID(), 2445);
//                     ASSERT_EQ(req->getHandle(), 11);
//                 } else if(requestsIndex == 1) {
//                     ASSERT_EQ(req->getClientPID(), 234);
//                     ASSERT_EQ(req->getClientTID(), 5566);
//                     ASSERT_EQ(req->getHandle(), 23);
//                 }
//                 requestsIndex++;

//                 delete req;
//             }
//         }
//     });

//     Request* exitReq = new Request(REQ_RESOURCE_TUNING, -1, -1, 1, 4, 321, 2445, nullptr);
//     requestQueue->addAndWakeup(exitReq);

//     consumerThread.join();
// }

// TEST(RequestQueueEdgeCasesTests, TestRequestQueueInvalidPriority) {
//     std::shared_ptr<RequestQueue> requestQueue = RequestQueue::getInstance();

//     requestQueue->registerAsConsumer();

//     while(requestQueue->hasPendingTasks()) {
//         requestQueue->pop();
//     }

//     ASSERT_EQ(requestQueue->hasPendingTasks(), false);

//     requestQueue->unRegisterAsConsumer();

//     Request* request = new Request(REQ_RESOURCE_TUNING, -1, -1, 1, -2, 321, 2445, nullptr);
//     requestQueue->addAndWakeup(request);

//     ASSERT_EQ(requestQueue->hasPendingTasks(), false);
// }
