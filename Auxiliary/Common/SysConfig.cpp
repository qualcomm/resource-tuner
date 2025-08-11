// Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
// SPDX-License-Identifier: BSD-3-Clause-Clear

#include "SysConfig.h"

int8_t SysConfig::getRequestType() {
    return this->mReqType;
}

const std::string SysConfig::getProp() {
    return this->mProp;
}

const std::string SysConfig::getValue() {
    return this->mValue;
}

const std::string SysConfig::getDefaultValue() {
    return this->mDefValue;
}

int32_t SysConfig::getClientPID() {
    return this->mClientPID;
}

int32_t SysConfig::getClientTID() {
    return this->mClientTID;
}

uint64_t SysConfig::getBufferSize() {
    return this->mBufferSize;
}

void SysConfig::setRequestType(int8_t reqType) {
    this->mReqType = reqType;
}

void SysConfig::setProp(const std::string& prop) {
    this->mProp = prop;
}

void SysConfig::setValue(const std::string& value) {
    this->mValue = value;
}

void SysConfig::setDefaultValue(const std::string& defValue) {
    this->mDefValue = defValue;
}

void SysConfig::setClientPID(int32_t clientPID) {
    this->mClientPID = clientPID;
}

void SysConfig::setClientTID(int32_t clientTID) {
    this->mClientTID = clientTID;
}

void SysConfig::setBufferSize(uint64_t bufferSize) {
    this->mBufferSize = bufferSize;
}

ErrCode SysConfig::serialize(char* buf) {
    try {
        int8_t* ptr8 = (int8_t*)buf;
        ASSIGN_AND_INCR(ptr8, this->getRequestType());

        char* charPointer = (char*) ptr8;
        for(char ch: this->getProp()) {
            ASSIGN_AND_INCR(charPointer, ch);
        }

        ASSIGN_AND_INCR(charPointer, '\0');

        for(char ch: this->getValue()) {
            ASSIGN_AND_INCR(charPointer, ch);
        }

        ASSIGN_AND_INCR(charPointer, '\0');

        for(char ch: this->getDefaultValue()) {
            ASSIGN_AND_INCR(charPointer, ch);
        }

        ASSIGN_AND_INCR(charPointer, '\0');

        int32_t* ptr = (int32_t*)charPointer;

        ASSIGN_AND_INCR(ptr, this->getClientPID());
        ASSIGN_AND_INCR(ptr, this->getClientTID());

        uint64_t* ptr64 = (uint64_t*)ptr;
        ASSIGN_AND_INCR(ptr64, this->getBufferSize());

    } catch(const std::bad_alloc& e) {
        return RC_REQUEST_PARSING_FAILED;

    } catch(const std::exception& e) {
        return RC_INVALID_VALUE;
    }

    return RC_SUCCESS;
}

ErrCode SysConfig::deserialize(char* buf) {
    try {
        int8_t* ptr8 = (int8_t*)buf;
        this->mReqType = DEREF_AND_INCR(ptr8, int8_t);

        char* charIterator = (char*)ptr8;
        this->mProp = charIterator;

        while(*charIterator != '\0') {
            charIterator++;
        }
        charIterator++;

        this->mValue = charIterator;

        while(*charIterator != '\0') {
            charIterator++;
        }
        charIterator++;

        this->mDefValue = charIterator;

        while(*charIterator != '\0') {
            charIterator++;
        }
        charIterator++;

        int32_t* ptr = (int32_t*)charIterator;
        this->mClientPID = DEREF_AND_INCR(ptr, int32_t);
        this->mClientTID = DEREF_AND_INCR(ptr, int32_t);

        uint64_t* ptr64 = (uint64_t*)ptr;
        this->mBufferSize = DEREF_AND_INCR(ptr64, uint64_t);

    } catch(const std::invalid_argument& e) {
        TYPELOGV(REQUEST_PARSING_FAILURE, e.what());

        return RC_REQUEST_PARSING_FAILED;

    } catch(const std::bad_alloc& e) {
        TYPELOGV(REQUEST_MEMORY_ALLOCATION_FAILURE, e.what());
        return RC_MEMORY_POOL_BLOCK_RETRIEVAL_FAILURE;

    } catch(const std::exception& e) {
        LOGE("URM_SYSTUNE_SERVER",
             "Request Deserialization Failed with error: " + std::string(e.what()));
        return RC_REQUEST_DESERIALIZATION_FAILURE;
    }

    return RC_SUCCESS;
}
