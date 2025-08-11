// Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
// SPDX-License-Identifier: BSD-3-Clause-Clear

#ifndef SERVER_UTILS_H
#define SERVER_UTILS_H

#include <sys/utsname.h>

#include "MemoryPool.h"
#include "SysConfigInternal.h"

void preAllocateMemory() {
    // Preallocate Memory for certain frequently used types.
    int32_t concurrentRequestsUB = ResourceTunerSettings::metaConfigs.mMaxConcurrentRequests;
    int32_t resourcesPerRequestUB = ResourceTunerSettings::metaConfigs.mMaxResourcesPerRequest;

    MakeAlloc<Message> (concurrentRequestsUB);
    MakeAlloc<Request> (concurrentRequestsUB);
    MakeAlloc<Timer> (concurrentRequestsUB);
    MakeAlloc<Resource> (concurrentRequestsUB * resourcesPerRequestUB);
    MakeAlloc<CocoNode> (concurrentRequestsUB * resourcesPerRequestUB);
    MakeAlloc<SysConfig> (concurrentRequestsUB);
    MakeAlloc<ClientInfo> (concurrentRequestsUB * resourcesPerRequestUB);
    MakeAlloc<ClientTidData> (concurrentRequestsUB * resourcesPerRequestUB);
    MakeAlloc<std::unordered_set<int64_t>> (concurrentRequestsUB * resourcesPerRequestUB);
    MakeAlloc<std::vector<Resource*>> (concurrentRequestsUB * resourcesPerRequestUB);
    MakeAlloc<std::vector<int32_t>> (concurrentRequestsUB * resourcesPerRequestUB);
    MakeAlloc<std::vector<CocoNode*>> (concurrentRequestsUB * resourcesPerRequestUB);
}

ErrCode fetchMetaConfigs() {
    std::string resultBuffer;

    try {
        // Fetch target Name
        struct utsname sysInfo;
        if(uname(&sysInfo) == -1) {
            TYPELOGV(ERRNO_LOG, "uname", strerror(errno));
            return RC_PROP_PARSING_ERROR;
        }

        ResourceTunerSettings::targetConfigs.targetName = sysInfo.nodename;

        sysConfigGetProp("resource_tuner.maximum.concurrent.requests", resultBuffer, sizeof(resultBuffer), "120");
        ResourceTunerSettings::metaConfigs.mMaxConcurrentRequests = (uint32_t)std::stol(resultBuffer);

        sysConfigGetProp("resource_tuner.maximum.resources.per.request", resultBuffer, sizeof(resultBuffer), "5");
        ResourceTunerSettings::metaConfigs.mMaxResourcesPerRequest = (uint32_t)std::stol(resultBuffer);

        sysConfigGetProp("resource_tuner.listening.port", resultBuffer, sizeof(resultBuffer), "12000");
        ResourceTunerSettings::metaConfigs.mListeningPort = (uint32_t)std::stol(resultBuffer);

        sysConfigGetProp("resource_tuner.pulse.duration", resultBuffer, sizeof(resultBuffer), "60000");
        ResourceTunerSettings::metaConfigs.mPulseDuration = (uint32_t)std::stol(resultBuffer);

        sysConfigGetProp("resource_tuner.garbage_collection.duration", resultBuffer, sizeof(resultBuffer), "83000");
        ResourceTunerSettings::metaConfigs.mClientGarbageCollectorDuration = (uint32_t)std::stol(resultBuffer);

        sysConfigGetProp("resource_tuner.rate_limiter.delta", resultBuffer, sizeof(resultBuffer), "5");
        ResourceTunerSettings::metaConfigs.mDelta = (uint32_t)std::stol(resultBuffer);

        sysConfigGetProp("resource_tuner.penalty.factor", resultBuffer, sizeof(resultBuffer), "2.0");
        ResourceTunerSettings::metaConfigs.mPenaltyFactor = std::stod(resultBuffer);

        sysConfigGetProp("resource_tuner.reward.factor", resultBuffer, sizeof(resultBuffer), "0.4");
        ResourceTunerSettings::metaConfigs.mRewardFactor = std::stod(resultBuffer);

        sysConfigGetProp("resource_tuner.logging.level", resultBuffer, sizeof(resultBuffer), "2");
        int8_t logLevel = (int8_t)std::stoi(resultBuffer);

        int8_t levelSpecificLogging = false;
        sysConfigGetProp("resource_tuner.logging.level.exact", resultBuffer, sizeof(resultBuffer), "false");
        if(resultBuffer == "true") {
            levelSpecificLogging = true;
        }

        RedirectOptions redirectOutputTo = LOG_FILE;
        sysConfigGetProp("resource_tuner.logging.redirect_to", resultBuffer, sizeof(resultBuffer), "1");
        if((int8_t)std::stoi(resultBuffer) == 0) {
            redirectOutputTo = FTRACE;
        }

        Logger::configure(logLevel, levelSpecificLogging, redirectOutputTo);

    } catch(const std::invalid_argument& ex) {
        TYPELOGV(META_CONFIG_PARSE_FAILURE, ex.what());
        return RC_PROP_PARSING_ERROR;

    } catch(const std::out_of_range& ex) {
        TYPELOGV(META_CONFIG_PARSE_FAILURE, ex.what());
        return RC_PROP_PARSING_ERROR;
    }

    return RC_SUCCESS;
}

#endif
