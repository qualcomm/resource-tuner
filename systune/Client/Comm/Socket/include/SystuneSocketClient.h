// Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
// SPDX-License-Identifier: BSD-3-Clause-Clear

#ifndef SYSTUNE_SOCKET_CLIENT_H
#define SYSTUNE_SOCKET_CLIENT_H

#include <iostream>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#include "Request.h"
#include "Signal.h"
#include "SysConfig.h"
#include "Types.h"
#include "SafeOps.h"
#include "Utils.h"
#include "ClientEndpoint.h"
#include "ErrCodes.h"

class SystuneSocketClient : public ClientEndpoint {
private:
    int32_t sockFd;

public:
    SystuneSocketClient();
    ~SystuneSocketClient();

    virtual int32_t initiateConnection();
    virtual int32_t sendMsg(int32_t reqType, void* msg);
    virtual int32_t readMsg(char* buf, size_t bufSize);
    virtual int32_t closeConnection();
};

#endif
