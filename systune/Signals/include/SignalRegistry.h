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

/**
 * @struct SignalInfo
 * @brief Representation of a single Signal Configuration
 * @details This information is read from the Config files.\n
 *          Note this (SignalInfo) struct is separate from the Signal struct.
 */
typedef struct {
    /**
     * @brief 16-bit Signal Opcode
     */
    int16_t mSignalOpId;

    /**
     * @brief Category of the Signal
     */
    int8_t mSignalCategory;

    /**
     * @brief Signal Name, for ex: EARLY_WAKEUP
     */
    std::string mSignalName;

    /**
     * @brief Boolean flag which is set if Signal is available for Provisioning.
     */
    int8_t mIsEnabled;

    /**
     * @brief Default Signal Timeout, to be used if Client specifies aduration
     *        of 0 in the tuneSignal API call.
     */
    int32_t mTimeout;

    /**
     * @brief Pointer to a vector, storing the list of targets for
     *        which the signal is enabled.
     */
    std::unordered_set<std::string>* mTargetsEnabled;

    /**
     * @brief Pointer to a vector, storing the list of targets for which the
     *        signal is not eligible for Provisioning.
     */
    std::unordered_set<std::string>* mTargetsDisabled;

    /**
     * @brief Pointer to a list of Permissions, i.e. only Clients with one of
     *        these permissions can provision the signal.
     */
    std::vector<enum Permissions>* mPermissions;

    std::vector<std::string>* mDerivatives;

    /**
     * @brief List of Actual Resource Opcodes (which will be Provisioned) and the
     *        Values to be configured for the Resources.
     */
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

    SignalInfoBuilder* setOpID(const std::string& signalOpIdString);
    SignalInfoBuilder* setCategory(const std::string& categoryString);
    SignalInfoBuilder* setName(const std::string& signalName);
    SignalInfoBuilder* setTimeout(int32_t timeout);
    SignalInfoBuilder* setIsEnabled(int8_t isEnabled);
    SignalInfoBuilder* addTarget(int8_t isEnabled, const std::string& target);
    SignalInfoBuilder* addPermission(const std::string& permissionString);
    SignalInfoBuilder* addDerivative(const std::string& derivative);
    SignalInfoBuilder* addLock(uint32_t lockId);

    SignalInfo* build();
};

#endif
