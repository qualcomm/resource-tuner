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

// Operational Tunable Parameters for Resource Tuner
typedef struct {
    uint32_t mMaxConcurrentRequests;
    uint32_t mMaxResourcesPerRequest;
    uint32_t mListeningPort;
    uint32_t mPulseDuration;
    uint32_t mClientGarbageCollectorDuration;
    uint32_t mDelta;
    double mPenaltyFactor;
    double mRewardFactor;
} MetaConfigs;

typedef struct {
    std::string targetName;
    uint8_t totalCoreCount;
    uint8_t totalClusterCount;
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

    // Support both versions: Common and Custom
    static const std::string mCommonResourceFilePath;
    static const std::string mCustomResourceFilePath;

    // Support both versions: Common and Custom
    static const std::string mCommonSignalFilePath;
    static const std::string mCustomSignalFilePath;

    // Only Support Common
    static const std::string mInitConfigFilePath;
    static const std::string mPropertiesFilePath;

    // Only Custom Config is supported for Ext Features Config
    static const std::string mCustomExtFeaturesFilePath;
    // Only Custom Config is supported for Target Config
    static const std::string mCustomTargetFilePath;

    static const std::string mBaseCGroupPath;

    static const std::string mTestResourceFilePath;
    static const std::string mTestSignalFilePath;
    static const std::string mTestPropertiesFilePath;
    static const std::string mTestTargetConfigFilePath;

    static std::shared_timed_mutex mModeLock;
    static MetaConfigs metaConfigs;
    static TargetConfigs targetConfigs;
    static int8_t serverInTestMode;

    static int32_t isServerOnline();
    static void setServerOnlineStatus(int32_t isOnline);

    static int64_t generateUniqueHandle();
    static int64_t getCurrentTimeInMilliseconds();
};

#endif
