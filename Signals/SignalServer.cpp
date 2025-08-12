// Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
// SPDX-License-Identifier: BSD-3-Clause-Clear

#include "SysSignalsInternal.h"
#include "SignalServerPrivate.h"
#include "SignalRegistry.h"
#include "SignalQueue.h"
#include "Extensions.h"
#include "ComponentRegistry.h"

static std::thread signalServerProcessorThread;

ErrCode initSignals() {
    ErrCode opStatus = RC_SUCCESS;

    // Pre-Allocate memory for commonly used types.
    int32_t concurrentRequestsUB = ResourceTunerSettings::metaConfigs.mMaxConcurrentRequests;
    int32_t resourcesPerRequestUB = ResourceTunerSettings::metaConfigs.mMaxResourcesPerRequest;

    MakeAlloc<Signal> (concurrentRequestsUB);
    MakeAlloc<std::vector<Resource*>> (concurrentRequestsUB * resourcesPerRequestUB);
    MakeAlloc<std::vector<uint32_t>> (concurrentRequestsUB * resourcesPerRequestUB);

    LOGI("RTN_SERVER_INIT", "Parsing Signal Configs");
    SignalConfigProcessor signalConfigProcessor(Extensions::getSignalsConfigFilePath());
    opStatus = signalConfigProcessor.parseSignalConfigs();
    if(RC_IS_NOTOK(opStatus)) {
        LOGE("RTN_SERVER_INIT", "Signal Configs Parsing Failed");
        return opStatus;
    }
    LOGI("RTN_SERVER_INIT", "Signal Configs Successfully Parsed");

    SignalRegistry::getInstance()->displaySignals();

    // Later: Parse Extension Features Configs here

    // Create one thread:
    // - Signal Server thread
    // signalServerProcessorThread = std::thread(SignalsdServerThread);

    // Wait for the thread to initialize
    std::this_thread::sleep_for(std::chrono::milliseconds(300));

    return RC_SUCCESS;
}

ErrCode terminateSignals() {
    // Terminate SysSignals
    // Check if the thread is joinable, to prevent undefined behaviour
    if(signalServerProcessorThread.joinable()) {
        SignalQueue::getInstance()->forcefulAwake();
        signalServerProcessorThread.join();
    } else {
        LOGE("RTN_SERVER_TERMINATION", "Signal server thread is not joinable");
    }
    return RC_SUCCESS;
}

RTN_REGISTER_MODULE(MOD_SYSSIGNAL,
                    initSignals,
                    terminateSignals,
                    submitSignalRequest);
