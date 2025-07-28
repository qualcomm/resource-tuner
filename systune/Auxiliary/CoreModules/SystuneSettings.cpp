// Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
// SPDX-License-Identifier: BSD-3-Clause-Clear

#include "SystuneSettings.h"

std::shared_timed_mutex SystuneSettings::mModeLock{};
int32_t SystuneSettings::serverOnlineStatus = false;
int8_t SystuneSettings::serverInTestMode = false;
MetaConfigs SystuneSettings::metaConfigs{};
TargetConfigs SystuneSettings::targetConfigs{};

int32_t SystuneSettings::isServerOnline() {
    return serverOnlineStatus;
}

void SystuneSettings::setServerOnlineStatus(int32_t isOnline) {
    serverOnlineStatus = isOnline;
}

int64_t SystuneSettings::generateUniqueHandle() {
    static int64_t handleGenerator = 0;
    OperationStatus opStatus;
    int64_t nextHandle = Add(handleGenerator, (int64_t)1, opStatus);
    if(opStatus == SUCCESS) {
        handleGenerator = nextHandle;
        return nextHandle;
    }
    return -1;
}

int64_t SystuneSettings::getCurrentTimeInMilliseconds() {
    auto now = std::chrono::system_clock::now();
    return std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()).count();
}
