// Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
// SPDX-License-Identifier: BSD-3-Clause-Clear

#ifndef SIGNAL_H
#define SIGNAL_H

#include <cstdint>
#include <string>

#include "SafeOps.h"
#include "Logger.h"
#include "MemoryPool.h"

/**
* @brief Encapsulation type for a Signal Tuning Request.
*/
class Signal : public Message {
private:
    uint32_t mSignalOpCode;  //!< ID of the Signal to be Tuned to be tuned as Part of the Request
    std::string mAppName;
    std::string mScenario;
    int32_t mNumArgs; //!< Number of Additional Args
    std::vector<uint32_t>* mListArgs; //!< Pointer to a list, storing the additional args.

public:
    Signal();
    ~Signal();

    uint32_t getSignalID();
    int32_t getNumArgs();
    const std::string getAppName();
    const std::string getScenario();
    uint32_t getListArgAt(int32_t index);
    std::vector<uint32_t>* getListArgs();

    void setSignalOpCode(uint32_t signalID);
    void setAppName(const std::string& appName);
    void setScenario(const std::string& scenario);
    void setNumArgs(int32_t numArgs);
    void setList(std::vector<uint32_t>* mListArgs);

    ErrCode serialize(char* buf);
    ErrCode deserialize(char* buf);

    static void cleanUpSignal(Signal* signal);
};

#endif
