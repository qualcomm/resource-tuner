// Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
// SPDX-License-Identifier: BSD-3-Clause-Clear

#ifndef SOCKET_CLIENT_H
#define SOCKET_CLIENT_H

#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <cstring>

#include "Request.h"
#include "Signal.h"
#include "SafeOps.h"
#include "Utils.h"
#include "ClientEndpoint.h"
#include "ErrCodes.h"
#include "ResourceTunerAPIs.h"

static const int32_t socketConnPort = 12000;

class SocketClient {
private:
    int32_t sockFd;

public:
    SocketClient();
    ~SocketClient();

    int32_t initiateConnection();
    int32_t closeConnection();
    int32_t sendMsg(char* buf, size_t bufSize);
    int32_t readMsg(char* buf, size_t bufSize);
};

#endif
