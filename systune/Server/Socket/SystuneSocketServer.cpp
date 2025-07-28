// Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
// SPDX-License-Identifier: BSD-3-Clause-Clear

#include "SystuneSocketServer.h"

#define DEREF_AND_INCR(ptr, type) ({     \
    type val = (type)(SafeDeref(ptr));   \
    ptr++;                               \
    val;                                 \
})

// Helper methods to decode the byte array
static ErrCode DeserializeFromByteArray(char* buf, Request* clientReq) {
    try {
        int8_t* ptr8 = (int8_t*)buf;
        clientReq->setRequestType(DEREF_AND_INCR(ptr8, int8_t));

        int64_t* ptr64 = (int64_t*)ptr8;
        clientReq->setHandle(DEREF_AND_INCR(ptr64, int64_t));
        clientReq->setDuration(DEREF_AND_INCR(ptr64, int64_t));

        int32_t* ptr = (int32_t*)ptr64;
        clientReq->setNumResources(DEREF_AND_INCR(ptr, int32_t));
        clientReq->setPriority(DEREF_AND_INCR(ptr, int32_t));
        clientReq->setClientPID(DEREF_AND_INCR(ptr, int32_t));
        clientReq->setClientTID(DEREF_AND_INCR(ptr, int32_t));

        ptr8 = (int8_t*)ptr;
        clientReq->setBackgroundProcessing(DEREF_AND_INCR(ptr8, int8_t));

        std::vector<Resource*>* resourceList =
            new (GetBlock<std::vector<Resource*>>()) std::vector<Resource*>;
        resourceList->resize(clientReq->getResourcesCount());

        ptr = (int32_t*)ptr8;
        for(int32_t i = 0; i < clientReq->getResourcesCount(); i++) {
            Resource* resource = (Resource*) (GetBlock<Resource>());

            resource->mOpId = DEREF_AND_INCR(ptr, int32_t);
            resource->mOpInfo = DEREF_AND_INCR(ptr, int32_t);
            resource->mOptionalInfo = DEREF_AND_INCR(ptr, int32_t);
            resource->mNumValues = DEREF_AND_INCR(ptr, int32_t);

            if(resource->mNumValues == 1) {
                resource->mConfigValue.singleValue = DEREF_AND_INCR(ptr, int32_t);
            } else {
                for(int32_t j = 0; j < resource->mNumValues; j++) {
                    resource->mConfigValue.valueArray->push_back(DEREF_AND_INCR(ptr, int32_t));
                }
            }

            (*resourceList)[i] = resource;
            clientReq->setResources(resourceList);
        }

    } catch(const std::invalid_argument& e) {
        TYPELOGV(REQUEST_PARSING_FAILURE, "Error", e);
        return RC_REQUEST_PARSING_FAILED;

    } catch(const std::bad_alloc& e) {
        TYPELOGV(REQUEST_MEMORY_ALLOCATION_FAILURE, "Error", e);
        return RC_MEMORY_POOL_BLOCK_RETRIEVAL_FAILURE;

    } catch(const std::exception& e) {
        LOGE("URM_SYSTUNE_SERVER",
             "Request Deserialization Failed with error: " + std::string(e.what()));
        return RC_REQUEST_DESERIALIZATION_FAILURE;
    }

    return RC_SUCCESS;
}

static ErrCode DeserializeFromByteArray(char* buf, SysConfig* clientReq) {
    try {
        int8_t* ptr8 = (int8_t*)buf;
        clientReq->setRequestType(DEREF_AND_INCR(ptr8, int8_t));

        char* charIterator = (char*)ptr8;
        clientReq->setProp(charIterator);

        while(*charIterator != '\0') {
            charIterator++;
        }
        charIterator++;

        clientReq->setValue(charIterator);

        while(*charIterator != '\0') {
            charIterator++;
        }
        charIterator++;

        clientReq->setDefaultValue(charIterator);

        while(*charIterator != '\0') {
            charIterator++;
        }
        charIterator++;

        int32_t* ptr = (int32_t*)charIterator;
        clientReq->setClientPID(DEREF_AND_INCR(ptr, int32_t));
        clientReq->setClientTID(DEREF_AND_INCR(ptr, int32_t));

        uint64_t* ptr64 = (uint64_t*)ptr;
        clientReq->setBufferSize(DEREF_AND_INCR(ptr64, uint64_t));

    } catch(const std::invalid_argument& e) {
        TYPELOGV(REQUEST_PARSING_FAILURE, "Error", e);

        return RC_REQUEST_PARSING_FAILED;

    } catch(const std::bad_alloc& e) {
        TYPELOGV(REQUEST_MEMORY_ALLOCATION_FAILURE, "Error", e);

        return RC_MEMORY_POOL_BLOCK_RETRIEVAL_FAILURE;

    } catch(const std::exception& e) {
        LOGE("URM_SYSTUNE_SERVER",
             "Request Deserialization Failed with error: " + std::string(e.what()));
        return RC_REQUEST_DESERIALIZATION_FAILURE;
    }

    return RC_SUCCESS;
}

static ErrCode DeserializeFromByteArray(char* buf, Signal* clientReq) {
    try {
        int8_t* ptr8 = (int8_t*)buf;
        clientReq->setRequestType(DEREF_AND_INCR(ptr8, int8_t));

        int32_t* ptr = (int32_t*)ptr8;
        clientReq->setSignalID(DEREF_AND_INCR(ptr, int32_t));

        int64_t* ptr64 = (int64_t*)ptr;
        clientReq->setHandle(DEREF_AND_INCR(ptr64, int64_t));
        clientReq->setDuration(DEREF_AND_INCR(ptr64, int64_t));

        char* charIterator = (char*)ptr64;
        clientReq->setAppName(charIterator);
        while(*charIterator != '\0') {
            charIterator++;
        }
        charIterator++;

        clientReq->setScenario(charIterator);

        while(*charIterator != '\0') {
            charIterator++;
        }
        charIterator++;

        ptr = (int32_t*)charIterator;
        clientReq->setNumArgs(DEREF_AND_INCR(ptr, int32_t));
        clientReq->setPriority(DEREF_AND_INCR(ptr, int32_t));
        clientReq->setClientPID(DEREF_AND_INCR(ptr, int32_t));
        clientReq->setClientTID(DEREF_AND_INCR(ptr, int32_t));

        std::vector<uint32_t>* mListArgs =
            new (GetBlock<std::vector<uint32_t>>()) std::vector<uint32_t>;
        mListArgs->resize(clientReq->getNumArgs());

        for(int32_t i = 0; i < clientReq->getNumArgs(); i++) {
            (*mListArgs)[i] = DEREF_AND_INCR(ptr, uint32_t);
        }

        clientReq->setList(mListArgs);

    } catch(const std::invalid_argument& e) {
        TYPELOGV(REQUEST_PARSING_FAILURE, "Error", e);
        return RC_REQUEST_PARSING_FAILED;

    } catch(const std::bad_alloc& e) {
        TYPELOGV(REQUEST_MEMORY_ALLOCATION_FAILURE, "Error", e);
        return RC_MEMORY_POOL_BLOCK_RETRIEVAL_FAILURE;

    } catch(const std::exception& e) {
        LOGE("URM_SYSTUNE_SERVER",
             "Request Deserialization Failed with error: " + std::string(e.what()));
        return RC_REQUEST_DESERIALIZATION_FAILURE;
    }

    return RC_SUCCESS;
}

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
            Request* request;
            try {
                request = new (GetBlock<Request>()) Request();
            } catch(const std::bad_alloc& e) {
                TYPELOGV(REQUEST_MEMORY_ALLOCATION_FAILURE, "Error", e);
                return;
            }

            if(RC_IS_NOTOK(DeserializeFromByteArray(buf, request))) {
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
            SysConfig* sysConfig;
            try {
                sysConfig = new (GetBlock<SysConfig>()) SysConfig();

            } catch(const std::bad_alloc& e) {
                TYPELOGV(REQUEST_MEMORY_ALLOCATION_FAILURE, "Error", e);
                return;
            }

            if(RC_IS_NOTOK(DeserializeFromByteArray(buf, sysConfig))) {
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
            Signal* clientReq;
            try {
                clientReq = new (GetBlock<Signal>()) Signal() ;

            } catch(const std::bad_alloc& e) {
                TYPELOGV(REQUEST_MEMORY_ALLOCATION_FAILURE, "Error", e);
                return;
            }

            if(RC_IS_NOTOK(DeserializeFromByteArray(buf, clientReq))) {
                FreeBlock<Signal>(static_cast<void*>(clientReq));
                return;
            }

            // Note in case of free Signal requests, this value is the original handle passed via the Request.
            // For grab Signal requests, this value is the new Request Handle.
            int64_t handleToReturn = this->mSystuneMessageAsyncCb(SIGNAL_MESSAGE_RECEIVER_CALLBACK, clientReq);

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
