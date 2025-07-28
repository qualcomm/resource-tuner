// Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
// SPDX-License-Identifier: BSD-3-Clause-Clear

#include "SystuneSocketClient.h"

#define ASSIGN_AND_INCR(ptr, val) \
    SafeAssignment(ptr, val);     \
    ptr++;                        \

// Helper methods for Serializing the Request struct to a byte-array
static ErrCode SerializeToByteArray(Request* request, char* buf) {
    try {
        int8_t* ptr8 = (int8_t*)buf;
        ASSIGN_AND_INCR(ptr8, request->getRequestType());

        int64_t* ptr64 = (int64_t*)ptr8;
        ASSIGN_AND_INCR(ptr64, request->getHandle());

        ASSIGN_AND_INCR(ptr64, request->getDuration());

        int32_t* ptr = (int32_t*)ptr64;
        ASSIGN_AND_INCR(ptr, request->getResourcesCount());
        ASSIGN_AND_INCR(ptr, request->getPriority());
        ASSIGN_AND_INCR(ptr, request->getClientPID());
        ASSIGN_AND_INCR(ptr, request->getClientTID());

        ptr8 = (int8_t*)ptr;
        ASSIGN_AND_INCR(ptr8, request->isBackgroundProcessingEnabled());

        ptr = (int32_t*)ptr8;
        for(int32_t i = 0; i < request->getResourcesCount(); i++) {
            Resource* resource = request->getResourceAt(i);
            if(resource == nullptr) {
                return RC_INVALID_VALUE;
            }

            ASSIGN_AND_INCR(ptr, resource->mOpId);
            ASSIGN_AND_INCR(ptr, resource->mOpInfo);
            ASSIGN_AND_INCR(ptr, resource->mOptionalInfo);
            ASSIGN_AND_INCR(ptr, resource->mNumValues);

            if(resource->mNumValues == 1) {
                ASSIGN_AND_INCR(ptr, resource->mConfigValue.singleValue);
            } else {
                for(int32_t j = 0; j < resource->mNumValues; j++) {
                    ASSIGN_AND_INCR(ptr, (*resource->mConfigValue.valueArray)[j]);
                }
            }
        }
    } catch(const std::invalid_argument& e) {
        return RC_REQUEST_PARSING_FAILED;
    } catch(const std::exception& e) {
        return RC_INVALID_VALUE;
    }

    return RC_SUCCESS;
}

static ErrCode SerializeToByteArray(SysConfig* sysConfig, char* buf) {
    try {
        int8_t* ptr8 = (int8_t*)buf;
        ASSIGN_AND_INCR(ptr8, sysConfig->getRequestType());

        const char* charIterator = sysConfig->getProp();
        char* charPointer = (char*) ptr8;

        while(*charIterator != '\0') {
            ASSIGN_AND_INCR(charPointer, *charIterator);
            charIterator++;
        }

        ASSIGN_AND_INCR(charPointer, '\0');

        charIterator = sysConfig->getValue();

        while(*charIterator != '\0') {
            ASSIGN_AND_INCR(charPointer, *charIterator);
            charIterator++;
        }

        ASSIGN_AND_INCR(charPointer, '\0');

        charIterator = sysConfig->getDefaultValue();

        while(*charIterator != '\0') {
            ASSIGN_AND_INCR(charPointer, *charIterator);
            charIterator++;
        }

        ASSIGN_AND_INCR(charPointer, '\0');

        int32_t* ptr = (int32_t*)charPointer;

        ASSIGN_AND_INCR(ptr, sysConfig->getClientPID());
        ASSIGN_AND_INCR(ptr, sysConfig->getClientTID());

        uint64_t* ptr64 = (uint64_t*)ptr;
        ASSIGN_AND_INCR(ptr64, sysConfig->getBufferSize());

    } catch(const std::bad_alloc& e) {
        return RC_REQUEST_PARSING_FAILED;
    } catch(const std::exception& e) {
        return RC_INVALID_VALUE;
    }

    return RC_SUCCESS;
}

static int8_t SerializeToByteArray(Signal* signal, char* buf) {
    try {
        int8_t* ptr8 = (int8_t*)buf;
        ASSIGN_AND_INCR(ptr8, signal->getRequestType());

        int32_t* ptr = (int32_t*)ptr8;
        ASSIGN_AND_INCR(ptr, signal->getSignalID());

        int64_t* ptr64 = (int64_t*)ptr;
        ASSIGN_AND_INCR(ptr64, signal->getHandle());

        ASSIGN_AND_INCR(ptr64, signal->getDuration());

        const char* charIterator = signal->getAppName();
        char* charPointer = (char*) ptr64;

        while(*charIterator != '\0') {
            ASSIGN_AND_INCR(charPointer, *charIterator);
            charIterator++;
        }

        ASSIGN_AND_INCR(charPointer, '\0');

        charIterator = signal->getScenario();

        while(*charIterator != '\0') {
            ASSIGN_AND_INCR(charPointer, *charIterator);
            charIterator++;
        }

        ASSIGN_AND_INCR(charPointer, '\0');

        ptr = (int32_t*)charPointer;
        ASSIGN_AND_INCR(ptr, signal->getNumArgs());
        ASSIGN_AND_INCR(ptr, signal->getPriority());
        ASSIGN_AND_INCR(ptr, signal->getClientPID());
        ASSIGN_AND_INCR(ptr, signal->getClientTID());

        for(int32_t i = 0; i < signal->getNumArgs(); i++) {
            uint32_t arg = signal->getListArgAt(i);
            ASSIGN_AND_INCR(ptr, arg)
        }
    } catch(const std::bad_alloc& e) {
        return RC_REQUEST_PARSING_FAILED;
    } catch(const std::exception& e) {
        return RC_INVALID_VALUE;
    }

    return RC_SUCCESS;
}

SystuneSocketClient::SystuneSocketClient() {
    this->sockFd = -1;
}

int32_t SystuneSocketClient::initiateConnection() {
    if((this->sockFd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("socket");
        return RC_SOCKET_CONN_NOT_INITIALIZED;
    }

    sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(12000);
    addr.sin_addr.s_addr = htonl(INADDR_ANY);

    if(connect(this->sockFd, (const sockaddr*)&addr, sizeof(addr)) < 0) {
        perror("connect");
        return RC_SOCKET_CONN_NOT_INITIALIZED;
    }

    return RC_SUCCESS;
}

int32_t SystuneSocketClient::sendMsg(int32_t reqType, void* msg) {
    if(msg == nullptr) return RC_BAD_ARG;

    // byte Array to Store the serialized output
    // This buffer of 1KB can accomodate around 38 Resources
    char buf[1024];

    int8_t opStatus = RC_SUCCESS;

    switch(reqType) {
        case REQ_RESOURCE_TUNING:
        case REQ_RESOURCE_RETUNING:
        case REQ_RESOURCE_UNTUNING:
        case REQ_CLIENT_GET_REQUESTS:
            opStatus = SerializeToByteArray(static_cast <Request*>(msg), buf);
            break;
        case REQ_SYSCONFIG_GET_PROP:
        case REQ_SYSCONFIG_SET_PROP:
            opStatus = SerializeToByteArray(static_cast <SysConfig*>(msg), buf);
            break;
        case SIGNAL_ACQ:
        case SIGNAL_FREE:
        case SIGNAL_RELAY:
            opStatus = SerializeToByteArray(static_cast <Signal*>(msg), buf);
            break;
        default:
            opStatus = RC_INVALID_VALUE;
            break;
    }

    if(RC_IS_OK(opStatus)) {
        if(write(this->sockFd, buf, sizeof(buf)) == -1) {
            perror("write");
            return RC_SOCKET_FD_WRITE_FAILURE;
        }
    }

    return opStatus;
}

int32_t SystuneSocketClient::readMsg(char* buf, size_t bufSize) {
    if(buf == nullptr || bufSize == 0) {
        return RC_BAD_ARG;
    }
    int32_t statusCode;

    if((statusCode = read(this->sockFd, buf, bufSize)) < 0) {
        perror("read");
        return RC_SOCKET_FD_READ_FAILURE;
    }

    return RC_SUCCESS;
}

int32_t SystuneSocketClient::closeConnection() {
    if(this->sockFd != -1) {
        return close(this->sockFd);
    }
    return RC_SOCKET_FD_CLOSE_FAILURE;
}

SystuneSocketClient::~SystuneSocketClient() {
    if(this->sockFd != -1) {
        close(this->sockFd);
    }
    this->sockFd = -1;
}
