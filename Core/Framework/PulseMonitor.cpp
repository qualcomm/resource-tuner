// Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
// SPDX-License-Identifier: BSD-3-Clause-Clear

#include "PulseMonitor.h"

// retuns pid of all running process right now
static int8_t getAllRunningProcess(std::unordered_set<int64_t>& processId) {
    DIR* proc = opendir("/proc");
    struct dirent* ent;

    if(proc == nullptr) {
        TYPELOGV(ERRNO_LOG, "opendir", strerror(errno));
        return -1;
    }

    while(ent = readdir(proc)) {
        if(!isdigit(*ent->d_name)) {
            continue;
        }
        int64_t tgid = strtol(ent->d_name, nullptr, 10);
        processId.insert(tgid);
    }
    closedir(proc);

    return 0;
}

std::shared_ptr<PulseMonitor> PulseMonitor::mPulseMonitorInstance = nullptr;

PulseMonitor::PulseMonitor() {
    this->mTimer = nullptr;
    this->mPulseDuration = ResourceTunerSettings::metaConfigs.mPulseDuration;
}

// Check for optimizations
int8_t PulseMonitor::checkForDeadClients() {
    // stores pid of all the running process right now
    std::unordered_set<int64_t> processIds;
    std::vector<int32_t> clientList;

    if(getAllRunningProcess(processIds) < 0) {
        return -1;
    }

    // This method will internally acquire a shared lock on the table.
    ClientDataManager::getInstance()->getActiveClientList(clientList);

    // Delete the clients if they are dead.
    for(int32_t pid: clientList) {
        if(processIds.find(pid) == processIds.end()) {
            // Client is dead, Schedule it for deletion.
            LOGD("RESTUNE_PULSE_MONITOR", "Client with PID: " + std::to_string(pid) + " is dead.");
            ClientGarbageCollector::getInstance()->submitClientThreadsForCleanup(pid);
            ClientDataManager::getInstance()->deleteClientPID(pid);
        }
    }

    return 0;
}

ErrCode PulseMonitor::startPulseMonitorDaemon() {
    try {
        this->mTimer = new (GetBlock<Timer>())
                            Timer(std::bind(&PulseMonitor::checkForDeadClients, this), true);

    } catch(const std::bad_alloc& e) {
        return RC_MEMORY_ALLOCATION_FAILURE;

    } catch(const std::exception& e) {
        return RC_MEMORY_ALLOCATION_FAILURE;
    }

    if(!this->mTimer->startTimer(this->mPulseDuration)) {
        return RC_WORKER_THREAD_ASSIGNMENT_FAILURE;
    }

    LOGI("RESTUNE_PULSE_MONITOR", "Pulse Monitor Daemon Thread Started");
    return RC_SUCCESS;
}

void PulseMonitor::stopPulseMonitorDaemon() {
    if(this->mTimer != nullptr) {
        this->mTimer->killTimer();
    }
}

PulseMonitor::~PulseMonitor() {
    if(this->mTimer != nullptr) {
        FreeBlock<Timer>(this->mTimer);
        this->mTimer = nullptr;
    }
}

ErrCode startPulseMonitorDaemon() {
    if(PulseMonitor::getInstance() == nullptr) {
        return RC_MEMORY_ALLOCATION_FAILURE;
    }
    return PulseMonitor::getInstance()->startPulseMonitorDaemon();
}

void stopPulseMonitorDaemon() {
    if(PulseMonitor::getInstance() == nullptr) {
        return;
    }
    return PulseMonitor::getInstance()->stopPulseMonitorDaemon();
}
