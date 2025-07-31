// Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
// SPDX-License-Identifier: BSD-3-Clause-Clear

#include "SystuneAPIs.h"

static std::unique_ptr<ClientEndpoint> conn(new SystuneSocketClient());
static std::mutex apiLock;

// - Construct a Request object and populate it with the API specified Params
// - Initiate a connection to the Systune Server, and send the request to the server
// - Wait for the response from the server, and return the response to the caller (end-client).
int64_t tuneResources(int64_t duration, int32_t properties, int32_t numRes, std::vector<Resource*>* res) {
    // Only one client Thread can send a Request at any moment
    try {
        const std::lock_guard<std::mutex> lock(apiLock);

        // Preliminary Tests
        // These are some basic checks done at the Client end itself to detect
        // Potentially Malformed Reqeusts, to prevent wastage of Server-End Resources.
        if(res == nullptr || res->size() != numRes || duration == 0 || duration < -1) {
            return RC_REQ_SUBMISSION_FAILURE;
        }

        Request* request = nullptr;
        try {
            request = new Request;

        } catch(const std::bad_alloc& e) {
            return RC_REQ_SUBMISSION_FAILURE;
        }

        request->setRequestType(REQ_RESOURCE_TUNING);
        request->setNumResources(numRes);
        request->setDuration(duration);
        request->setProperties(properties);
        request->setClientPID(getpid());
        request->setClientTID(gettid());
        request->setResources(res);

        if(conn == nullptr || RC_IS_NOTOK(conn->initiateConnection())) {
            delete request;
            return RC_REQ_SUBMISSION_FAILURE;
        }

        // Send the request to Systune Server
        if(RC_IS_NOTOK(conn->sendMsg(REQ_RESOURCE_TUNING, static_cast<void*>(request)))) {
            conn->closeConnection();
            delete request;
            return RC_REQ_SUBMISSION_FAILURE;
        }

        // Get the handle
        char buf[64];
        if(RC_IS_NOTOK(conn->readMsg(buf, sizeof(buf)))) {
            conn->closeConnection();
            delete request;
            return RC_REQ_SUBMISSION_FAILURE;
        }

        int64_t handleReceived = -1;
        try {
            handleReceived = (int64_t)(SafeDeref(buf));
        } catch(std::invalid_argument& e) {
            std::cerr<<"Failed to read Handle, Error: "<<e.what()<<std::endl;
        }

        conn->closeConnection();

        delete request;
        return handleReceived;

    } catch(std::exception & e) {
        // add logging to file or ftrace
    }

    return RC_REQ_SUBMISSION_FAILURE;
}

// - Construct a Request object and populate it with the API specified Params
// - Initiate a connection to the Systune Server, and send the request to the server
ErrCode retuneResources(int64_t handle, int64_t duration) {
    try {
        const std::lock_guard<std::mutex> lock(apiLock);

        if(handle <= 0  || duration == 0 || duration < -1) return RC_REQ_SUBMISSION_FAILURE;

        Request* request = nullptr;
        try {
            request = new Request;
        } catch(const std::bad_alloc& e) {
            return RC_REQ_SUBMISSION_FAILURE;
        }

        request->setRequestType(REQ_RESOURCE_RETUNING);
        request->setHandle(handle);
        request->setDuration(duration);
        request->setClientPID(getpid());
        request->setClientTID(gettid());
        request->setProperties(0); // Not important for Retune Requests
        request->setNumResources(0);
        request->setResources(nullptr);

        if(RC_IS_NOTOK(conn == nullptr || conn->initiateConnection())) {
            delete request;
            return RC_REQ_SUBMISSION_FAILURE;
        }

        if(RC_IS_NOTOK(conn->sendMsg(REQ_RESOURCE_RETUNING, static_cast<void*>(request)))) {
            conn->closeConnection();
            delete request;
            return RC_REQ_SUBMISSION_FAILURE;
        }

        conn->closeConnection();

        delete request;
        return RC_SUCCESS;

    } catch(std::exception& e) {
        return RC_REQ_SUBMISSION_FAILURE;
    }
}

// - Construct a Request object and populate it with the API specified Params
// - Initiate a connection to the Systune Server, and send the request to the server
ErrCode untuneResources(int64_t handle) {
    try {
        const std::lock_guard<std::mutex> lock(apiLock);

        if(handle <= 0) return RC_REQ_SUBMISSION_FAILURE;

        Request* request = nullptr;
        try {
            request = new Request;
        } catch(const std::bad_alloc& e) {
            return RC_REQ_SUBMISSION_FAILURE;
        }

        request->setRequestType(REQ_RESOURCE_UNTUNING);
        request->setHandle(handle);
        request->setClientPID(getpid());
        request->setClientTID(gettid());
        request->setProperties(0); // Not important for Untune Requests
        request->setNumResources(0);
        request->setResources(nullptr);

        if(conn == nullptr || RC_IS_NOTOK(conn->initiateConnection())) {
            delete request;
            return RC_REQ_SUBMISSION_FAILURE;
        }

        if(RC_IS_NOTOK(conn->sendMsg(REQ_RESOURCE_UNTUNING, static_cast<void*>(request)))) {
            conn->closeConnection();
            delete request;
            return RC_REQ_SUBMISSION_FAILURE;
        }

        conn->closeConnection();

        delete request;
        return RC_SUCCESS;

    } catch(std::exception& e) {
        return RC_REQ_SUBMISSION_FAILURE;
    }
}

// - Construct a Request object and populate it with the API specified Params
// - Initiate a connection to the Systune Server, and send the request to the server
// - Wait for the response from the server, and return the response to the caller (end-client).
std::string getrequests() {
    try {

        const std::lock_guard<std::mutex> lock(apiLock);

        Request* request = nullptr;
        try {
            request = new Request;

        } catch(const std::bad_alloc& e) {
            return "Error sending get request to Systune Server";
        }

        request->setRequestType(REQ_CLIENT_GET_REQUESTS);
        request->setClientPID(getpid());
        request->setClientTID(gettid());
        request->setNumResources(0);
        request->setResources(nullptr);

        if(RC_IS_NOTOK(conn->initiateConnection())) {
            delete request;
            return "";
        }

        if(RC_IS_NOTOK(conn->sendMsg(REQ_CLIENT_GET_REQUESTS, request))) {
            conn->closeConnection();
            delete request;
            return "Error sending request to Systune Server";
        }

        // read the response
        char buffer[2048];
        if(RC_IS_NOTOK(conn->readMsg(buffer, sizeof(buffer)))) {
            conn->closeConnection();
            delete request;
            return "Error reading request from Systune Server";
        }

        // parse the response
        std::string response(buffer);

        // clear the buffer after every call
        memset(buffer, 0, sizeof(buffer));

        conn->closeConnection();

        delete request;
        return response;

    } catch(std::exception& e) {}

    return "";
}

// - Construct a SysConfig object and populate it with the SysConfig Request Params
// - Initiate a connection to the Systune Server, and send the request to the server
// - Wait for the response from the server, and return the response to the caller (end-client).
ErrCode getprop(const char* prop, char* buffer, size_t buffer_size, const char* def_value) {
    try {
        const std::lock_guard<std::mutex> lock(apiLock);

        SysConfig* sysConfig;
        try {
            sysConfig = new SysConfig;
        } catch(const std::bad_alloc& e) {
            return RC_REQ_SUBMISSION_FAILURE;
        }

        sysConfig->setRequestType(REQ_SYSCONFIG_GET_PROP);
        sysConfig->setClientPID(getpid());
        sysConfig->setClientTID(gettid());
        sysConfig->setProp(prop);
        sysConfig->setValue("");
        sysConfig->setDefaultValue(def_value);
        sysConfig->setBufferSize(buffer_size);

        if(conn == nullptr || RC_IS_NOTOK(conn->initiateConnection())) {
            delete sysConfig;
            return RC_REQ_SUBMISSION_FAILURE;
        }

        if(RC_IS_NOTOK(conn->sendMsg(REQ_SYSCONFIG_GET_PROP, static_cast<void*>(sysConfig)))) {
            conn->closeConnection();
            delete sysConfig;
            return RC_REQ_SUBMISSION_FAILURE;
        }

        // read the response
        char buf[buffer_size];
        if(RC_IS_NOTOK(conn->readMsg(buf, sizeof(buf)) == -1)) {
            conn->closeConnection();
            delete sysConfig;
            return RC_REQ_SUBMISSION_FAILURE;
        }

        buffer[buffer_size - 1] = '\0';
        strncpy(buffer, buf, buffer_size - 1);

        conn->closeConnection();
        delete sysConfig;
        return RC_SUCCESS;

    } catch(const std::exception& e) {}

    return RC_REQ_SUBMISSION_FAILURE;
}

// - Construct a SysConfig object and populate it with the SysConfig Request Params
// - Initiate a connection to the Systune Server, and send the request to the server
ErrCode setprop(const char* prop, const char* value) {
    try {
        const std::lock_guard<std::mutex> lock(apiLock);

        SysConfig* sysConfig;
        try {
            sysConfig = new SysConfig;

        } catch(const std::bad_alloc& e) {
            return RC_REQ_SUBMISSION_FAILURE;
        }

        sysConfig->setRequestType(REQ_SYSCONFIG_SET_PROP);
        sysConfig->setClientPID(getpid());
        sysConfig->setClientTID(gettid());
        sysConfig->setProp(prop);
        sysConfig->setValue(value);
        sysConfig->setDefaultValue("");
        sysConfig->setBufferSize(0);

        if(conn == nullptr || RC_IS_NOTOK(conn->initiateConnection())) {
            delete sysConfig;
            return RC_REQ_SUBMISSION_FAILURE;
        }

        if(RC_IS_NOTOK(conn->sendMsg(REQ_SYSCONFIG_SET_PROP, static_cast<void*>(sysConfig)))) {
            conn->closeConnection();
            delete sysConfig;
            return RC_REQ_SUBMISSION_FAILURE;
        }

        conn->closeConnection();
        delete sysConfig;
        return RC_SUCCESS;

    } catch(std::exception& e) {}

    return RC_REQ_SUBMISSION_FAILURE;
}

// - Construct a Signal object and populate it with the SysSignal Request Params
// - Initiate a connection to the Systune Server, and send the request to the server
// - Wait for the response from the server, and return the response to the caller (end-client).
int64_t tuneSignal(uint32_t signalID, int64_t duration, int32_t properties,
                   const char* appName, const char* scenario, int32_t numArgs,
                   std::vector<uint32_t>* list) {
    try {
        const std::lock_guard<std::mutex> lock(apiLock);

        if(duration < -1 || (list != nullptr && list->size() != numArgs)) {
            return RC_REQ_SUBMISSION_FAILURE;
        }

        Signal* signal = nullptr;
        try {
            signal = new Signal;

        } catch(const std::bad_alloc& e) {
            return RC_REQ_SUBMISSION_FAILURE;
        }

        signal->setRequestType(SIGNAL_ACQ);
        signal->setSignalID(signalID);
        signal->setDuration(duration);
        signal->setProperties(properties);
        signal->setClientPID(getpid());
        signal->setClientTID(gettid());
        signal->setNumArgs(numArgs);
        signal->setAppName(appName);
        signal->setScenario(scenario);
        signal->setList(list);

        if(conn == nullptr || RC_IS_NOTOK(conn->initiateConnection())) {
            delete signal;
            return RC_REQ_SUBMISSION_FAILURE;
        }

        // Send the request to Systune Server
        if(RC_IS_NOTOK(conn->sendMsg(SIGNAL_ACQ, static_cast<void*>(signal)))) {
            conn->closeConnection();
            delete signal;
            return RC_REQ_SUBMISSION_FAILURE;
        }

        // Get the handle
        char buf[64];
        if(RC_IS_NOTOK(conn->readMsg(buf, sizeof(buf)))) {
            conn->closeConnection();
            delete signal;
            return RC_REQ_SUBMISSION_FAILURE;
        }

        int64_t handleReceived = -1;
        try {
            handleReceived = (int64_t)(SafeDeref(buf));

        } catch(std::invalid_argument& e) {}

        conn->closeConnection();
        delete signal;
        return handleReceived;

    } catch(std::exception& e) {}

    return RC_REQ_SUBMISSION_FAILURE;
}

// - Construct a Signal object and populate it with the SysSignal Request Params
// - Initiate a connection to the Systune Server, and send the request to the server
ErrCode untuneSignal(int64_t handle) {
    try {
        const std::lock_guard<std::mutex> lock(apiLock);

        Signal* signal = nullptr;
        try {
            signal = new Signal;

        } catch(const std::bad_alloc& e) {
            return RC_REQ_SUBMISSION_FAILURE;
        }

        signal->setRequestType(SIGNAL_FREE);
        signal->setSignalID(0);
        signal->setDuration(0);
        signal->setHandle(handle);
        signal->setProperties(0);
        signal->setClientPID(getpid());
        signal->setClientTID(gettid());
        signal->setNumArgs(0);
        signal->setAppName(nullptr);
        signal->setScenario(nullptr);
        signal->setList(nullptr);

        if(conn == nullptr || RC_IS_NOTOK(conn->initiateConnection())) {
            delete signal;
            return RC_REQ_SUBMISSION_FAILURE;
        }

        // Send the request to Systune Server
        if(RC_IS_NOTOK(conn->sendMsg(SIGNAL_FREE, static_cast<void*>(signal)))) {
            conn->closeConnection();
            delete signal;
            return RC_REQ_SUBMISSION_FAILURE;
        }

        // Close the connection
        conn->closeConnection();

        delete signal;
        return RC_SUCCESS;

    } catch(std::exception& e) {}

    return RC_REQ_SUBMISSION_FAILURE;
}
