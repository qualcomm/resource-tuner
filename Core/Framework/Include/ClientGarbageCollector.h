// Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
// SPDX-License-Identifier: BSD-3-Clause-Clear

#ifndef CLIENT_GARBAGE_COLLECTOR_H
#define CLIENT_GARBAGE_COLLECTOR_H

#define BATCH_SIZE 5

#include <cstdint>
#include <memory>
#include <queue>
#include <mutex>

#include "Timer.h"
#include "RequestQueue.h"
#include "RequestManager.h"

class ClientGarbageCollector {
private:
    static std::shared_ptr<ClientGarbageCollector> mClientGarbageCollectorInstance;

    std::mutex mGcQueueMutex;
    Timer* mTimer;
    uint32_t mGarbageCollectionDuration;
    std::queue<int32_t> mGcQueue;

    ClientGarbageCollector();

    void performCleanup();

public:
    ~ClientGarbageCollector();

    void submitClientThreadsForCleanup(int32_t clientTid);

    ErrCode startClientGarbageCollectorDaemon();

    static std::shared_ptr<ClientGarbageCollector> getInstance() {
        if(mClientGarbageCollectorInstance == nullptr) {
            mClientGarbageCollectorInstance = std::shared_ptr<ClientGarbageCollector> (new ClientGarbageCollector());
        }
        return mClientGarbageCollectorInstance;
    }
};

ErrCode startClientGarbageCollectorDaemon();

#endif
