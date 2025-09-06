// Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
// SPDX-License-Identifier: BSD-3-Clause-Clear

#include "TestUtils.h"
#include "CocoTable.h"

static void TestCocoTableInsertRequest1() {
    C_ASSERT(CocoTable::getInstance()->insertRequest(nullptr) == false);
}

static void TestCocoTableInsertRequest2() {
    Request* request = new Request;
    C_ASSERT(CocoTable::getInstance()->insertRequest(request) == false);
    delete request;
}

static void TestCocoTableInsertRequest3() {
    Request* request = new Request;
    MakeAlloc<std::vector<CocoNode*>>(1);
    C_ASSERT(CocoTable::getInstance()->insertRequest(request) == false);
    delete request;
}

static void TestCocoTableInsertRequest4() {
    Request* request = new Request;
    MakeAlloc<std::vector<CocoNode*>>(1);
    MakeAlloc<CocoNode>(1);

    request->setNumResources(1);
    std::vector<Resource*>* resources = new std::vector<Resource*>;
    resources->push_back(nullptr);
    request->setResources(resources);

    C_ASSERT(CocoTable::getInstance()->insertRequest(request) == false);
    delete resources;
    delete request;
}

static void TestCocoTableInsertRequest5() {
    Request* request = new Request;
    MakeAlloc<std::vector<CocoNode*>>(1);
    MakeAlloc<CocoNode>(1);
    MakeAlloc<Timer>(1);

    request->setNumResources(1);
    std::vector<Resource*>* resources = new std::vector<Resource*>;
    resources->push_back(nullptr);
    request->setResources(resources);

    C_ASSERT(CocoTable::getInstance()->insertRequest(request) == false);
    delete resources;
    delete request;
}

int32_t main() {
     std::cout<<"Running Test Suite: [CocoTableTests]\n"<<std::endl;

    RUN_TEST(TestCocoTableInsertRequest1);
    RUN_TEST(TestCocoTableInsertRequest2);
    RUN_TEST(TestCocoTableInsertRequest3);
    RUN_TEST(TestCocoTableInsertRequest4);
    RUN_TEST(TestCocoTableInsertRequest5);

    std::cout<<"\nAll Tests from the suite: [CocoTableTests], executed successfully"<<std::endl;
}
