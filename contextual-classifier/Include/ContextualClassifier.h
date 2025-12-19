// Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
// SPDX-License-Identifier: BSD-3-Clause-Clear

#ifndef CONTEXTUAL_CLASSIFIER_H
#define CONTEXTUAL_CLASSIFIER_H

#include "ComponentRegistry.h"
#include "MLInference.h"
#include "NetLinkComm.h"
#include <condition_variable>
#include <mutex>
#include <queue>
#include <string>
#include <thread>
#include <unordered_map>
#include <unordered_set>
#include <vector>

class ContextualClassifier {
  public:
    ContextualClassifier();
    ~ContextualClassifier();
    ErrCode Init();
    ErrCode Terminate();

  private:
    void WorkerThread();
    int HandleProcEv();
    void ClassifyProcess(int pid, int tgid,
                         std::unordered_map<int, int> &pid_perf_handle);
    void LoadIgnoredProcesses();
    void RemoveActions(int pid, int tgid,
                       std::unordered_map<int, int> &pid_perf_handle);

    NetLinkComm mNetLinkComm;
    MLInference mMLInference;

    // FIFO queue + pending set for PID lifecycle management
    std::queue<int> mClassificationQueue; // FIFO of PIDs
    std::unordered_set<int> mPendingPids; // PIDs still pending/alive
    std::mutex mQueueMutex;
    std::condition_variable mQueueCond;
    std::thread mWorkerThread;
    std::thread mNetlinkThread;
    volatile bool mNeedExit = false;

    std::unordered_set<std::string> mIgnoredProcesses;
    std::unordered_map<std::string, std::unordered_set<std::string>>
        mTokenIgnoreMap;
    bool mDebugMode = false;
};

#endif // CONTEXTUAL_CLASSIFIER_H
