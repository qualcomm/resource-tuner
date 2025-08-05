// Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
// SPDX-License-Identifier: BSD-3-Clause-Clear

// #include <gtest/gtest.h>
// #include <stdio.h>
// #include <stdlib.h>
// #include <unistd.h>
// #include <limits.h>
// #include <memory>

// #include "Extensions.h"
// #include "CocoTable.h"
// #include "ResourceRegistry.h"
// #include "SyslockServerRequests.h"

// URM_REGISTER_CONFIG(RESOURCE_CONFIG, "../Tests/Configs/testResourceConfigs.json")

// class CocoTableTest : public ::testing::Test {
// protected:
//     void SetUp() override {
//         static int8_t firstTest = true;

//         if(firstTest == true) {
//             firstTest = false;
//             ResourceProcessor resourceProcessor(Extensions::getResourceConfigFilePath());

//             if(RC_IS_NOTOK(resourceProcessor.parseResourceConfigs())) {
//                 return;
//             }

//             MakeAlloc<ClientInfo> (120);
//             MakeAlloc<ClientTidData> (120);
//             MakeAlloc<std::unordered_set<int64_t>> (120);
//             MakeAlloc<Resource> (120);
//             MakeAlloc<std::vector<Resource*>> (120);
//             MakeAlloc<Request> (120);
//             MakeAlloc<CocoNode>(120);
//         }
//     }

//     std::vector<std::vector<std::pair<CocoNode*, CocoNode*>>> getCocoTableInternal() {
//         return CocoTable::getInstance()->mCocoTable;
//     }

//     Resource* getCocoNodeResource(CocoNode* cocoNode) {
//         return cocoNode->mResource;
//     }

//     int32_t getCocoTablePrimaryIndex(uint32_t opId) {
//        return CocoTable::getInstance()->getCocoTablePrimaryIndex(opId);
//     }

//     int32_t getCocoTableSecondaryIndex(uint32_t opId, int32_t mOpInfo, int32_t priority) {
//         return CocoTable::getInstance()->getCocoTableSecondaryIndex(opId, mOpInfo, priority);
//     }

//     CocoNode* getNext(CocoNode* cocoTableNode) {
//         return cocoTableNode->next;
//     }

//     int32_t getCurrentlyAppliedPriorityAtIndex(int32_t index) {
//         return CocoTable::getInstance()-> mCurrentlyAppliedPriority[index];
//     }

//     std::string readFromNode(const std::string& fName) {
//         std::fstream myFile(fName, std::ios::in | std::ios::out | std::ios::app);
//         std::string value;

//         if(myFile.is_open()) {
//             getline(myFile, value);
//             myFile.close();
//         } else {
//             LOGE("URM_COCO_TABLE",
//                  "Failed to open the file: " + fName);
//             return "";
//         }
//         return value;
//     }
// };

// TEST_F(CocoTableTest, ConstructorInitializesFields) {
//     int32_t totalResources = ResourceRegistry::getInstance()->getTotalResourcesCount();
//     EXPECT_EQ(totalResources, 4);
//     EXPECT_EQ(getCocoTableInternal().size(), 4);

//     EXPECT_EQ(getCocoTableInternal()[0].size(), TOTAL_PRIORITIES);
//     EXPECT_EQ(getCocoTableInternal()[1].size(), TOTAL_PRIORITIES * TOTAL_CORES);
//     EXPECT_EQ(getCocoTableInternal()[2].size(), TOTAL_PRIORITIES);
//     EXPECT_EQ(getCocoTableInternal()[3].size(), TOTAL_PRIORITIES * TOTAL_CORES);
// }

// TEST_F(CocoTableTest, BasicFunctionality) {
//     int32_t valueToBeWritten = 100;

//     Resource* resource = (Resource*) (GetBlock<Resource>());
//     resource->mOpCode = (1 << 31) | (1 << 16);
//     resource->mOptionalInfo = 0;
//     resource->mNumValues = 1;
//     resource->mConfigValue.singleValue = valueToBeWritten;

//     std::vector<Resource*>* resources =
//         new (GetBlock<std::vector<Resource*>>()) std::vector<Resource*>;

//     resources->push_back(resource);

//     Request* request = new (GetBlock<Request>())
//                             Request(REQ_RESOURCE_TUNING, 1, -1, 1, 1, 1, 1, resources);

//     CocoTable::getInstance()->insertRequest(request);

//     EXPECT_EQ(readFromNode("../Tests/Configs/sched_util_clamp_min"),
//               std::to_string(valueToBeWritten));

//     CocoTable::getInstance()->removeRequest(request);

//     Request::cleanUpRequest(request);
// }

// TEST_F(CocoTableTest, HigherBetter) {
//     std::vector<int32_t> values = {300, 500, 100, 200};
//     uint32_t testOpId = (1 << 31) | (1 << 16);

//     std::vector<Request*> requests;
//     for(int32_t i = 0; i < values.size(); i++) {
//         std::vector<Resource*>* resources =
//             new (GetBlock<std::vector<Resource*>>()) std::vector<Resource*>;

//         Resource* resource = (Resource*) (GetBlock<Resource>());
//         resource->mOpCode = testOpId;
//         resource->mOptionalInfo = 0;
//         resource->mNumValues = 1;
//         resource->mConfigValue.singleValue = values[i];

//         resources->push_back(resource);

//         Request* request = new (GetBlock<Request>())
//                                 Request(REQ_RESOURCE_TUNING, 1, -1, 1, 1, 1, 1, resources);

//         requests.push_back(request);
//         CocoTable::getInstance()->insertRequest(request);
//     }

//     int32_t index = getCocoTableIndex(testOpId);
//     ASSERT_GE(index, 0);
//     ASSERT_LT(index, getCocoTableInternal().size());

//     std::sort(values.begin(), values.end(), std::greater<int32_t>());

//     CocoNode* head = getCocoTableInternal().at(index).at(1).second;

//     for(int32_t i = 0; i < values.size(); i++) {
//         ASSERT_NE(head, nullptr);
//         EXPECT_EQ(getCocoNodeResource(head)->mConfigValue.singleValue, values[i]);
//         head = getNext(head);
//     }
//     EXPECT_EQ(head, nullptr);

//     for(Request* request : requests) {
//         CocoTable::getInstance()->removeRequest(request);
//     }

//     for(Request* request : requests) {
//         Request::cleanUpRequest(request);
//     }
// }

// TEST_F(CocoTableTest, LowerBetter) {
//     std::vector<int32_t> values = {300, 500, 100, 200};

//     uint32_t testOpId = (1 << 31) | (1 << 16) | (1 << 0);

//     std::vector<Request*> requests;
//     for(int32_t i = 0; i < values.size(); i++) {
//         std::vector<Resource*>* resources =
//             new (GetBlock<std::vector<Resource*>>()) std::vector<Resource*>;

//         Resource* resource = (Resource*) (GetBlock<Resource>());
//         resource->mOpCode = testOpId;
//         resource->mOptionalInfo = 0;
//         resource->mNumValues = 1;
//         resource->mConfigValue.singleValue = values[i];

//         resources->push_back(resource);

//         Request* request = new (GetBlock<Request>())
//                                 Request(REQ_RESOURCE_TUNING, 1, -1, 1, 1, 1, 1, resources);

//         requests.push_back(request);
//         CocoTable::getInstance()->insertRequest(request);
//     }

//     int32_t index = getCocoTableIndex(testOpId);
//     ASSERT_GE(index, 0);
//     ASSERT_LT(index, getCocoTableInternal().size());

//     std::sort(values.begin(), values.end());

//     CocoNode* head = getCocoTableInternal().at(index).at(1).second;

//     for(int32_t i = 0; i < values.size(); i++) {
//         ASSERT_NE(head, nullptr);
//         EXPECT_EQ(getCocoNodeResource(head)->mConfigValue.singleValue, values[i]);
//         head = getNext(head);
//     }
//     EXPECT_EQ(head, nullptr);

//     for(Request* request : requests) {
//         CocoTable::getInstance()->removeRequest(request);
//     }

//     for(Request* request : requests) {
//         Request::cleanUpRequest(request);
//     }
// }

// TEST_F(CocoTableTest, LazyApply) {
//     std::vector<int32_t> values = {300, 500, 100, 200};

//     uint32_t testOpId = (1 << 31) | (1 << 16) | (1 << 1);

//     std::vector<Request*> requests;
//     for(int32_t i = 0; i < values.size(); i++) {
//         std::vector<Resource*>* resources =
//             new (GetBlock<std::vector<Resource*>>()) std::vector<Resource*>;

//         Resource* resource = (Resource*) (GetBlock<Resource>());
//         resource->mOpCode = testOpId;
//         resource->mOptionalInfo = 0;
//         resource->mNumValues = 1;
//         resource->mConfigValue.singleValue = values[i];

//         resources->push_back(resource);

//         Request* request = new (GetBlock<Request>())
//                                 Request(REQ_RESOURCE_TUNING, 1, -1, 1, 1, 1, 1, resources);

//         requests.push_back(request);
//         CocoTable::getInstance()->insertRequest(request);
//     }

//     int32_t index = getCocoTableIndex(testOpId);
//     ASSERT_GE(index, 0);
//     ASSERT_LT(index, getCocoTableInternal().size());

//     CocoNode* head = getCocoTableInternal().at(index).at(1).second;

//     for(int32_t i = 0; i < values.size(); i++) {
//         ASSERT_NE(head, nullptr);
//         EXPECT_EQ(getCocoNodeResource(head)->mConfigValue.singleValue, values[i]);
//         head = getNext(head);
//     }
//     EXPECT_EQ(head, nullptr);

//     for(Request* request : requests) {
//         CocoTable::getInstance()->removeRequest(request);
//     }

//     for(Request* request : requests) {
//         Request::cleanUpRequest(request);
//     }
// }

// TEST_F(CocoTableTest, InstantApply) {
//     std::vector<int32_t> values = {300, 500, 100, 200};

//     uint32_t testOpId = (1 << 31) | (1 << 16) | (1 << 1) | (1 << 0);

//     std::vector<Request*> requests;
//     for(int32_t i = 0; i < values.size(); i++) {
//         std::vector<Resource*>* resources =
//             new (GetBlock<std::vector<Resource*>>()) std::vector<Resource*>;

//         Resource* resource = (Resource*) (GetBlock<Resource>());
//         resource->mOpCode = testOpId;
//         resource->mOptionalInfo = 0;
//         resource->mNumValues = 1;
//         resource->mConfigValue.singleValue = values[i];

//         resources->push_back(resource);

//         Request* request = new (GetBlock<Request>())
//                                 Request(REQ_RESOURCE_TUNING, 1, -1, 1, 1, 1, 1, resources);

//         requests.push_back(request);
//         CocoTable::getInstance()->insertRequest(request);
//     }

//     int32_t index = getCocoTableIndex(testOpId);
//     ASSERT_GE(index, 0);
//     ASSERT_LT(index, getCocoTableInternal().size());

//     std::reverse(values.begin(), values.end());

//     CocoNode* head = getCocoTableInternal().at(index).at(1).second;

//     for(int32_t i = 0; i < values.size(); i++) {
//         ASSERT_NE(head, nullptr);
//         EXPECT_EQ(getCocoNodeResource(head)->mConfigValue.singleValue, values[i]);
//         head = getNext(head);
//     }
//     EXPECT_EQ(head, nullptr);

//     for(Request* request : requests) {
//         CocoTable::getInstance()->removeRequest(request);
//     }

//     for(Request* request : requests) {
//         Request::cleanUpRequest(request);
//     }
// }

// TEST_F(CocoTableTest, HigherBetterStress)
// {
//     std::vector<int32_t> values;
//     uint32_t testOpId = (1 << 31) | (1 << 16);
//     std::vector<Request*> requests;

//     for(int32_t i = 0; i < 100; i++) {
//         std::vector<Resource*>* resources =
//             new (GetBlock<std::vector<Resource*>>()) std::vector<Resource*>;

//         values.push_back(rand() % 1000);

//         Resource* resource = (Resource*) (GetBlock<Resource>());
//         resource->mOpCode = testOpId;
//         resource->mOptionalInfo = 0;
//         resource->mNumValues = 1;
//         resource->mConfigValue.singleValue = values.back();

//         resources->push_back(resource);

//         Request* request = new (GetBlock<Request>())
//                                 Request(REQ_RESOURCE_TUNING, 1, -1, 1, 1, 1, 1, resources);

//         requests.push_back(request);
//         CocoTable::getInstance()->insertRequest(request);
//     }

//     int32_t index = getCocoTableIndex(testOpId);
//     ASSERT_GE(index, 0);
//     ASSERT_LT(index, getCocoTableInternal().size());

//     std::sort(values.begin(), values.end(), std::greater<int32_t>());

//     CocoNode* head = getCocoTableInternal().at(index).at(1).second;

//     for(int32_t i = 0; i < values.size(); i++) {
//         ASSERT_NE(head, nullptr);
//         EXPECT_EQ(getCocoNodeResource(head)->mConfigValue.singleValue, values[i]);
//         head = getNext(head);
//     }
//     EXPECT_EQ(head, nullptr);

//     for(Request* request : requests) {
//         CocoTable::getInstance()->removeRequest(request);
//     }

//     for(Request* request : requests) {
//         Request::cleanUpRequest(request);
//     }
// }

// TEST_F(CocoTableTest, LowerBetterStress) {
//     std::vector<int32_t> values;
//     uint32_t testOpId = (1 << 31) | (1 << 16) | (1 << 0);
//     std::vector<Request*> requests;

//     for(int32_t i = 0; i < 100; i++) {
//         std::vector<Resource*>* resources =
//             new (GetBlock<std::vector<Resource*>>()) std::vector<Resource*>;

//         values.push_back(rand() % 1000);

//         Resource* resource = (Resource*) (GetBlock<Resource>());
//         resource->mOpCode = testOpId;
//         resource->mOptionalInfo = 0;
//         resource->mNumValues = 1;
//         resource->mConfigValue.singleValue = values.back();

//         resources->push_back(resource);

//         Request* request = new (GetBlock<Request>())
//                                 Request(REQ_RESOURCE_TUNING, 1, -1, 1, 1, 1, 1, resources);

//         requests.push_back(request);
//         CocoTable::getInstance()->insertRequest(request);
//     }

//     int32_t index = getCocoTableIndex(testOpId);
//     ASSERT_GE(index, 0);
//     ASSERT_LT(index, getCocoTableInternal().size());

//     std::sort(values.begin(), values.end());

//     CocoNode* head = getCocoTableInternal().at(index).at(1).second;

//     for(int32_t i = 0; i < values.size(); i++) {
//         ASSERT_NE(head, nullptr);
//         EXPECT_EQ(getCocoNodeResource(head)->mConfigValue.singleValue, values[i]);
//         head = getNext(head);
//     }
//     EXPECT_EQ(head, nullptr);

//     for(Request* request : requests) {
//         CocoTable::getInstance()->removeRequest(request);
//     }

//     for(Request* request : requests) {
//         Request::cleanUpRequest(request);
//     }
// }

// TEST_F(CocoTableTest, LazyApplyStress) {
//     uint32_t testOpId = (1 << 31) | (1 << 16) | (1 << 1);
//     std::vector<int32_t> values;
//     std::vector<Request*> requests;

//     for(int32_t i = 0; i < 100; i++) {
//         std::vector<Resource*>* resources =
//             new (GetBlock<std::vector<Resource*>>()) std::vector<Resource*>;

//         values.push_back(rand() % 1000);

//         Resource* resource = (Resource*) (GetBlock<Resource>());
//         resource->mOpCode = testOpId;
//         resource->mOptionalInfo = 0;
//         resource->mNumValues = 1;
//         resource->mConfigValue.singleValue = values.back();

//         resources->push_back(resource);

//         Request* request = new (GetBlock<Request>())
//                                 Request(REQ_RESOURCE_TUNING, 1, -1, 1, 1, 1, 1, resources);

//         requests.push_back(request);
//         CocoTable::getInstance()->insertRequest(request);
//     }

//     int32_t index = getCocoTableIndex(testOpId);
//     ASSERT_GE(index, 0);
//     ASSERT_LT(index, getCocoTableInternal().size());

//     CocoNode* head = getCocoTableInternal().at(index).at(1).second;

//     for(int32_t i = 0; i < values.size(); i++) {
//         ASSERT_NE(head, nullptr);
//         EXPECT_EQ(getCocoNodeResource(head)->mConfigValue.singleValue, values[i]);
//         head = getNext(head);
//     }
//     EXPECT_EQ(head, nullptr);

//     for(Request* request : requests) {
//         CocoTable::getInstance()->removeRequest(request);
//     }

//     for(Request* request : requests) {
//         Request::cleanUpRequest(request);
//     }
// }

// TEST_F(CocoTableTest, InstantApplyStress) {
//     std::vector<int32_t> values;
//     uint32_t testOpId = (1 << 31) | (1 << 16) | (1 << 1) | (1 << 0);
//     std::vector<Request*> requests;

//     for(int32_t i = 0; i < 100; i++) {
//         std::vector<Resource*>* resources =
//             new (GetBlock<std::vector<Resource*>>()) std::vector<Resource*>;

//         values.push_back(rand() % 1000);

//         Resource* resource = (Resource*) (GetBlock<Resource>());
//         resource->mOpCode = testOpId;
//         resource->mOptionalInfo = 0;
//         resource->mNumValues = 1;
//         resource->mConfigValue.singleValue = values.back();

//         resources->push_back(resource);

//         Request* request = new (GetBlock<Request>())
//                                 Request(REQ_RESOURCE_TUNING, 1, -1, 1, 1, 1, 1, resources);

//         requests.push_back(request);
//         CocoTable::getInstance()->insertRequest(request);
//     }

//     int32_t index = getCocoTableIndex(testOpId);
//     ASSERT_GE(index, 0);
//     ASSERT_LT(index, getCocoTableInternal().size());

//     std::reverse(values.begin(), values.end());

//     CocoNode* head = getCocoTableInternal().at(index).at(1).second;

//     for(int32_t i = 0; i < values.size(); i++) {
//         ASSERT_NE(head, nullptr);
//         EXPECT_EQ(getCocoNodeResource(head)->mConfigValue.singleValue, values[i]);
//         head = getNext(head);
//     }
//     EXPECT_EQ(head, nullptr);

//     for(Request* request : requests) {
//         CocoTable::getInstance()->removeRequest(request);
//     }

//     for(Request* request : requests) {
//         Request::cleanUpRequest(request);
//     }
// }

// TEST_F(CocoTableTest, BasicDeletionTest) {
//     uint32_t testOpId = (1 << 31) | (1 << 16);

//     std::vector<int32_t> values = {300, 500, 100, 200};
//     std::vector<Request*> requests;

//     for(int32_t i = 0; i < values.size(); i++) {
//          std::vector<Resource*>* resources =
//             new (GetBlock<std::vector<Resource*>>()) std::vector<Resource*>;

//         Resource* resource = (Resource*) (GetBlock<Resource>());
//         resource->mOpCode = testOpId;
//         resource->mOptionalInfo = 0;
//         resource->mNumValues = 1;
//         resource->mConfigValue.singleValue = values[i];

//         resources->push_back(resource);

//         Request* request = new (GetBlock<Request>())
//                                 Request(REQ_RESOURCE_TUNING, 1, -1, 1, 1, 1, 1, resources);

//         requests.push_back(request);
//         CocoTable::getInstance()->insertRequest(requests.back());
//     }

//     int32_t index = getCocoTableIndex(testOpId);
//     ASSERT_GE(index, 0);
//     ASSERT_LT(index, getCocoTableInternal().size());

//     CocoNode* head = getCocoTableInternal().at(index).at(1).second;

//     std::sort(values.begin(), values.end(), std::greater<int32_t>());

//     for(int32_t i = 0; i < values.size(); i++) {
//         ASSERT_NE(head, nullptr);
//         EXPECT_EQ(getCocoNodeResource(head)->mConfigValue.singleValue, values[i]);
//         head = getNext(head);
//     }
//     EXPECT_EQ(head, nullptr);

//     CocoTable::getInstance()->removeRequest(requests[0]); //Delete 300 valued request.

//     CocoTable::getInstance()->removeRequest(requests[1]); //Delete 500 valued request. 500 and 300 should be deleted;

//     head = getCocoTableInternal().at(index).at(1).second;
//     ASSERT_NE(head, nullptr);
//     EXPECT_EQ(getCocoNodeResource(head)->mConfigValue.singleValue, 200);
//     head = getNext(head);

//     ASSERT_NE(head, nullptr);
//     EXPECT_EQ(getCocoNodeResource(head)->mConfigValue.singleValue, 100);
//     head = getNext(head);

//     EXPECT_EQ(head, nullptr);

//     for(Request* request : requests) {
//         Request::cleanUpRequest(request);
//     }
// }

// // TEST_F(CocoTableTest, PriorityCheckingHigherBetterRandomDeletion) {

// //     uint32_t testOpId = (1 << 31) | (1 << 16);

// //     //This is for true randomness. seed is printed to reproduce a failing test.
// //     unsigned seed = static_cast<unsigned>(time(nullptr));
// //     srand(seed);

// //     std::vector<Request*> requests;
// //     // Track active requests and their values
// //     std::vector<std::pair<int32_t, Request*>> activeRequests;

// //     for(int32_t i = 0; i < 100; ++i) {
// //         int32_t action = rand() % 2;

// //         if(action == 0 || activeRequests.empty() || i == 99) {
// //             // Insert
// //             // Ensures that last action is an insertion.
// //             int32_t val = rand() % 100;

// //             Resource* resource = (Resource*) (GetBlock<Resource>());
// //             resource->mOpCode = testOpId;
// //             resource->mOptionalInfo = 0;
// //             resource->mNumValues = 1;
// //             resource->mConfigValue.singleValue = val;

// //             std::vector<Resource*>* resourceVec =
// //                 new (GetBlock<std::vector<Resource*>>()) std::vector<Resource*>;

// //             resourceVec->push_back(resource);

// //             Request* req = new (GetBlock<Request>())
// //                                 Request(REQ_RESOURCE_TUNING, 1, -1, 1, 1, 1, 1, resourceVec);

// //             // Insert this request to the CocoTable
// //             CocoTable::getInstance()->insertRequest(req);
// //             requests.push_back(req);
// //             activeRequests.push_back({val, req});
// //         } else {
// //             int32_t idx = rand() % activeRequests.size();
// //             CocoTable::getInstance()->removeRequest(activeRequests[idx].second);
// //             activeRequests.erase(activeRequests.begin() + idx);
// //         }
// //     }

// //     // Extract remaining values for validation
// //     std::vector<int32_t> remainingValues;
// //     for (const auto& pair : activeRequests) {
// //         remainingValues.push_back(pair.first);
// //     }

// //     std::sort(remainingValues.begin(), remainingValues.end(), std::greater<int32_t>());

// //     int32_t index = getCocoTableIndex(testOpId);
// //     CocoNode* head = getCocoTableInternal().at(index).at(1).second;

// //     for(int32_t val : remainingValues) {
// //         ASSERT_NE(head, nullptr);
// //         EXPECT_EQ(getCocoNodeResource(head)->mConfigValue.singleValue, val);
// //         head = getNext(head);
// //     }
// //     EXPECT_EQ(head, nullptr);

// //     for(const auto& pair : activeRequests) {
// //         Request::cleanUpRequest(pair.second);
// //     }
// // }

// TEST_F(CocoTableTest, TestCocoTableDefaultValueApplication) {
//     uint32_t testOpId = (1 << 31) | (1 << 16);
//     std::vector<Resource*>* resources =
//         new (GetBlock<std::vector<Resource*>>()) std::vector<Resource*>;

//     Resource* resource = (Resource*) (GetBlock<Resource>());
//     resource->mOpCode = testOpId;
//     resource->mOptionalInfo = 0;
//     resource->mNumValues = 1;
//     resource->mConfigValue.singleValue = 111;

//     resources->push_back(resource);

//     Request* request = new (GetBlock<Request>())
//                             Request(REQ_RESOURCE_TUNING, 1, -1, 1, 0, 1, 1, resources);

//     CocoTable::getInstance()->insertRequest(request);

//     EXPECT_EQ(readFromNode("../Tests/Configs/sched_util_clamp_min"),
//               std::to_string(resource->mConfigValue.singleValue));

//     CocoTable::getInstance()->removeRequest(request);

//     Request::cleanUpRequest(request);
// }

// // #if !defined(UNIT_TESTS_BUILD)
// // TEST(CocoTableTest, DifferentPriorityLevels,
// // "Objective: To verify that the priority is maintained while applying requests. ")
// // #else
// // TEST_F(CocoTableTest, DifferentPriorityLevels)
// // #endif
// // {
// //     vector<int32_t> values = {300, 500, 100, 200, 600, 700,  50};
// //     vector<int32_t> priorities  = {1,   0,   0,   1,   2,   3,    2};

// //     MakeAlloc<CocoNode>(values.size() + 1);
// //     MakeAlloc<Resource>(values.size() + 1);
// //     MakeAlloc<Request>(values.size() + 1);

// //     vector<Request*> requests;

// //     for (int32_t i = 0; i < values.size(); ++i) {
// //         vector<Resource*> resources;

// //         Resource* resource = (Resource*) (GetBlock<Resource>());
// //         resource->mOpCode = 0;
// //         resource->mNumValues = 1;
// //         resource->mConfigValue.singleValue = values[i];

// //         resources.push_back(resource);

// //         Request* request = new (GetBlock<Request>())
// //                                 Request(REQ_RESOURCE_TUNING, 1, -1, 1, priorities[i], 1, 1, resources);

// //         requests.push_back(request);
// //         cocoTable->insertRequest(requests[i]);
// //     }

// //     int32_t index = getCocoTableIndex(0);

// //     // Map of expected values per priority level
// //     unordered_map<int32_t, vector<int32_t>> expectedValuesByPriority = {
// //         {0, {500, 100}},
// //         {1, {300, 200}},
// //         {2, {600,  50}},
// //         {3, {700}}
// //     };

// //     // Validate each priority level
// //     for (const auto& [priority, expectedValues] : expectedValuesByPriority) {
// //         CocoNode* head = getCocoTable().at(index).at(priority).second;

// //         for (int32_t expectedValue : expectedValues) {
// //             ASSERT_NE(head, nullptr);
// //             EXPECT_EQ(getCocoNodeResource(head)->mConfigValue.singleValue, expectedValue);
// //             head = getNext(head);
// //         }

// //         EXPECT_EQ(head, nullptr);
// //     }

// //     EXPECT_EQ(getCurrentlyAppliedPriorityAtIndex(index), 0);

// //     //DELETE 700
// //     cocoTable->removeRequest(requests[5]);
// //     CocoNode* head = getCocoTable().at(index).at(3).second;
// //     ASSERT_EQ(head, nullptr);
// //     EXPECT_EQ(getCurrentlyAppliedPriorityAtIndex(index), 0);

// //     //DELETE 100
// //     cocoTable->removeRequest(requests[2]);
// //     //DELETE 500
// //     cocoTable->removeRequest(requests[1]);

// //     head = getCocoTable().at(index).at(0).second;
// //     ASSERT_EQ(head, nullptr);
// //     EXPECT_EQ(getCurrentlyAppliedPriorityAtIndex(index), 1);

// //     for (auto& req : requests) {
// //         Request::cleanUpRequest(req);
// //     }
// // }

// // //FIX AFTER INDEXING ISSUES ARE SORTED

// // // #if !defined(UNIT_TESTS_BUILD)
// // // TEST(CocoTableTest, DeletionDueToTimerOver,
// // // "Objective: To verify that requests are cleaned up when timer is over.",
// // // "Two requests with finite durations are given by the client. It is checked that when the timer ends the cocoTable maintains consistent state and the next request is honored. when the second request timer is over, the checks are made again")
// // // #else
// // // TEST_F(CocoTableTest, DeletionDueToTimerOver)
// // // #endif
// // // {
// // //     MakeAlloc<CocoNode>(30);
// // //     MakeAlloc<Resource>(30);
// // //     MakeAlloc<Request>(30);
// // //     MakeAlloc<ClientData> (30);
// // //     MakeAlloc<std::unordered_set<int64_t>> (30);

// // //     std::shared_ptr<ClientDataManager> clientDataManager = ClientDataManager::getInstance();
// // //     std::shared_ptr<RequestMap> requestMap = RequestManager::getInstance();

// // //     thread syslocksdServerThread(SyslocksdServerThread);
// // //     std::shared_ptr<RequestQueue> requestQueue = RequestQueue::getInstance();

// // //     vector<int32_t> values = {300,500,100,200};
// // //     vector<int32_t> time = {-1, 10000, -1, 15000};

// // //     vector<Request*> requests;

// // //     for(int32_t i = 0; i < values.size(); i++) {
// // //         vector<Resource*> resource;
// // //         Resource* r = (Resource*) (GetBlock<Resource>());
// // //         r->mOpCode = 0;
// // //         r->mNumValues = 1;
// // //         r->mConfigValue.singleValue = values[i];

// // //         resource.push_back(r);
// // //         requests.push_back(new (GetBlock<Request>())
// // //                                 Request(REQ_RESOURCE_TUNING, i+1, time[i], 1, 1, 1, 1, resource));

// // //         if(!clientDataManager->clientExists(requests.back()->getClientPID())) {
// // //             clientDataManager->createNewClient(requests.back()->getClientPID());
// // //         }

// // //         requestMap->addRequest(requests.back());
// // //         requestQueue->addAndWakeup(requests.back());
// // //     }

// // //     this_thread::sleep_for(chrono::seconds(1)); //Allow time for server side ops.
    
// // //     int32_t index = getCocoTableIndex(0);
// // //     CocoNode* head = getCocoTable().at(index).at(1).second;
 
// // //     sort(values.begin(), values.end(), greater<int32_t>());
    
// // //     while(head!=nullptr){
// // //         cout<<"hi"<<endl;
// // //         cout<<getCocoNodeResource(head)->mConfigValue.singleValue<<endl;
// // //         head = getNext(head);
// // //     }

// // //     for(int32_t i = 0; i < values.size(); i++) {
// // //         cout<<"hi"<<endl;
// // //         EXPECT_NE(head, nullptr);
// // //         EXPECT_EQ(getCocoNodeResource(head)->mConfigValue.singleValue, values[i]);
// // //         head = getNext(head);
// // //     }

// // //     EXPECT_EQ(head, nullptr);
 
// // //     this_thread::sleep_for(chrono::seconds(10)); //Delete 500 valued request.
// // //     values.erase(values.begin());

// // //     head = getCocoTable().at(index).at(1).second;
// // //     for(int32_t i = 0; i < values.size(); i++) {
// // //         ASSERT_NE(head, nullptr);
// // //         EXPECT_EQ(getCocoNodeResource(head)->mConfigValue.singleValue, values[i]);
// // //         head = getNext(head);
// // //     }
// // //     EXPECT_EQ(head, nullptr);
 
// // //     this_thread::sleep_for(chrono::seconds(6)); //Delete 200 valued request.
 
// // //     head = getCocoTable().at(index).at(1).second;
// // //     ASSERT_NE(head, nullptr);
// // //     EXPECT_EQ(getCocoNodeResource(head)->mConfigValue.singleValue, 300);
// // //     head = getNext(head);
 
// // //     ASSERT_NE(head, nullptr);
// // //     EXPECT_EQ(getCocoNodeResource(head)->mConfigValue.singleValue, 100);
// // //     head = getNext(head);
 
// // //     EXPECT_EQ(head, nullptr);
 
// // //     Request* exitReq = Request::createReleaseRequest();
// // //     requestQueue->addAndWakeup(exitReq);
// // //     syslocksdServerThread.join();
// // // }

// // // #if !defined(UNIT_TESTS_BUILD)
// // // TEST(CocoTableTest, DeletionDueToTimerOver,
// // // "Objective: To verify that a basic retune request is honored", 
// // // "A single request with finite duration is sent by the client. After some time, a retune request is made. It is checked that the original timer is not honored and the new one is.")
// // // #else
// // // TEST_F(CocoTableTest, BasicUpdation)
// // // #endif
// // // {
// // //     MakeAlloc<CocoNode>(3);
// // //     MakeAlloc<Resource>(3);
// // //     MakeAlloc<Request>(3);
// // //     MakeAlloc<ClientData> (3);                        
// // //     MakeAlloc<std::unordered_set<int64_t>> (3);
 
// // //     ClientDataManager* clientDataManager = ClientDataManager::getInstance();
 
// // //     thread syslocksdServerThread(SyslocksdServerThread);
// // //     RequestMap* requestMap = RequestManager::getInstance();
// // //     std::shared_ptr<RequestQueue> requestQueue = RequestQueue::getInstance();
 
   
// // //     vector<Resource*> resource;
// // //     Resource* r = (Resource*) (GetBlock<Resource>());
// // //     r->mOpCode = 0;
// // //     r->mNumValues = 1;
// // //     r->mConfigValue.singleValue = 300;
 
// // //     resource.push_back(r);
// // //     Request* req = new (GetBlock<Request>())
// // //                             Request(REQ_RESOURCE_TUNING, 1, 5000, 1, 1, 1, 1, resource);
 
// // //     if(!clientDataManager->clientExists(req->getClientPID())) {
// // //         clientDataManager->createNewClient(req->getClientPID());
// // //     }
 
// // //     requestMap->addRequest(req);
// // //     requestQueue->addAndWakeup(req);
   
 
// // //     this_thread::sleep_for(chrono::seconds(1)); //Allow time for server side ops.
   
// // //     int32_t index = getCocoTableIndex(0);
// // //     CocoNode* head = getCocoTable().at(index).at(1).second;
   
// // //     ASSERT_NE(head, nullptr);
// // //     EXPECT_EQ(getCocoNodeResource(head)->mConfigValue.singleValue, 300);
 
   

// // //     Request* retuneRequest = req->createRetuneRequest(8000);
// // //     requestQueue->addAndWakeup(retuneRequest);

// // //     this_thread::sleep_for(chrono::seconds(5)); //request shouldn't be deleted as of now.
    
// // //     ASSERT_NE(head, nullptr);
// // //     EXPECT_EQ(getCocoNodeResource(head)->mConfigValue.singleValue, 300);
 
// // //     this_thread::sleep_for(chrono::seconds(5)); //Delete request. shouldn't exist anymore.
    
// // //     head = getCocoTable().at(index).at(1).second;
// // //     EXPECT_EQ(head, nullptr);
 
 
// // //     Request* exitReq = Request::createReleaseRequest();
// // //     requestQueue->addAndWakeup(exitReq);
// // //     syslocksdServerThread.join();
// // // }
 