// Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
// SPDX-License-Identifier: BSD-3-Clause-Clear

#include "RateLimiter.h"

std::shared_ptr<RateLimiter> RateLimiter::mRateLimiterInstance = nullptr;
std::mutex RateLimiter::instanceProtectionLock{};

RateLimiter::RateLimiter() {
    this->mDelta = SystuneSettings::metaConfigs.mDelta;
    this->mPenaltyFactor = SystuneSettings::metaConfigs.mPenaltyFactor;
    this->mRewardFactor = SystuneSettings::metaConfigs.mRewardFactor;
}

int8_t RateLimiter::shouldBeProcessed(int32_t clientTID) {
    this->mRateLimiterMutex.lock();

    double health = ClientDataManager::getInstance()->getHealthByClientID(clientTID);
    if(health <= 0) {
        // Repeat offender, total block
        this->mRateLimiterMutex.unlock();
        return false;
    }

    int64_t currentMillis = SystuneSettings::getCurrentTimeInMilliseconds();
    // If this is the First Request, don't update the Health
    if(ClientDataManager::getInstance()->getLastRequestTimestampByClientID(clientTID) != 0) {
        int64_t requestDelta = currentMillis - ClientDataManager::getInstance()->getLastRequestTimestampByClientID(clientTID);

        if(requestDelta < mDelta) {
            health -= mPenaltyFactor;
        } else {
            // Increase in health
            health = std::min(100.0, health + this->mRewardFactor);
        }
    }

    ClientDataManager::getInstance()->updateHealthByClientID(clientTID, health);

    // Check if client health is positive
    if(health <= 0) {
        this->mRateLimiterMutex.unlock();
        return false;
    }

    // Request can be accepted
    ClientDataManager::getInstance()->updateLastRequestTimestampByClientID(clientTID, currentMillis);
    this->mRateLimiterMutex.unlock();

    return true;
}

int8_t RateLimiter::isRateLimitHonored(int32_t clientTID) {
    return shouldBeProcessed(clientTID);
}
