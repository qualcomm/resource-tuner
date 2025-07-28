// Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
// SPDX-License-Identifier: BSD-3-Clause-Clear

#ifndef SIGNAL_H
#define SIGNAL_H

#include <cstdint>

#include "Types.h"

class Signal : public Message {
private:
    uint32_t mSignalID;
    const char* mAppName;
    const char* mScenario;
    int32_t mNumArgs;
    std::vector<uint32_t>* mListArgs;

public:
    Signal();
    ~Signal();

    uint32_t getSignalID();
    int32_t getNumArgs();
    const char* getAppName();
    const char* getScenario();
    std::vector<uint32_t>* getListArgs();
    uint32_t getListArgAt(int32_t index);

    void setSignalID(uint32_t signalID);
    void setAppName(const char* appName);
    void setScenario(const char* scenario);
    void setNumArgs(int32_t numArgs);
    void setList(std::vector<uint32_t>* mListArgs);
};

#endif
