// Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
// SPDX-License-Identifier: BSD-3-Clause-Clear

#ifndef SIGNAL_CONFIG_PROCESSOR_H
#define SIGNAL_CONFIG_PROCESSOR_H

#include <iostream>
#include <memory>

#include "JsonParser.h"
#include "SignalRegistry.h"

#define SIGNAL_CONFIGS_FILE  "/etc/systune/signalConfigs.json"
#define SIGNAL_CONFIGS_ROOT "SignalConfigs"

#define SIGNAL_SIGID "SigId"
#define SIGNAL_CATEGORY "Category"
#define SIGNAL_NAME "Name"
#define SIGNAL_TIMEOUT "Timeout"
#define SIGNAL_ENABLE "Enable"
#define SIGNAL_TARGETS_ENABLED "TargetsEnabled"
#define SIGNAL_TARGETS_DISABLED "TargetsDisabled"
#define SIGNAL_PERMISSIONS "Permissions"
#define SIGNAL_DERIVATIVES "Derivatives"
#define SIGNAL_RESOURCES "Resources"

class SignalConfigProcessor {
private:
    JsonParser* mJsonParser;
    std::string mSignalConfigJsonFilePath;
    int8_t mCustomSignalsFileSpecified;

    void SignalConfigsParserCB(const Json::Value& item);

public:
    SignalConfigProcessor(std::string jsonFilePath);
    ~SignalConfigProcessor();

    ErrCode parseSignalConfigs();
};

#endif
