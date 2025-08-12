// Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
// SPDX-License-Identifier: BSD-3-Clause-Clear

/*!
 * \file  PulseMonitor.h
 */

/*!
 * \ingroup  PULSE_MONITOR
 * \defgroup PULSE_MONITOR Pulse Monitor
 * \details Runs as a Daemon Thread and Periodically (Every 60 seconds) checks if any of the Clients with
 *          Active or Pending Requests with the Resource Tuner Server have died or terminated.
 *          When such a Client is Found it is added to the Garbage Collector Queue, so that it
 *          can be cleaned up.\n\n
 *          Pulse Monitor Flow:\n\n
 *          1) The Pulse Monitor, retrieves the list of Clients (i.e. clients with Outstanding Requests)
 *             from the ClientDataManager.\n\n
 *          2) Next, it checks if the /proc/<pid>/status file exists for this Process or not. If it does
 *             not exist, it indicates that the Process has been terminated.\n\n
 *          3) If it detects a Dead Client, the Pulse Monitor adds it to the Garbage Collector Queue,
 *             for further cleanup (Refer ClientGarbageCollector for more details regarding Cleanup).\n\n
 *
 * @{
 */

#ifndef PULSE_MONITOR_H
#define PULSE_MONITOR_H

#include <mutex>
#include <dirent.h>

#include "Timer.h"
#include "RequestManager.h"
#include "CocoTable.h"
#include "ClientDataManager.h"
#include "ClientGarbageCollector.h"
#include "ResourceTunerSettings.h"
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

/*! @} */
