// Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
// SPDX-License-Identifier: BSD-3-Clause-Clear

#ifndef RATE_LIMITER_H
#define RATE_LIMITER_H

#include <mutex>
#include <shared_mutex>
#include <memory>

#include "RequestManager.h"
#include "ClientDataManager.h"
#include "SystuneSettings.h"

/**
 * @brief This class inherits from request Map and is solely responsible for checking
 *        if a client violates pre-set rate limits.
 */
class RateLimiter {

private:
    static std::shared_ptr<RateLimiter> mRateLimiterInstance;
    static std::mutex instanceProtectionLock;
    std::shared_timed_mutex mRateLimiterMutex;

    uint32_t mDelta;
    double mPenaltyFactor;
    double mRewardFactor;
    int8_t shouldBeProcessed(int32_t clientPID);

    RateLimiter();

public:
    /**
    * @brief Checks if rate limit is honored.
    * @details Uses clientAccess struct to store the last timestamp of the request
    *          and checks if the new request is within a certain delta. For every
    *          violation, client health is punished and for every non-violation, it is
    *          rewarded by a smaller factor than the punishment.
    * @param clientTID TID of the client
    * @return int8_t
    *            1: if the request was successfully added
    *            0: otherwise
    */
    int8_t isRateLimitHonored(int32_t clientTID);

    static std::shared_ptr<RateLimiter> getInstance() {
        if(mRateLimiterInstance == nullptr) {
            instanceProtectionLock.lock();
            if(mRateLimiterInstance == nullptr) {
                try {
                    mRateLimiterInstance = std::shared_ptr<RateLimiter> (new RateLimiter());
                } catch(const std::bad_alloc& e) {
                    instanceProtectionLock.unlock();
                    return nullptr;
                }
            }
            instanceProtectionLock.unlock();
        }
        return mRateLimiterInstance;
    }
};

#endif
