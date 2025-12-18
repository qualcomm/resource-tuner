// Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
// SPDX-License-Identifier: BSD-3-Clause-Clear

#ifndef CONTEXTUAL_CLASSIFIER_H
#define CONTEXTUAL_CLASSIFIER_H

#include <thread>
#include <vector>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <unordered_set>
#include <unordered_map>
#include <string>
#include <set>
#include "NetLinkComm.h"
#include "MLInference.h"
#include "ComponentRegistry.h" 

class ContextualClassifier {
public:
    ContextualClassifier();
    ~ContextualClassifier();
    ErrCode Init();
    ErrCode Terminate();

private:
    void WorkerThread();
    int HandleProcEv();
    void ClassifyProcess(int pid, int tgid, std::unordered_map<int, int>& pid_perf_handle);
    void LoadIgnoredProcesses();
    void RemoveActions(int pid, int tgid, std::unordered_map<int, int>& pid_perf_handle);

    NetLinkComm mNetLinkComm;
    MLInference mMLInference;

    // Queue + pending set for PID lifecycle management
    std::queue<int> mClassificationQueue;
    std::set<int> mPendingPids;
    std::mutex mQueueMutex;
    std::condition_variable mQueueCond;
    std::thread mWorkerThread;
    std::thread mNetlinkThread;
    volatile bool mNeedExit = false;
    
    std::unordered_set<std::string> mIgnoredProcesses;
    std::unordered_map<std::string, std::unordered_set<std::string>> mTokenIgnoreMap;
    bool mDebugMode = false;
};

#endif // CONTEXTUAL_CLASSIFIER_H
