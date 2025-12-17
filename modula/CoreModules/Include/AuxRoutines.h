// Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
// SPDX-License-Identifier: BSD-3-Clause-Clear

#ifndef AUX_ROUTINES_H
#define AUX_ROUTINES_H

#include <string>
#include <cstring>
#include <sstream>
#include <fstream>
#include <unistd.h>
#include <getopt.h>

#include "Logger.h"
#include "Request.h"
#include "Signal.h"
#include "UrmSettings.h"

class AuxRoutines {
public:
    static std::string readFromFile(const std::string& fileName);
    static void writeToFile(const std::string& fileName, const std::string& value);
    static void deleteFile(const std::string& fileName);
    static void writeSysFsDefaults();
    static int8_t fileExists(const std::string& filePath);
    static int32_t createProcess();
    static std::string getMachineName();

    static void dumpRequest(Request* request);
    static void dumpRequest(Signal* signal);

    static int64_t generateUniqueHandle();
    static int64_t getCurrentTimeInMilliseconds();
};

#endif
