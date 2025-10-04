// Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
// SPDX-License-Identifier: BSD-3-Clause-Clear

#ifndef SOCKET_SERVER_H
#define SOCKET_SERVER_H

#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/epoll.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <fcntl.h>
#include <string.h>

#include "MemoryPool.h"
#include "Request.h"
#include "Signal.h"
#include "SafeOps.h"
#include "ComponentRegistry.h"
#include "RequestReceiver.h"
#include "ResourceTunerSettings.h"
#include "ErrCodes.h"
#include "Logger.h"

static const uint32_t maxEvents = 128;

class SocketServer {
private:
    int32_t sockFd;
    uint32_t mListeningPort;

public:
    SocketServer(uint32_t mListeningPort);
    ~SocketServer();

    virtual int32_t ListenForClientRequests();
    virtual int32_t closeConnection();
};

#endif
