// Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
// SPDX-License-Identifier: BSD-3-Clause-Clear

#ifndef SYSTUNE_MESSAGE_H
#define SYSTUNE_MESSAGE_H

#include <cstdint>

/**
* @brief Base-Type for Request and Signal classes.
*/
class Message {
protected:
    int8_t mReqType; //!< Type of the request. Possible values: TUNE, UNTUNE, RETUNE, TUNESIGNAL, FREESIGNAL.
    int64_t mHandle; //!< The unique generated handle for the request.
    int64_t mDuration; //!< Duration. -1 means infinite duration.
    int32_t mProperties; //!< Request Properties, includes Priority and Background Processing Status.
    int32_t mClientPID; //!< Process ID of the client making the request.
    int32_t mClientTID; //!< Thread ID of the client making the request.

public:
    Message() : mProperties(0) {}

    int8_t getRequestType();
    int64_t getDuration();
    int32_t getClientPID();
    int32_t getClientTID();
    int64_t getHandle();
    int8_t getPriority();
    int8_t isBackgroundProcessingEnabled();
    int32_t getProperties();

    void setRequestType(int8_t reqType);
    void setDuration(int64_t duration);
    void setClientPID(int32_t clientPID);
    void setClientTID(int32_t clientTID);
    void setProperties(int32_t properties);
    void setPriority(int8_t priority);
    void setHandle(int64_t handle);
    void setBackgroundProcessing(int8_t backgroundProcessing);

    virtual ~Message() {}
};

#endif
