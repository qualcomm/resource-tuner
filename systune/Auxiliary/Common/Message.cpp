// Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
// SPDX-License-Identifier: BSD-3-Clause-Clear

#include "Message.h"

int8_t Message::getRequestType() {
    return this->mReqType;
}

int64_t Message::getHandle() {
    return this->mHandle;
}

int32_t Message::getClientPID() {
    return this->mClientPID;
}

int32_t Message::getClientTID() {
    return this->mClientTID;
}

int32_t Message::getPriority() {
    return this->mPriority;
}

int64_t Message::getDuration() {
    return this->mDuration;
}

void Message::setRequestType(int8_t reqType) {
    this->mReqType = reqType;
}

void Message::setHandle(int64_t handle) {
    this->mHandle = handle;
}

void Message::setDuration(int64_t duration) {
    this->mDuration = duration;
}

void Message::setClientPID(int32_t clientPid) {
    this->mClientPID = clientPid;
}

void Message::setClientTID(int32_t clientTid) {
    this->mClientTID = clientTid;
}

void Message::setPriority(int32_t priority) {
    this->mPriority = priority;
}
