// Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
// SPDX-License-Identifier: BSD-3-Clause-Clear

#include "SystuneSocketClient.h"

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
    addr.sin_port = htons(socketConnPort);
    addr.sin_addr.s_addr = htonl(INADDR_ANY);

    if(connect(this->sockFd, (const sockaddr*)&addr, sizeof(addr)) < 0) {
        // Use Logger = perror("connect");
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
        case REQ_CLIENT_GET_REQUESTS: {
            Request* request = static_cast <Request*>(msg);
            opStatus = request->serialize(buf);
            break;
        }
        case REQ_SYSCONFIG_GET_PROP:
        case REQ_SYSCONFIG_SET_PROP: {
            SysConfig* sysConfig = static_cast <SysConfig*>(msg);
            opStatus = sysConfig->serialize(buf);
            break;
        }
        case SIGNAL_ACQ:
        case SIGNAL_FREE:
        case SIGNAL_RELAY: {
            Signal* signal = static_cast <Signal*>(msg);
            opStatus = signal->serialize(buf);
            break;
        }
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
