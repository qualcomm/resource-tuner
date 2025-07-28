// Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
// SPDX-License-Identifier: BSD-3-Clause-Clear

#ifndef SIGNAL_REGISTRY_H
#define SIGNAL_REGISTRY_H

#include <vector>
#include <memory>
#include <unordered_map>
#include <unordered_set>
#include <algorithm>

#include "Utils.h"
#include "Logger.h"
#include "MemoryPool.h"

typedef struct {
    int16_t mSignalOpId;
    int8_t mSignalCategory;
    std::string mSignalName;
    int8_t mIsEnabled;
    int32_t mTimeout;
    std::unordered_set<std::string>* mTargetsEnabled;
    std::unordered_set<std::string>* mTargetsDisabled;
    std::vector<enum Permissions>* mPermissions;
    std::vector<std::string>* mDerivatives;
    std::vector<uint32_t>* mLocks;
} SignalInfo;

class SignalRegistry {
private:
    static std::shared_ptr<SignalRegistry> signalRegistryInstance;
    static int32_t mTotalSignals;
    std::vector<SignalInfo*> mSignalsConfigs;
    int8_t customerBit;

    std::unordered_map<uint32_t, int32_t> mSystemIndependentLayerMappings;

    SignalRegistry();

public:
    ~SignalRegistry();

    void initRegistry(int32_t size, int8_t customerBit);

    void registerSignal(SignalInfo* signalInfo);

    std::vector<SignalInfo*> getSignalConfigs();

    SignalInfo* getSignalConfigById(uint32_t signalID);

    int32_t getSignalsConfigCount();

    void displaySignals();

    static std::shared_ptr<SignalRegistry> getInstance() {
        if(signalRegistryInstance == nullptr) {
            try {
                signalRegistryInstance = std::shared_ptr<SignalRegistry>(new SignalRegistry());
            } catch (const std::bad_alloc& e) {
                LOGE("URM_SIGNAL_REGISTRY",
                     "Failed to allocate memory for SignalRegistry instance: " + std::string(e.what()));
                return nullptr;
            }
        }
        return signalRegistryInstance;
    }
};

class SignalInfoBuilder {
private:
    SignalInfo* mSignalInfo;

public:
    SignalInfoBuilder();

    SignalInfoBuilder* setOpID(std::string signalOpIdString);
    SignalInfoBuilder* setCategory(std::string categoryString);
    SignalInfoBuilder* setName(std::string signalName);
    SignalInfoBuilder* setTimeout(int32_t timeout);
    SignalInfoBuilder* setIsEnabled(int8_t isEnabled);
    SignalInfoBuilder* addTarget(int8_t isEnabled, std::string target);
    SignalInfoBuilder* addPermission(std::string permissionString);
    SignalInfoBuilder* addDerivative(std::string derivative);
    SignalInfoBuilder* addLock(uint32_t lockId);

    SignalInfo* build();
};

#endif
