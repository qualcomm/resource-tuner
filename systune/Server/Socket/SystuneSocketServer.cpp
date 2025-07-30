// Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
// SPDX-License-Identifier: BSD-3-Clause-Clear

#include "SystuneSocketServer.h"

SystuneSocketServer::SystuneSocketServer(uint32_t mListeningPort,
                                         SystuneMessageAsyncCallback mSystuneMessageAsyncCb,
                                         SystuneMessageSyncCallback mSystuneMessageSyncCb) {

    this->mListeningPort = mListeningPort;
    this->mSystuneMessageAsyncCb = mSystuneMessageAsyncCb;
    this->mSystuneMessageSyncCb = mSystuneMessageSyncCb;
}

void SystuneSocketServer::parseIncomingRequest(char* buf, int32_t clientSocket) {
    int8_t requestType = *(int8_t*) buf;

    switch(requestType) {
        case REQ_RESOURCE_TUNING:
        case REQ_RESOURCE_RETUNING:
        case REQ_RESOURCE_UNTUNING:
        case REQ_CLIENT_GET_REQUESTS: {
            Request* request = nullptr;
            try {
                request = new (GetBlock<Request>()) Request();
                std::vector<Resource*>* resourceList =
                    new (GetBlock<std::vector<Resource*>>()) std::vector<Resource*>;
                request->setResources(resourceList);

            } catch(const std::bad_alloc& e) {
                TYPELOGV(REQUEST_MEMORY_ALLOCATION_FAILURE, "Error", e);
                return;
            }

            if(RC_IS_NOTOK(request->deserialize(buf))) {
                FreeBlock<Request>(static_cast<void*>(request));
                return;
            }

            if(requestType == REQ_CLIENT_GET_REQUESTS) {
                char resultBuffer[2048];
                resultBuffer[sizeof(resultBuffer) - 1] = '\0';
                // this->mSystuneMessageAsyncCb(PROVISIONER_SEND_DATA_TO_BUFFER_CALLBACK, request, resultBuffer);

                if(write(clientSocket, resultBuffer, sizeof(resultBuffer)) == -1) {
                    TYPELOGV(ERRNO_LOG, "write", strerror(errno));
                }
            } else {
                // Note in case of untune / retune requests, this value is the original handle passed via the Request.
                // For tune requests, this value is the new Request Handle.
                int64_t handleToReturn = this->mSystuneMessageAsyncCb(PROVISIONER_MESSAGE_RECEIVER_CALLBACK, request);

                // Only in Case of Tune Requests, Write back the handle to the client.
                if(requestType == REQ_RESOURCE_TUNING) {
                    if(write(clientSocket, &handleToReturn, sizeof(int64_t)) == -1) {
                        TYPELOGV(ERRNO_LOG, "write", strerror(errno));
                    }
                }
            }
            break;
        }
        case REQ_SYSCONFIG_GET_PROP:
        case REQ_SYSCONFIG_SET_PROP: {
            SysConfig* sysConfig = nullptr;
            try {
                sysConfig = new (GetBlock<SysConfig>()) SysConfig();

            } catch(const std::bad_alloc& e) {
                TYPELOGV(REQUEST_MEMORY_ALLOCATION_FAILURE, "Error", e);
                return;
            }

            if(RC_IS_NOTOK(sysConfig->deserialize(buf))) {
                FreeBlock<SysConfig>(static_cast<void*>(sysConfig));
                return;
            }

            uint64_t bufferSize = sysConfig->getBufferSize();

            // Create a Buffer to hold the result
            char* resultBuf = nullptr;

            try {
                resultBuf = new char[bufferSize];

            } catch(const std::bad_alloc& e) {
                LOGE("URM_MEMORY_POOL", "Failed to allocate memory for Result Buffer");
                FreeBlock<SysConfig>(static_cast<void*>(sysConfig));
                return;
            }

            // Call the SysConfig CALLBACK
            this->mSystuneMessageSyncCb(requestType, sysConfig, resultBuf, bufferSize);

            if(requestType == REQ_SYSCONFIG_GET_PROP) {
                if(write(clientSocket, resultBuf, bufferSize) == -1) {
                    TYPELOGV(ERRNO_LOG, "write", strerror(errno));
                }
            }
            break;
        }
        case SIGNAL_ACQ:
        case SIGNAL_FREE:
        case SIGNAL_RELAY: {
            Signal* signal = nullptr;
            try {
                signal = new (GetBlock<Signal>()) Signal();
                std::vector<uint32_t>* mListArgs =
                    new (GetBlock<std::vector<uint32_t>>()) std::vector<uint32_t>;
                signal->setList(mListArgs);

            } catch(const std::bad_alloc& e) {
                TYPELOGV(REQUEST_MEMORY_ALLOCATION_FAILURE, "Error", e);
                return;
            }

            if(RC_IS_NOTOK(signal->deserialize(buf))) {
                FreeBlock<Signal>(static_cast<void*>(signal));
                return;
            }

            // Note in case of free Signal requests, this value is the original handle passed via the Request.
            // For grab Signal requests, this value is the new Request Handle.
            int64_t handleToReturn = this->mSystuneMessageAsyncCb(SIGNAL_MESSAGE_RECEIVER_CALLBACK, signal);

            if(requestType == SIGNAL_ACQ) {
                if(write(clientSocket, &handleToReturn, sizeof(int64_t)) == -1) {
                    TYPELOGV(ERRNO_LOG, "write", strerror(errno));
                }
            }
            break;
        }
        default:
            LOGE("URM_SOCKET_SERVER", "Invalid Request Type");
            break;
    }
}

// Called by server, this will put the server in listening mode
int32_t SystuneSocketServer::ListenForClientRequests() {
    if((sockFd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        TYPELOGV(ERRNO_LOG, "socket", strerror(errno));
        LOGE("URM_SOCKET_SERVER", "Failed to initialize Server Socket");
        return RC_SOCKET_CONN_NOT_INITIALIZED;
    }

    // Make the socket Non-Blocking
    fcntl(sockFd, F_SETFL, O_NONBLOCK);

    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(this->mListeningPort);
    addr.sin_addr.s_addr = htonl(INADDR_ANY);

    // Reuse address. Useful when the server is restarted right away. The OS takes time to clean up the socket.
    // The socket spends the longest of that cleanup state in TIME_WAIT section. This option allows the socket
    // to reuse that same address when server is in TIME_WAIT state.
    int32_t reuse = 1;
    if(setsockopt(sockFd, SOL_SOCKET, SO_REUSEADDR, (const char*)&reuse, sizeof(reuse)) < 0) {
        TYPELOGV(ERRNO_LOG, "setsockopt", strerror(errno));
        LOGE("URM_SOCKET_SERVER", "Failed to initialize Server Socket");
        return RC_SOCKET_CONN_NOT_INITIALIZED;
    }

    if(bind(sockFd, (const sockaddr*)&addr, sizeof(addr)) < 0) {
        TYPELOGV(ERRNO_LOG, "bind", strerror(errno));
        LOGE("URM_SOCKET_SERVER", "Failed to initialize Server Socket");
        return RC_SOCKET_CONN_NOT_INITIALIZED;
    }

    if(listen(sockFd, 128) < 0) {
        TYPELOGV(ERRNO_LOG, "listen", strerror(errno));
        LOGE("URM_SOCKET_SERVER", "Failed to initialize Server Socket");
        return RC_SOCKET_CONN_NOT_INITIALIZED;
    }

    while(this->mSystuneMessageAsyncCb(SERVER_ONLINE_CHECK_CALLBACK, nullptr)) {
        int32_t clientSocket;
        if((clientSocket = accept(sockFd, nullptr, nullptr)) < 0) {
            if(errno != EAGAIN && errno != EWOULDBLOCK) {
                TYPELOGV(ERRNO_LOG, "accept", strerror(errno));
                LOGE("URM_SOCKET_SERVER", "Server Socket-Endpoint crashed");
                return RC_SOCKET_OP_FAILURE;
            }
        }

        if(clientSocket >= 0) {
            char buf[1024];

            int32_t bytesRead;
            if((bytesRead = recv(clientSocket, buf, 1024, 0)) < 0) {
                if(errno != EAGAIN && errno != EWOULDBLOCK) {
                    TYPELOGV(ERRNO_LOG, "recv", strerror(errno));
                    LOGE("URM_SOCKET_SERVER", "Server Socket-Endpoint crashed");
                    return RC_SOCKET_OP_FAILURE;
                }
            }

            if(bytesRead > 0) {
                parseIncomingRequest(buf, clientSocket);
            }
            close(clientSocket);
        }
    }

    return RC_SUCCESS;
}

int32_t SystuneSocketServer::closeConnection() {
    if(this->sockFd != -1) {
        return close(this->sockFd);
    }
    return RC_SOCKET_FD_CLOSE_FAILURE;
}

SystuneSocketServer::~SystuneSocketServer() {
    if(this->sockFd != -1) {
        close(this->sockFd);
    }
    this->sockFd = -1;
}
