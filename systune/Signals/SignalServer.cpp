// Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
// SPDX-License-Identifier: BSD-3-Clause-Clear

#include "SysSignalsInternal.h"
#include "SignalServerPrivate.h"
#include "SignalRegistry.h"
#include "SignalQueue.h"
#include "Extensions.h"
#include "ComponentRegistry.h"

static std::thread signalServerProcessorThread;

ErrCode initSysSignals() {
    ErrCode opStatus = RC_SUCCESS;

    // Pre-Allocate memory for commonly used types.
    int32_t concurrentRequestsUB = SystuneSettings::metaConfigs.mMaxConcurrentRequests;
    int32_t resourcesPerRequestUB = SystuneSettings::metaConfigs.mMaxResourcesPerRequest;

    MakeAlloc<Signal> (concurrentRequestsUB);
    MakeAlloc<std::vector<Resource*>> (concurrentRequestsUB * resourcesPerRequestUB);
    MakeAlloc<std::vector<uint32_t>> (concurrentRequestsUB * resourcesPerRequestUB);

    SignalConfigProcessor signalConfigProcessor(Extensions::getSignalsConfigFilePath());

    opStatus = signalConfigProcessor.parseSignalConfigs();
    if(RC_IS_NOTOK(opStatus)) {
        LOGE("URM_SIGNALS_SERVER", "Signal Configs Parsing Failed");
        return opStatus;
    }

    SignalRegistry::getInstance()->displaySignals();

    // Create one thread:
    // - Signal Server thread
    signalServerProcessorThread = std::thread(SignalsdServerThread);

    // Wait for the thread to initialize
    std::this_thread::sleep_for(std::chrono::milliseconds(300));

    return opStatus;
}

ErrCode terminateSysSignals() {
    // Terminate SysSignals
    // Check if the thread is joinable, to prevent undefined behaviour
    if(signalServerProcessorThread.joinable()) {
        SignalQueue::getInstance()->forcefulAwake();
        signalServerProcessorThread.join();
    } else {
        LOGE("URM_SYSSIGNA_SERVER", "Signal server thread is not joinable");
    }
    return RC_SUCCESS;
}

URM_REGISTER_MODULE(MOD_SYSSIGNAL,
                    initSysSignals,
                    terminateSysSignals,
                    submitSignalRequest);
