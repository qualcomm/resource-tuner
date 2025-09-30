// Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
// SPDX-License-Identifier: BSD-3-Clause-Clear

#include "SocketClient.h"

SocketClient::SocketClient() {
    this->sockFd = -1;
}

int32_t SocketClient::initiateConnection() {
    if((this->sockFd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        TYPELOGV(ERRNO_LOG, "socket", strerror(errno));
        return RC_SOCKET_CONN_NOT_INITIALIZED;
    }

    sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(socketConnPort);
    addr.sin_addr.s_addr = htonl(INADDR_ANY);

    if(connect(this->sockFd, (const sockaddr*)&addr, sizeof(addr)) < 0) {
        TYPELOGV(ERRNO_LOG, "connect", strerror(errno));
        return RC_SOCKET_CONN_NOT_INITIALIZED;
    }

    return RC_SUCCESS;
}

int32_t SocketClient::sendMsg(char* buf, size_t bufSize) {
    if(buf == nullptr) return RC_BAD_ARG;

    if(write(this->sockFd, buf, bufSize) == -1) {
        TYPELOGV(ERRNO_LOG, "write", strerror(errno));
        return RC_SOCKET_FD_WRITE_FAILURE;
    }

    return RC_SUCCESS;
}

int32_t SocketClient::readMsg(char* buf, size_t bufSize) {
    if(buf == nullptr || bufSize == 0) {
        return RC_BAD_ARG;
    }
    int32_t statusCode;

    if((statusCode = read(this->sockFd, buf, bufSize)) < 0) {
        TYPELOGV(ERRNO_LOG, "read", strerror(errno));
        return RC_SOCKET_FD_READ_FAILURE;
    }

    return RC_SUCCESS;
}

int32_t SocketClient::closeConnection() {
    if(this->sockFd != -1) {
        return close(this->sockFd);
    }
    return RC_SOCKET_FD_CLOSE_FAILURE;
}

SocketClient::~SocketClient() {
    if(this->sockFd != -1) {
        close(this->sockFd);
    }
    this->sockFd = -1;
}
