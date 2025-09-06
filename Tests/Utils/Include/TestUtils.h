// Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
// SPDX-License-Identifier: BSD-3-Clause-Clear

#ifndef TEST_UTILS_H
#define TEST_UTILS_H

#include <iostream>
#include <fstream>
#include <thread>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <fcntl.h>
#include <assert.h>

#include "AuxRoutines.h"

/*
* TEST RESOURCES DESCRIPTION:
* | Name                   | ResType | ResCode | Def Value | ApplyType | Enabled | Permissions   | High Threshold | Low Threshold |
* |------------------------|--------|--------|-----------|---------------------|---------------|----------------|---------------|
* | sched_util_clamp_min   |   01   |   00   |   300     |   global  |  True   | [third_party] |      1024      |      0        |
* | sched_util_clamp_max   |   01   |   01   |   684     |   global  |  True   | [third_party] |      1024      |      0        |
* | scaling_min_freq       |   01   |   02   |   107     |   global  |  True   | [third_party] |      1024      |      0        |
* | scaling_max_freq       |   01   |   03   |   114     |   global  |  True   | [third_party] |      2048      |      0        |
* | target_test_resource1  |   01   |   04   |   240     |   global  |  True   | [system]      |      400       |      0        |
* | target_test_resource2  |   01   |   05   |   333     |   global  |  True   | [third_party] |      6500      |      50       |
* | target_test_resource3  |   01   |   06   |   4400    |   global  |  True   | [third_party] |      5511      |      4000     |
* | target_test_resource4  |   01   |   07   |   516     |   global  |  False  | [third_party] |      900       |      300      |
* | target_test_resource5  |   01   |   08   |   17      |   global  |  True   | [third_party] |      20        |      0        |
* |------------------------|--------|--------|-----------|---------------------|---------------|----------------|---------------|
*/

void SetUp() {
    // Make sure all the tests have a sane starting point

    AuxRoutines::writeToFile("/etc/resource-tuner/tests/Configs/ResourceSysFsNodes/sched_util_clamp_min", "300");
    AuxRoutines::writeToFile("/etc/resource-tuner/tests/Configs/ResourceSysFsNodes/sched_util_clamp_max", "684");
    AuxRoutines::writeToFile("/etc/resource-tuner/tests/Configs/ResourceSysFsNodes/scaling_min_freq", "107");
    AuxRoutines::writeToFile("/etc/resource-tuner/tests/Configs/ResourceSysFsNodes/scaling_max_freq", "114");
    AuxRoutines::writeToFile("/etc/resource-tuner/tests/Configs/ResourceSysFsNodes/target_test_resource1", "240");
    AuxRoutines::writeToFile("/etc/resource-tuner/tests/Configs/ResourceSysFsNodes/target_test_resource2", "333");
    AuxRoutines::writeToFile("/etc/resource-tuner/tests/Configs/ResourceSysFsNodes/target_test_resource3", "4400");
    AuxRoutines::writeToFile("/etc/resource-tuner/tests/Configs/ResourceSysFsNodes/target_test_resource4", "516");
    AuxRoutines::writeToFile("/etc/resource-tuner/tests/Configs/ResourceSysFsNodes/target_test_resource5", "17");

    std::this_thread::sleep_for(std::chrono::seconds(3));
}

#define RUN_INTEGRATION_TEST(test)  \
do {                                \
    SetUp();                        \
    test();                         \
} while(false);                     \

#define C_STOI(value) ({                                                        \
    int32_t parsedValue = -1;                                                   \
    try {                                                                       \
        parsedValue = (int32_t)std::stoi(value);                                \
    } catch(const std::invalid_argument& ex) {                                  \
        std::cerr<<"std::invalid_argument::what(): " << ex.what()<<std::endl;   \
    } catch(const std::out_of_range& ex) {                                      \
        std::cerr<<"std::out_of_range::what(): " << ex.what()<<std::endl;       \
    }                                                                           \
    parsedValue;                                                                \
});

#define GENERATE_RESOURCE_ID(resType, resCode) ({                               \
    uint32_t resourceBitmap = 0;                                                \
    resourceBitmap |= (1 << 31);                                                \
    resourceBitmap |= ((uint32_t)resCode);                                      \
    resourceBitmap |= ((uint32_t)resType << 16);                                \
    resourceBitmap;                                                             \
})

#define LOG_START std::cout<<"\nRunning Test: "<<__func__<<std::endl;
#define LOG_END std::cout<<__func__<<": Run Successful"<<std::endl;

#define RUN_TEST(test)                                              \
do {                                                                \
    std::cout<<"Running Test: "<<#test<<std::endl;                  \
    test();                                                         \
    std::cout<<#test<<": Run Successful"<<std::endl;                \
    std::cout<<"-------------------------------------"<<std::endl;  \
} while(false);                                                     \

#define C_ASSERT(cond)                                                                          \
    if(cond == false) {                                                                         \
        std::cerr<<"Assertion failed at line [" << __LINE__ << "]: "<<#cond<<std::endl;         \
        std::cerr<<"Test: ["<<__func__<<"] Failed, Terminating Suite\n"<<std::endl;             \
        exit(EXIT_FAILURE);                                                                     \
    }                                                                                           \

#define C_ASSERT_NEAR(val1, val2, tol)                                               \
    if (std::fabs((val1) - (val2)) > (tol)) {                                        \
        std::cerr<<"Condition Check on line:["<<__LINE__<<"]  failed"<<std::endl;    \
        std::cerr<<"Test: ["<<__func__<<"] Failed, Terminating Suite\n"<<std::endl;  \
        exit(EXIT_FAILURE);                                                          \
    }                                                                                \

#endif
