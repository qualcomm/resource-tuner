// Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
// SPDX-License-Identifier: BSD-3-Clause-Clear

#ifndef SYSTUNE_MESSAGE_H
#define SYSTUNE_MESSAGE_H

#include <cstdint>

class Message {
protected:
    int8_t mReqType; //!< Type of the request. Possible values: TUNE, UNTUNE, RETUNE.
    int64_t mHandle; //!< The unique generated handle for the request.
    int64_t mDuration; //!< Duration. -1 means infinite duration.
    int32_t mPriority; //!<Priority of the request, as specified in the tuneResources API call.
    int32_t mClientPID; //!< Process ID of the client making the request.
    int32_t mClientTID; //!< Thread ID of the client making the request.

public:
    int8_t getRequestType();
    int64_t getDuration();
    int32_t getClientPID();
    int32_t getClientTID();
    int64_t getHandle();
    int32_t getPriority();

    void setRequestType(int8_t reqType);
    void setDuration(int64_t duration);
    void setClientPID(int32_t clientPID);
    void setClientTID(int32_t clientTID);
    void setPriority(int32_t priority);
    void setHandle(int64_t handle);

    virtual ~Message() {}
};

#endif
