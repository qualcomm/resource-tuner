// Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
// SPDX-License-Identifier: BSD-3-Clause-Clear

#ifndef RESOURCE_TUNER_SOCKET_CLIENT_H
#define RESOURCE_TUNER_SOCKET_CLIENT_H

#include <iostream>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#include "Request.h"
#include "Signal.h"
#include "SafeOps.h"
#include "Utils.h"
#include "ClientEndpoint.h"
#include "ErrCodes.h"

static const int32_t socketConnPort = 12000;

class ResourceTunerSocketClient : public ClientEndpoint {
private:
    int32_t sockFd;

public:
    ResourceTunerSocketClient();
    ~ResourceTunerSocketClient();

    virtual int32_t initiateConnection();
    virtual int32_t sendMsg(char* buf, size_t bufSize);
    virtual int32_t readMsg(char* buf, size_t bufSize);
    virtual int32_t closeConnection();
};

#endif
