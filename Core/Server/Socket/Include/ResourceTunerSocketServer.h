// Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
// SPDX-License-Identifier: BSD-3-Clause-Clear

#ifndef RESOURCE_TUNER_SOCKET_SERVER_H
#define RESOURCE_TUNER_SOCKET_SERVER_H

#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <unordered_map>
#include <fcntl.h>
#include <string.h>

#include "MemoryPool.h"
#include "Request.h"
#include "Signal.h"
#include "SysConfig.h"
#include "SafeOps.h"
#include "ServerEndpoint.h"
#include "ErrCodes.h"
#include "Logger.h"

static const int32_t requestBufferSize = 1024;

class ResourceTunerSocketServer : public ServerEndpoint {
private:
    int32_t sockFd;
    uint32_t mListeningPort;
    ServerOnlineCheckCallback mServerOnlineCheckCb;
    ResourceTunerMessageReceivedCallback mResourceTunerMessageRecvCb;

public:
    ResourceTunerSocketServer(uint32_t mListeningPort,
                        ServerOnlineCheckCallback mServerOnlineCheckCb,
                        ResourceTunerMessageReceivedCallback mResourceTunerMessageRecvCb);

    ~ResourceTunerSocketServer();

    virtual int32_t ListenForClientRequests();
    virtual int32_t closeConnection();
};

#endif
