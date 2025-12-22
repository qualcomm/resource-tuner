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

enum { CC_IGNORE = 0x00, CC_APP_OPEN = 0x01, CC_APP_CLOSE = 0x02 };

enum {
    CC_BROWSER_APP_OPEN = 0x00,
    CC_GAME_APP_OPEN = 0x01,
    CC_MULTIMEDIA_APP_OPEN = 0x02
};

enum { DEFAULT_CONFIG, PER_APP_CONFIG };

typedef enum CC_TYPE {
    CC_APP = 0x01,
    CC_BROWSER = 0x02,
    CC_GAME = 0x03,
    CC_MULTIMEDIA = 0x04,
    CC_UNKNOWN = 0x05
} CC_TYPE;

struct ProcEvent {
    int pid;
    int tgid;
    int type; // CC_APP_OPEN / CC_APP_CLOSE / CC_IGNORE
};

class ContextualClassifier {
  public:
    ContextualClassifier();
    ~ContextualClassifier();
    ErrCode Init();
    ErrCode Terminate();

  private:
    void ClassifierMain();
    int HandleProcEv();
    int ClassifyProcess(int pid, int tgid, const std::string &comm,
                        uint32_t &ctxDetails);
    void LoadIgnoredProcesses();
    void ApplyActions(std::string comm, int32_t sigId, int32_t sigType);
    void RemoveActions(int pid, int tgid);
    bool isIgnoredProcess(int evType, int pid);

    NetLinkComm mNetLinkComm;
    MLInference mMLInference;

    // Event queue for classifier main thread
    std::queue<ProcEvent> mPendingEv;
    std::mutex mQueueMutex;
    std::condition_variable mQueueCond;
    std::thread mClassifierMain;
    std::thread mNetlinkThread;
    volatile bool mNeedExit = false;

    std::unordered_set<std::string> mIgnoredProcesses;
    std::unordered_map<std::string, std::unordered_set<std::string>>
        mTokenIgnoreMap;
    bool mDebugMode = false;

    std::unordered_set<int> mIgnoredPids;
    std::unordered_map<int, uint64_t> mResTunerHandles;

    void GetSignalDetailsForWorkload(int32_t contextType, uint32_t &sigId,
                                     uint32_t &sigSubtype);
};

#endif // CONTEXTUAL_CLASSIFIER_H
