// Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
// SPDX-License-Identifier: BSD-3-Clause-Clear

#include <memory>
#include <mutex>

#include "ResourceTunerAPIs.h"
#include "Utils.h"
#include "SafeOps.h"
#include "Request.h"
#include "Signal.h"
#include "ResourceTunerSocketClient.h"

static std::shared_ptr<ClientEndpoint> conn(new ResourceTunerSocketClient());
static std::mutex apiLock;

class ConnectionManager {
private:
    std::shared_ptr<ClientEndpoint> connection;

public:
    ConnectionManager(std::shared_ptr<ClientEndpoint> connection) {
        this->connection = connection;
    }

    ~ConnectionManager() {
        if(this->connection != nullptr) {
            this->connection->closeConnection();
        }
    }
};

// - Construct a Request object and populate it with the API specified Params
// - Initiate a connection to the Resource Tuner Server, and send the request to the server
// - Wait for the response from the server, and return the response to the caller (end-client).
int64_t tuneResources(int64_t duration, int32_t properties, int32_t numRes, SysResource* resourceList) {
    // Only one client Thread can send a Request at any moment
    try {
        const std::lock_guard<std::mutex> lock(apiLock);
        const ConnectionManager connMgr(conn);

        // Preliminary Tests
        // These are some basic checks done at the Client end itself to detect
        // Potentially Malformed Reqeusts, to prevent wastage of Server-End Resources.
        if(resourceList == nullptr || numRes <= 0 || duration == 0 || duration < -1) {
            return -1;
        }

        char buf[1024];
        int8_t* ptr8 = (int8_t*)buf;
        ASSIGN_AND_INCR(ptr8, REQ_RESOURCE_TUNING);

        int64_t* ptr64 = (int64_t*)ptr8;
        ASSIGN_AND_INCR(ptr64, 0);
        ASSIGN_AND_INCR(ptr64, duration);

        int32_t* ptr = (int32_t*)ptr64;
        ASSIGN_AND_INCR(ptr, numRes);
        ASSIGN_AND_INCR(ptr, VALIDATE_GE(properties, 0));
        ASSIGN_AND_INCR(ptr, (int32_t)getpid());
        ASSIGN_AND_INCR(ptr, (int32_t)gettid());

        for(int32_t i = 0; i < numRes; i++) {
            SysResource resource = SafeDeref((resourceList + i));

            ASSIGN_AND_INCR(ptr, VALIDATE_GE(resource.mResCode, 0));
            ASSIGN_AND_INCR(ptr, VALIDATE_GE(resource.mResInfo, 0));
            ASSIGN_AND_INCR(ptr, VALIDATE_GE(resource.mOptionalInfo, 0));
            ASSIGN_AND_INCR(ptr, VALIDATE_GT(resource.mNumValues, 0));

            if(resource.mNumValues == 1) {
                ASSIGN_AND_INCR(ptr, VALIDATE_GE(resource.mResValue.value, 0));
            } else {
                for(int32_t j = 0; j < resource.mNumValues; j++) {
                    ASSIGN_AND_INCR(ptr, VALIDATE_GE(resource.mResValue.values[j], 0));
                }
            }
        }

        if(conn == nullptr || RC_IS_NOTOK(conn->initiateConnection())) {
            return -1;
        }

        // Send the request to Resource Tuner Server
        if(RC_IS_NOTOK(conn->sendMsg(buf, sizeof(buf)))) {
            return -1;
        }

        // Get the handle
        char resultBuf[64];
        if(RC_IS_NOTOK(conn->readMsg(resultBuf, sizeof(resultBuf)))) {
            return -1;
        }

        int64_t handleReceived = -1;
        try {
            handleReceived = (int64_t)(SafeDeref(resultBuf));
        } catch(const std::invalid_argument& e) {
            std::cerr<<"Failed to read Handle, Error: "<<e.what()<<std::endl;
        }

        return handleReceived;

    } catch(const std::invalid_argument& e) {
        std::cerr<<"Failed to send Request to Server, Error: "<<e.what()<<std::endl;
        return -1;

    } catch(const std::exception& e) {
        std::cerr<<"Failed to send Request to Server, Error: "<<e.what()<<std::endl;
        return -1;
    }

    return -1;
}

// - Construct a Request object and populate it with the API specified Params
// - Initiate a connection to the Resource Tuner Server, and send the request to the server
int8_t retuneResources(int64_t handle, int64_t duration) {
    try {
        const std::lock_guard<std::mutex> lock(apiLock);
        const ConnectionManager connMgr(conn);

        if(handle <= 0  || duration == 0 || duration < -1) {
            return -1;
        }

        char buf[1024];
        int8_t* ptr8 = (int8_t*)buf;
        ASSIGN_AND_INCR(ptr8, REQ_RESOURCE_RETUNING);

        int64_t* ptr64 = (int64_t*)ptr8;
        ASSIGN_AND_INCR(ptr64, handle);
        ASSIGN_AND_INCR(ptr64, duration);

        int32_t* ptr = (int32_t*)ptr64;
        ASSIGN_AND_INCR(ptr, 0);
        ASSIGN_AND_INCR(ptr, 0);
        ASSIGN_AND_INCR(ptr, (int32_t)getpid());
        ASSIGN_AND_INCR(ptr, (int32_t)gettid());

        if(RC_IS_NOTOK(conn == nullptr || conn->initiateConnection())) {
            return -1;
        }

        if(RC_IS_NOTOK(conn->sendMsg(buf, sizeof(buf)))) {
            return -1;
        }

        return 0;

    } catch(const std::invalid_argument& e) {
        return -1;

    } catch(const std::exception& e) {
        return -1;
    }
}

// - Construct a Request object and populate it with the API specified Params
// - Initiate a connection to the Resource Tuner Server, and send the request to the server
int8_t untuneResources(int64_t handle) {
    try {
        const std::lock_guard<std::mutex> lock(apiLock);
        const ConnectionManager connMgr(conn);

        if(handle <= 0) return -1;

        char buf[1024];
        int8_t* ptr8 = (int8_t*)buf;
        ASSIGN_AND_INCR(ptr8, REQ_RESOURCE_UNTUNING);

        int64_t* ptr64 = (int64_t*)ptr8;
        ASSIGN_AND_INCR(ptr64, handle);
        ASSIGN_AND_INCR(ptr64, -1);

        int32_t* ptr = (int32_t*)ptr64;
        ASSIGN_AND_INCR(ptr, 0);
        ASSIGN_AND_INCR(ptr, 0);
        ASSIGN_AND_INCR(ptr, (int32_t)getpid());
        ASSIGN_AND_INCR(ptr, (int32_t)gettid());

        if(conn == nullptr || RC_IS_NOTOK(conn->initiateConnection())) {
            return -1;
        }

        if(RC_IS_NOTOK(conn->sendMsg(buf, sizeof(buf)))) {
            return -1;
        }

        return 0;

    } catch(const std::invalid_argument& e) {
        return -1;

    } catch(const std::exception& e) {
        return -1;
    }
}

// - Construct a SysConfig object and populate it with the SysConfig Request Params
// - Initiate a connection to the Resource Tuner Server, and send the request to the server
// - Wait for the response from the server, and return the response to the caller (end-client).
int8_t getProp(const char* prop, char* buffer, size_t bufferSize, const char* defValue) {
    try {
        const std::lock_guard<std::mutex> lock(apiLock);
        const ConnectionManager connMgr(conn);

        char buf[1024];
        int8_t* ptr8 = (int8_t*)buf;
        ASSIGN_AND_INCR(ptr8, REQ_SYSCONFIG_GET_PROP);

        const char* charIterator = prop;
        char* charPointer = (char*) ptr8;

        while(*charIterator != '\0') {
            ASSIGN_AND_INCR(charPointer, *charIterator);
            charIterator++;
        }

        ASSIGN_AND_INCR(charPointer, '\0');

        charIterator = "";

        while(*charIterator != '\0') {
            ASSIGN_AND_INCR(charPointer, *charIterator);
            charIterator++;
        }

        ASSIGN_AND_INCR(charPointer, '\0');

        charIterator = defValue;

        while(*charIterator != '\0') {
            ASSIGN_AND_INCR(charPointer, *charIterator);
            charIterator++;
        }

        ASSIGN_AND_INCR(charPointer, '\0');

        int32_t* ptr = (int32_t*)charPointer;

        ASSIGN_AND_INCR(ptr, (int32_t)getpid());
        ASSIGN_AND_INCR(ptr, (int32_t)gettid());

        uint64_t* ptr64 = (uint64_t*)ptr;
        ASSIGN_AND_INCR(ptr64, bufferSize);

        if(conn == nullptr || RC_IS_NOTOK(conn->initiateConnection())) {
            return -1;
        }

        if(RC_IS_NOTOK(conn->sendMsg(buf, sizeof(buf)))) {
            return -1;
        }

        // read the response
        char resultBuf[bufferSize];
        if(RC_IS_NOTOK(conn->readMsg(resultBuf, sizeof(resultBuf)) == -1)) {
            return -1;
        }

        buffer[bufferSize - 1] = '\0';
        strncpy(buffer, resultBuf, bufferSize - 1);

        return 0;

    } catch(const std::invalid_argument& e) {
        return -1;

    } catch(const std::exception& e) {
        return -1;
    }

    return -1;
}

// - Construct a Signal object and populate it with the Signal Request Params
// - Initiate a connection to the Resource Tuner Server, and send the request to the server
// - Wait for the response from the server, and return the response to the caller (end-client).
int64_t tuneSignal(uint32_t signalID, int64_t duration, int32_t properties,
                   const char* appName, const char* scenario, int32_t numArgs,
                   uint32_t* list) {
    try {
        const std::lock_guard<std::mutex> lock(apiLock);
        const ConnectionManager connMgr(conn);

        if(duration < -1) {
            return -1;
        }

        char buf[1024];
        int8_t* ptr8 = (int8_t*)buf;
        ASSIGN_AND_INCR(ptr8, SIGNAL_ACQ);

        int32_t* ptr = (int32_t*)ptr8;
        ASSIGN_AND_INCR(ptr, signalID);

        int64_t* ptr64 = (int64_t*)ptr;
        ASSIGN_AND_INCR(ptr64, 0);
        ASSIGN_AND_INCR(ptr64, duration);

        const char* charIterator = appName;
        char* charPointer = (char*) ptr64;

        while(*charIterator != '\0') {
            ASSIGN_AND_INCR(charPointer, *charIterator);
            charIterator++;
        }

        ASSIGN_AND_INCR(charPointer, '\0');

        charIterator = scenario;

        while(*charIterator != '\0') {
            ASSIGN_AND_INCR(charPointer, *charIterator);
            charIterator++;
        }

        ASSIGN_AND_INCR(charPointer, '\0');

        ptr = (int32_t*)charPointer;
        ASSIGN_AND_INCR(ptr, VALIDATE_GE(numArgs, 0));
        ASSIGN_AND_INCR(ptr, VALIDATE_GE(properties, 0));
        ASSIGN_AND_INCR(ptr, (int32_t)getpid());
        ASSIGN_AND_INCR(ptr, (int32_t)gettid());

        for(int32_t i = 0; i < numArgs; i++) {
            uint32_t arg = list[i];
            ASSIGN_AND_INCR(ptr, arg)
        }

        if(conn == nullptr || RC_IS_NOTOK(conn->initiateConnection())) {
            return -1;
        }

        // Send the request to Resource Tuner Server
        if(RC_IS_NOTOK(conn->sendMsg(buf, sizeof(buf)))) {
            return -1;
        }

        // Get the handle
        char resultBuffer[64];
        if(RC_IS_NOTOK(conn->readMsg(resultBuffer, sizeof(resultBuffer)))) {
            return -1;
        }

        int64_t handleReceived = -1;
        try {
            handleReceived = (int64_t)(SafeDeref(resultBuffer));

        } catch(const std::invalid_argument& e) {}

        return handleReceived;

    } catch(const std::invalid_argument& e) {
        return -1;

    } catch(const std::exception& e) {
        return -1;
    }

    return -1;
}

// - Construct a Signal object and populate it with the Signal Request Params
// - Initiate a connection to the Resource Tuner Server, and send the request to the server
int8_t untuneSignal(int64_t handle) {
    try {
        const std::lock_guard<std::mutex> lock(apiLock);
        const ConnectionManager connMgr(conn);

        char buf[1024];
        int8_t* ptr8 = (int8_t*)buf;
        ASSIGN_AND_INCR(ptr8, SIGNAL_ACQ);

        int32_t* ptr = (int32_t*)ptr8;
        ASSIGN_AND_INCR(ptr, 0);

        int64_t* ptr64 = (int64_t*)ptr;
        ASSIGN_AND_INCR(ptr64, 0);
        ASSIGN_AND_INCR(ptr64, -1);

        const char* charIterator = "";
        char* charPointer = (char*) ptr64;

        while(*charIterator != '\0') {
            ASSIGN_AND_INCR(charPointer, *charIterator);
            charIterator++;
        }

        ASSIGN_AND_INCR(charPointer, '\0');

        charIterator = "";

        while(*charIterator != '\0') {
            ASSIGN_AND_INCR(charPointer, *charIterator);
            charIterator++;
        }

        ASSIGN_AND_INCR(charPointer, '\0');

        ptr = (int32_t*)charPointer;
        ASSIGN_AND_INCR(ptr, 0);
        ASSIGN_AND_INCR(ptr, 0);
        ASSIGN_AND_INCR(ptr, (int32_t)getpid());
        ASSIGN_AND_INCR(ptr, (int32_t)gettid());

        if(conn == nullptr || RC_IS_NOTOK(conn->initiateConnection())) {
            return -1;
        }

        // Send the request to Resource Tuner Server
        if(RC_IS_NOTOK(conn->sendMsg(buf, sizeof(buf)))) {
            return -1;
        }

        return 0;

    } catch(const std::invalid_argument& e) {
        return -1;

    } catch(const std::exception& e) {
        return -1;
    }

    return -1;
}

// - Construct a Signal object and populate it with the Signal Request Params
// - Initiate a connection to the Resource Tuner Server, and send the request to the server
int8_t relaySignal(uint32_t signalID, int64_t duration, int32_t properties,
                   const char* appName, const char* scenario, int32_t numArgs, uint32_t* list) {
    try {
        const std::lock_guard<std::mutex> lock(apiLock);
        const ConnectionManager connMgr(conn);

        if(duration < -1) {
            return -1;
        }

        char buf[1024];
        int8_t* ptr8 = (int8_t*)buf;
        ASSIGN_AND_INCR(ptr8, SIGNAL_RELAY);

        int32_t* ptr = (int32_t*)ptr8;
        ASSIGN_AND_INCR(ptr, signalID);

        int64_t* ptr64 = (int64_t*)ptr;
        ASSIGN_AND_INCR(ptr64, 0);
        ASSIGN_AND_INCR(ptr64, duration);

        const char* charIterator = appName;
        char* charPointer = (char*) ptr64;

        while(*charIterator != '\0') {
            ASSIGN_AND_INCR(charPointer, *charIterator);
            charIterator++;
        }

        ASSIGN_AND_INCR(charPointer, '\0');

        charIterator = scenario;

        while(*charIterator != '\0') {
            ASSIGN_AND_INCR(charPointer, *charIterator);
            charIterator++;
        }

        ASSIGN_AND_INCR(charPointer, '\0');

        ptr = (int32_t*)charPointer;
        ASSIGN_AND_INCR(ptr, VALIDATE_GE(numArgs, 0));
        ASSIGN_AND_INCR(ptr, VALIDATE_GE(properties, 0));
        ASSIGN_AND_INCR(ptr, (int32_t)getpid());
        ASSIGN_AND_INCR(ptr, (int32_t)gettid());

        for(int32_t i = 0; i < numArgs; i++) {
            uint32_t arg = list[i];
            ASSIGN_AND_INCR(ptr, arg)
        }

        if(conn == nullptr || RC_IS_NOTOK(conn->initiateConnection())) {
            return -1;
        }

        // Send the request to Resource Tuner Server
        if(RC_IS_NOTOK(conn->sendMsg(buf, sizeof(buf)))) {
            return -1;
        }

        // Get the handle
        char resultBuffer[64];
        if(RC_IS_NOTOK(conn->readMsg(resultBuffer, sizeof(resultBuffer)))) {
            return -1;
        }

        int64_t handleReceived = -1;
        try {
            handleReceived = (int64_t)(SafeDeref(resultBuffer));

        } catch(const std::invalid_argument& e) {}

        return handleReceived;

    } catch(const std::invalid_argument& e) {
        return -1;

    } catch(const std::exception& e) {
        return -1;
    }

    return -1;
}
