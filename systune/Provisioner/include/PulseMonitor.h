// Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
// SPDX-License-Identifier: BSD-3-Clause-Clear

#ifndef PULSE_MONITOR_H
#define PULSE_MONITOR_H

#include <mutex>
#include <dirent.h>

#include "Timer.h"
#include "RequestManager.h"
#include "CocoTable.h"
#include "ClientDataManager.h"
#include "ClientGarbageCollector.h"
#include "SystuneSettings.h"
#include "Logger.h"

/**
 * @brief Responsible for checking if all clients are alive after a certain time interval.
 * @details It spawns a background thread which lists all alive processes in the system and
 *          compares them with the client list. If a clientPID doesn't exist in the system,
 *          it is cleaned up.
 */
class PulseMonitor {
private:
    static std::shared_ptr<PulseMonitor> mPulseMonitorInstance;
    Timer* mTimer;
    uint32_t mPulseDuration;

    PulseMonitor();

    int8_t checkForDeadClients();

public:
    ~PulseMonitor();

    ErrCode startPulseMonitorDaemon();

    static std::shared_ptr<PulseMonitor> getInstance() {
        if(mPulseMonitorInstance == nullptr) {
            mPulseMonitorInstance = std::shared_ptr<PulseMonitor>(new PulseMonitor());
        }
        return mPulseMonitorInstance;
    }
};

ErrCode startPulseMonitorDaemon();

#endif
