// Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
// SPDX-License-Identifier: BSD-3-Clause-Clear
#include <sys/epoll.h>
#include "ResourceTunerSocketServer.h"

ResourceTunerSocketServer::ResourceTunerSocketServer(uint32_t mListeningPort,
                                         ServerOnlineCheckCallback mServerOnlineCheckCb,
                                         ResourceTunerMessageReceivedCallback mResourceTunerMessageRecvCb) {

    this->mListeningPort = mListeningPort;
    this->mServerOnlineCheckCb = mServerOnlineCheckCb;
    this->mResourceTunerMessageRecvCb = mResourceTunerMessageRecvCb;
}

// Called by server, this will put the server in listening mode
int32_t ResourceTunerSocketServer::ListenForClientRequests() {
    if((sockFd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        TYPELOGV(ERRNO_LOG, "socket", strerror(errno));
        LOGE("RESTUNE_SOCKET_SERVER", "Failed to initialize Server Socket");
        return RC_SOCKET_CONN_NOT_INITIALIZED;
    }

    // Make the socket Non-Blocking
    fcntl(sockFd, F_SETFL, O_NONBLOCK);

    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(this->mListeningPort);
    addr.sin_addr.s_addr = htonl(INADDR_ANY);

    int epoll_fd = epoll_create1(0);
    epoll_event event{}, events[maxEvents];
    event.events = EPOLLIN;
    event.data.fd = sockFd;
    epoll_ctl(epoll_fd, EPOLL_CTL_ADD, sockFd, &event);

    // Reuse address. Useful when the server is restarted right away. The OS takes time to clean up the socket.
    // The socket spends the longest of that cleanup state in TIME_WAIT section. This option allows the socket
    // to reuse that same address when server is in TIME_WAIT state.
    int32_t reuse = 1;
    if(setsockopt(sockFd, SOL_SOCKET, SO_REUSEADDR, (const char*)&reuse, sizeof(reuse)) < 0) {
        TYPELOGV(ERRNO_LOG, "setsockopt", strerror(errno));
        LOGE("RESTUNE_SOCKET_SERVER", "Failed to initialize Server Socket");
        return RC_SOCKET_CONN_NOT_INITIALIZED;
    }

    if(bind(sockFd, (const sockaddr*)&addr, sizeof(addr)) < 0) {
        TYPELOGV(ERRNO_LOG, "bind", strerror(errno));
        LOGE("RESTUNE_SOCKET_SERVER", "Failed to initialize Server Socket");
        return RC_SOCKET_CONN_NOT_INITIALIZED;
    }

    if(listen(sockFd, maxEvents) < 0) {
        TYPELOGV(ERRNO_LOG, "listen", strerror(errno));
        LOGE("RESTUNE_SOCKET_SERVER", "Failed to initialize Server Socket");
        return RC_SOCKET_CONN_NOT_INITIALIZED;
    }

    while(this->mServerOnlineCheckCb()) {
        int32_t clientSocket = -1;

        int n = epoll_wait(epoll_fd, events, maxEvents, -1);
        for (int i = 0; i < n; ++i) {
            if (events[i].data.fd == sockFd) {
                    sockaddr_in client_addr{};
                    socklen_t client_len = sizeof(client_addr);

                    if((clientSocket = accept(sockFd, nullptr, nullptr)) < 0) {
                       if(errno != EAGAIN && errno != EWOULDBLOCK) {
                            TYPELOGV(ERRNO_LOG, "accept", strerror(errno));
                            LOGE("RESTUNE_SOCKET_SERVER", "Server Socket-Endpoint crashed");
                            return RC_SOCKET_OP_FAILURE;
                    }
                }
            }
        }

        if(clientSocket >= 0) {
            MsgForwardInfo* info = new MsgForwardInfo;
            info->buffer = new char[requestBufferSize];
            info->bufferSize = requestBufferSize;

            int32_t bytesRead;
            if((bytesRead = recv(clientSocket, info->buffer, info->bufferSize, 0)) < 0) {
                if(errno != EAGAIN && errno != EWOULDBLOCK) {
                    TYPELOGV(ERRNO_LOG, "recv", strerror(errno));
                    LOGE("RESTUNE_SOCKET_SERVER", "Server Socket-Endpoint crashed");
                    return RC_SOCKET_OP_FAILURE;
                }
            }

            if(bytesRead > 0) {
                this->mResourceTunerMessageRecvCb(clientSocket, info);
            }
            close(clientSocket);
        }
    }

    return RC_SUCCESS;
}

int32_t ResourceTunerSocketServer::closeConnection() {
    if(this->sockFd != -1) {
        return close(this->sockFd);
    }
    return RC_SOCKET_FD_CLOSE_FAILURE;
}

ResourceTunerSocketServer::~ResourceTunerSocketServer() {
    if(this->sockFd != -1) {
        close(this->sockFd);
    }
    this->sockFd = -1;
}
