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
    // MakeAlloc<std::vector<CocoNode*>>(1);
    C_ASSERT(CocoTable::getInstance()->insertRequest(request) == false);
    delete request;
}

static void RunTests()  {
    std::cout<<"Running Test Suite: [CocoTableTests]\n"<<std::endl;

    RUN_TEST(TestCocoTableInsertRequest1);

    std::cout<<"\nAll Tests from the suite: [CocoTableTests], executed successfully"<<std::endl;
}

REGISTER_TEST(RunTests);
