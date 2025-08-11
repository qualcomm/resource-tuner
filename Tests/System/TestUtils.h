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

/*
* TEST RESOURCES DESCRIPTION:
* | Name                   | Optype | Opcode | Def Value | Core Level Conflict | Enabled | Permissions   | High Threshold | Low Threshold |
* |------------------------|--------|--------|-----------|---------------------|---------|---------------|----------------|---------------|
* | sched_util_clamp_min   |   01   |   00   |   300     |        False        |  True   | [third_party] |      1024      |      0        |
* | sched_util_clamp_max   |   01   |   01   |   684     |        False        |  True   | [third_party] |      1024      |      0        |
* | scaling_min_freq       |   01   |   02   |   107     |        False        |  True   | [third_party] |      1024      |      0        |
* | scaling_max_freq       |   01   |   03   |   114     |        False        |  True   | [third_party] |      2048      |      0        |
* | target_test_resource1  |   01   |   04   |   240     |        False        |  True   | [system]      |      400       |      0        |
* | target_test_resource2  |   01   |   05   |   333     |        True         |  True   | [third_party] |      6500      |      50       |
* | target_test_resource3  |   01   |   06   |   4400    |        False        |  True   | [third_party] |      5511      |      4000     |
* | target_test_resource4  |   01   |   07   |   516     |        False        |  False  | [third_party] |      900       |      300      |
* | target_test_resource5  |   01   |   08   |   17      |        False        |  True   | [third_party] |      20        |      0        |
* |------------------------|--------|--------|-----------|---------------------|---------|---------------|----------------|---------------|
*/

static void writeToNode(const std::string& fName, std::string value) {
    std::ofstream file(fName);
    if(file.is_open()) {
        file << value;
        file.close();
    } else {
        std::cerr<<"Unable to open file"<<std::endl;
    }
}

static std::string readFromNode(const std::string& fName) {
    std::fstream myFile(fName, std::ios::in);
    std::string value;

    if(myFile.is_open()) {
        getline(myFile, value);
        myFile.close();
    } else {
        std::cerr<<"Failed to open the file."<<std::endl;
        return "";
    }

    return value;
}

void SetUp() {
    // Make sure all the tests have a sane starting point

    writeToNode("../Tests/Configs/ResourceSysFsNodes/sched_util_clamp_min", "300");
    writeToNode("../Tests/Configs/ResourceSysFsNodes/sched_util_clamp_max", "684");
    writeToNode("../Tests/Configs/ResourceSysFsNodes/scaling_min_freq", "107");
    writeToNode("../Tests/Configs/ResourceSysFsNodes/scaling_max_freq", "114");
    writeToNode("../Tests/Configs/ResourceSysFsNodes/target_test_resource1", "240");
    writeToNode("../Tests/Configs/ResourceSysFsNodes/target_test_resource2", "333");
    writeToNode("../Tests/Configs/ResourceSysFsNodes/target_test_resource3", "4400");
    writeToNode("../Tests/Configs/ResourceSysFsNodes/target_test_resource4", "516");
    writeToNode("../Tests/Configs/ResourceSysFsNodes/target_test_resource5", "17");

    std::this_thread::sleep_for(std::chrono::seconds(3));
}

#define RUN_TEST(test)  \
do {                    \
    SetUp();            \
    test();             \
} while(false);         \

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

#define GENERATE_RESOURCE_ID(optype, opcode) ({                                \
    uint32_t resourceBitmap = 0;                                               \
    resourceBitmap |= (1 << 31);                                               \
    resourceBitmap |= ((uint32_t)opcode);                                      \
    resourceBitmap |= ((uint32_t)optype << 16);                                \
    resourceBitmap;                                                            \
})

#define LOG_START std::cout<<"\nRunning Test: "<<__func__<<std::endl;
#define LOG_END std::cout<<__func__<<": Run Successful"<<std::endl;

#endif
