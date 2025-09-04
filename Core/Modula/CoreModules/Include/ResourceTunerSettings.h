// Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
// SPDX-License-Identifier: BSD-3-Clause-Clear

#ifndef SYSTEM_PROPS_H
#define SYSTEM_PROPS_H

#include <cstdint>
#include <unordered_map>
#include <shared_mutex>
#include <atomic>

#include "MemoryPool.h"
#include "SafeOps.h"
#include "Utils.h"

#define REQ_BUFFER_SIZE 1024

// Operational Tunable Parameters for Resource Tuner
typedef struct {
    uint32_t mMaxConcurrentRequests;
    uint32_t mMaxResourcesPerRequest;
    uint32_t mListeningPort;
    uint32_t mPulseDuration;
    uint32_t mClientGarbageCollectorDuration;
    uint32_t mDelta;
    uint32_t mCleanupBatchSize;
    double mPenaltyFactor;
    double mRewardFactor;
} MetaConfigs;

typedef struct {
    std::string targetName;
    int32_t totalCoreCount;
    int32_t totalClusterCount;
    // Determine whether the system is in Display On or Off / Doze Mode
    // This needs to be tracked, so that only those Requests for which background Processing
    // is Enabled can be processed during Display Off / Doze.
    uint8_t currMode;
} TargetConfigs;

class ResourceTunerSettings {
private:
    static int32_t serverOnlineStatus;

public:
    static const int32_t desiredThreadCount = 10;
    static const int32_t maxPendingQueueSize = 12;
    static const int32_t maxScalingCapacity = 25;

    static const std::string mCommonResourceFilePath;
    static const std::string mCustomResourceFilePath;
    static const std::string mCommonSignalFilePath;
    static const std::string mCustomSignalFilePath;
    static const std::string mCommonPropertiesFilePath;
    static const std::string mCustomPropertiesFilePath;
    static const std::string mCommonTargetFilePath;
    static const std::string mCustomTargetFilePath;

    // Only Custom Config is supported for Ext Features Config
    static const std::string mCustomExtFeaturesFilePath;

    // Support both versions: Common and Custom
    static const std::string mCommonInitConfigFilePath;
    static const std::string mCustomInitConfigFilePath;

    static const std::string mExtensionsPluginLibPath;

    static const std::string mBaseCGroupPath;

    static const std::string mTestResourceFilePath;
    static const std::string mTestSignalFilePath;
    static const std::string mTestPropertiesFilePath;
    static const std::string mTestTargetConfigFilePath;
    static const std::string mTestInitConfigFilePath;

    static std::shared_timed_mutex mModeLock;
    static MetaConfigs metaConfigs;
    static TargetConfigs targetConfigs;

    static int32_t isServerOnline();
    static void setServerOnlineStatus(int32_t isOnline);
};

#endif
