// Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
// SPDX-License-Identifier: BSD-3-Clause-Clear

#include "SocketServer.h"

ResourceTunerSocketServer::ResourceTunerSocketServer(
    ServerOnlineCheckCallback mServerOnlineCheckCb,
    ResourceTunerMessageReceivedCallback mResourceTunerMessageRecvCb) {

    this->sockFd = -1;
    this->mServerOnlineCheckCb = mServerOnlineCheckCb;
    this->mResourceTunerMessageRecvCb = mResourceTunerMessageRecvCb;
}

// Called by server, this will put the server in listening mode
int32_t ResourceTunerSocketServer::ListenForClientRequests() {
    if((this->sockFd = socket(AF_UNIX, SOCK_STREAM, 0)) < 0) {
        TYPELOGV(ERRNO_LOG, "socket", strerror(errno));
        LOGE("RESTUNE_SOCKET_SERVER", "Failed to initialize Server Socket");
        return RC_SOCKET_CONN_NOT_INITIALIZED;
    }

    // Make the socket Non-Blocking
    fcntl(this->sockFd, F_SETFL, O_NONBLOCK);

    struct sockaddr_un addr;
    memset(&addr, 0, sizeof(sockaddr_un));
    addr.sun_family = AF_UNIX;
    strncpy(addr.sun_path, RESTUNE_SOCKET_PATH, sizeof(addr.sun_path) - 1);

    int32_t epollFd = epoll_create1(0);
    epoll_event event{}, events[maxEvents];
    event.events = EPOLLIN;
    event.data.fd = this->sockFd;
    epoll_ctl(epollFd, EPOLL_CTL_ADD, this->sockFd, &event);

    int32_t reuse = 1;
    if(setsockopt(this->sockFd, SOL_SOCKET, SO_REUSEADDR, (const char*)&reuse, sizeof(reuse)) < 0) {
        TYPELOGV(ERRNO_LOG, "setsockopt", strerror(errno));
        LOGE("RESTUNE_SOCKET_SERVER", "Failed to initialize Server Socket");
        return RC_SOCKET_CONN_NOT_INITIALIZED;
    }

    if(bind(this->sockFd, (const sockaddr*)&addr, sizeof(addr)) < 0) {
        TYPELOGV(ERRNO_LOG, "bind", strerror(errno));
        LOGE("RESTUNE_SOCKET_SERVER", "Failed to initialize Server Socket");
        return RC_SOCKET_CONN_NOT_INITIALIZED;
    }

    if(listen(this->sockFd, maxEvents) < 0) {
        TYPELOGV(ERRNO_LOG, "listen", strerror(errno));
        LOGE("RESTUNE_SOCKET_SERVER", "Failed to initialize Server Socket");
        return RC_SOCKET_CONN_NOT_INITIALIZED;
    }

    while(this->mServerOnlineCheckCb()) {
        int32_t clientsFdCount = epoll_wait(epollFd, events, maxEvents, 1000);

        for(int32_t i = 0; i < clientsFdCount; i++) {
            if(events[i].data.fd == this->sockFd) {
                // Process all the Requests in the backlog
                while(true) {
                    int32_t clientSocket = -1;
                    if((clientSocket = accept(this->sockFd, nullptr, nullptr)) < 0) {
                        if(errno != EAGAIN && errno != EWOULDBLOCK) {
                            TYPELOGV(ERRNO_LOG, "accept", strerror(errno));
                            LOGE("RESTUNE_SOCKET_SERVER", "Server Socket-Endpoint crashed");
                            return RC_SOCKET_OP_FAILURE;
                        } else {
                            // No more clients to accept, Backlog is completely drained.
                            break;
                        }
                    }

                    if(clientSocket >= 0) {
                        MsgForwardInfo* info = nullptr;
                        char* reqBuf = nullptr;

                        try {
                            info = new (GetBlock<MsgForwardInfo>()) MsgForwardInfo;
                            reqBuf = new (GetBlock<char[REQ_BUFFER_SIZE]>()) char[REQ_BUFFER_SIZE];

                            info->buffer = reqBuf;
                            info->bufferSize = REQ_BUFFER_SIZE;

                        } catch(const std::bad_alloc& e) {
                            FreeBlock<MsgForwardInfo>(info);
                            FreeBlock<char[REQ_BUFFER_SIZE]>(reqBuf);

                            // Failed to allocate memory for Request, close client socket.
                            close(clientSocket);
                            continue;
                        }

                        int32_t bytesRead = 0;
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
            }
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
