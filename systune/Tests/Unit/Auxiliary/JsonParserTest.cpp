// Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
// SPDX-License-Identifier: BSD-3-Clause-Clear

#include <gtest/gtest.h>
#include <string>

#include "JsonParser.h"

class JsonParserTest: public::testing::Test {
    protected:
        JsonParser* jsonParser;

        void SetUp() override {
            jsonParser = new (std::nothrow) JsonParser();
            if(jsonParser == nullptr) {
                throw std::bad_alloc();
            }
        }
};

TEST_F(JsonParserTest, EmptyFileJSON) {
    const std::string rootName("ResourceConfigs");
    int32_t size;
    ErrCode rc = jsonParser->verifyAndGetSize("../Tests/Configs/EmptyFile.json", rootName, size);
    EXPECT_EQ(rc, RC_INVALID_JSON);
}

TEST_F(JsonParserTest, EmptyObjectJSON) {
    const std::string rootName("ResourceConfigs");
    int32_t size;
    ErrCode rc = jsonParser->verifyAndGetSize("../Tests/Configs/EmptyObject.json", rootName, size);
    EXPECT_EQ(rc, RC_INVALID_JSON);
}

TEST_F(JsonParserTest, MultipleRootsJSON) {
    const std::string rootName("ResourceConfigs");
    int32_t size;
    ErrCode rc = jsonParser->verifyAndGetSize("../Tests/Configs/MultipleRoots.json", rootName, size);
    EXPECT_EQ(rc, RC_INVALID_JSON);
}

TEST_F(JsonParserTest, InvalidFileJSON) {
    const std::string rootName("ResourceConfigs");
    int32_t size;
    ErrCode rc = jsonParser->verifyAndGetSize("../Tests/Configs/InvalidFile.json", rootName, size);
    EXPECT_EQ(rc, RC_INVALID_JSON);
}
