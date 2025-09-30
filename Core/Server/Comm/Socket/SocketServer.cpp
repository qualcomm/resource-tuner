// Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
// SPDX-License-Identifier: BSD-3-Clause-Clear

#include "SocketServer.h"

static int8_t isServerOnline() {
    return ResourceTunerSettings::isServerOnline();
}

static void onMsgRecv(int32_t clientSocket, MsgForwardInfo* msgForwardInfo) {
    std::shared_ptr<RequestReceiver> receiver = RequestReceiver::getInstance();

    int8_t requestType = *(int8_t*) msgForwardInfo->buffer;
    ThreadPool* requestPool = receiver->getRequestThreadPool();

    switch(requestType) {
        // Resource Provisioning Requests
        case REQ_RESOURCE_TUNING: {
            if(!ComponentRegistry::isModuleEnabled(MOD_CORE)) {
                TYPELOGV(NOTIFY_MODULE_NOT_ENABLED, "Core");
                return;
            }
            msgForwardInfo->handle = AuxRoutines::generateUniqueHandle();
            if(msgForwardInfo->handle < 0) {
                // Handle Generation Failure
                LOGE("RESTUNE_REQUEST_RECEIVER",
                     "Failed to Generate Request handle");
                return;
            }
        }
        case REQ_RESOURCE_RETUNING:
        case REQ_RESOURCE_UNTUNING: {
            if(!ComponentRegistry::isModuleEnabled(MOD_CORE)) {
                TYPELOGV(NOTIFY_MODULE_NOT_ENABLED, "Core");
                return;
            }
            // Enqueue the Request to the Thread Pool for async processing.
            if(requestPool != nullptr) {
                if(!requestPool->enqueueTask(
                    ComponentRegistry::getEventCallback(MOD_CORE_ON_MSG_RECV), msgForwardInfo)) {
                    TYPELOGD(INCOMING_REQUEST_THREAD_POOL_ENQUEUE_FAILURE);
                }
            } else {
                TYPELOGD(INCOMING_REQUEST_THREAD_POOL_INIT_NOT_DONE);
            }

            // Only in Case of Tune Requests, Write back the handle to the client.
            if(requestType == REQ_RESOURCE_TUNING) {
                if(write(clientSocket, (const void*)&msgForwardInfo->handle, sizeof(int64_t)) == -1) {
                    TYPELOGV(ERRNO_LOG, "write", strerror(errno));
                }
            }
            break;
        }
        // Prop Get Requests
        case REQ_PROP_GET: {
            // Decode Prop Fetch Request
            PropConfig propConfig;
            memset(&propConfig, 0, sizeof(propConfig));

            int8_t* ptr8 = (int8_t*)msgForwardInfo->buffer;
            (void) DEREF_AND_INCR(ptr8, int8_t);

            char* charIterator = (char*)ptr8;
            propConfig.mPropName = charIterator;

            while(*charIterator != '\0') {
                charIterator++;
            }
            charIterator++;

            uint64_t* ptr64 = (uint64_t*)charIterator;
            propConfig.mBufferSize = DEREF_AND_INCR(ptr64, uint64_t);

            ComponentRegistry::getEventCallback(PROP_ON_MSG_RECV)(&propConfig);
            std::string result = propConfig.mResult;

            size_t maxSafeSize = result.size() + 1;
            size_t bytesToWrite = std::min(propConfig.mBufferSize, maxSafeSize);

            if(write(clientSocket, (const void*)result.c_str(), bytesToWrite) == -1) {
                TYPELOGV(ERRNO_LOG, "write", strerror(errno));
            }
            break;
        }
        // Signal Requests
        case REQ_SIGNAL_TUNING: {
            if(!ComponentRegistry::isModuleEnabled(MOD_SIGNAL)) {
                TYPELOGV(NOTIFY_MODULE_NOT_ENABLED, "Signals");
                return;
            }
            msgForwardInfo->handle = AuxRoutines::generateUniqueHandle();
            if(msgForwardInfo->handle < 0) {
                // Handle Generation Failure
                LOGE("RESTUNE_REQUEST_RECEIVER",
                     "Failed to Generate Request handle");
                return;
            }
        }
        case REQ_SIGNAL_UNTUNING:
        case REQ_SIGNAL_RELAY: {
            if(!ComponentRegistry::isModuleEnabled(MOD_SIGNAL)) {
                TYPELOGV(NOTIFY_MODULE_NOT_ENABLED, "Signals");
                return;
            }
            // Enqueue the Request to the Thread Pool for async processing.
            if(requestPool != nullptr) {
                if(!requestPool->enqueueTask(
                    ComponentRegistry::getEventCallback(MOD_SIGNAL_ON_MSG_RECV), msgForwardInfo)) {
                    TYPELOGD(INCOMING_REQUEST_THREAD_POOL_ENQUEUE_FAILURE);
                }
            } else {
                TYPELOGD(INCOMING_REQUEST_THREAD_POOL_INIT_NOT_DONE);
            }

            // Only in Case of Tune Signals, Write back the handle to the client.
            if(requestType == REQ_SIGNAL_TUNING) {
                if(write(clientSocket, (const void*)&msgForwardInfo->handle, sizeof(int64_t)) == -1) {
                    TYPELOGV(ERRNO_LOG, "write", strerror(errno));
                }
            }
            break;
        }
        default:
            LOGE("RESTUNE_SOCKET_SERVER", "Invalid Request Type");
            break;
    }
}

SocketServer::SocketServer(uint32_t mListeningPort) {

    this->sockFd = -1;
    this->mListeningPort = mListeningPort;
}

// Called by server, this will put the server in listening mode
int32_t SocketServer::ListenForClientRequests() {
    if((this->sockFd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        TYPELOGV(ERRNO_LOG, "socket", strerror(errno));
        LOGE("RESTUNE_SOCKET_SERVER", "Failed to initialize Server Socket");
        return RC_SOCKET_CONN_NOT_INITIALIZED;
    }

    // Make the socket Non-Blocking
    fcntl(this->sockFd, F_SETFL, O_NONBLOCK);

    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(this->mListeningPort);
    addr.sin_addr.s_addr = htonl(INADDR_ANY);

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

    while(isServerOnline()) {
        int32_t clientsFdCount = epoll_wait(epollFd, events, maxEvents, -1);

        for(int32_t i = 0; i < clientsFdCount; i++) {
            if(events[i].data.fd == this->sockFd) {
                // Process all the Requests in the backlog
                while(true) {
                    sockaddr_in clientAddr{};
                    socklen_t clientLen = sizeof(clientAddr);

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
                            onMsgRecv(clientSocket, info);
                        }
                        close(clientSocket);
                    }
                }
            }
        }
    }

    return RC_SUCCESS;
}

int32_t SocketServer::closeConnection() {
    if(this->sockFd != -1) {
        return close(this->sockFd);
    }
    return RC_SOCKET_FD_CLOSE_FAILURE;
}

SocketServer::~SocketServer() {
    if(this->sockFd != -1) {
        close(this->sockFd);
    }
    this->sockFd = -1;
}

void listenerThreadStartRoutine() {
    SocketServer* connection;
    try {
        connection = new SocketServer(ResourceTunerSettings::metaConfigs.mListeningPort);
    } catch(const std::bad_alloc& e) {
        LOGE("RESTUNE_REQUEST_RECEIVER",
             "Failed to allocate memory for Resource Tuner Socket Server-Endpoint, Resource Tuner \
              Server startup failed: " + std::string(e.what()));

        return;
    } catch(const std::exception& e) {
        LOGE("RESTUNE_REQUEST_RECEIVER",
             "Failed to start the Resource Tuner Listener, error: " + std::string(e.what()));

        return;
    }

    if(RC_IS_NOTOK(connection->ListenForClientRequests())) {
        LOGE("RESTUNE_REQUEST_RECEIVER", "Server Socket Endpoint crashed");
    }

    if(connection != nullptr) {
        delete(connection);
    }
    return;
}
