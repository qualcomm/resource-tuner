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
#include "Resource.h"
#include "MemoryPool.h"

/**
 * @struct SignalInfo
 * @brief Representation of a single Signal Configuration
 * @details This information is read from the Config files.\n
 *          Note this (SignalInfo) struct is separate from the Signal struct.
 */
typedef struct {
    /**
     * @brief 16-bit Signal ID
     */
    uint16_t mSignalID;

    /**
     * @brief Category of the Signal
     */
    uint8_t mSignalCategory;

    /**
     * @brief Signal Name, for ex: EARLY_WAKEUP
     */
    std::string mSignalName;

    /**
     * @brief Boolean flag which is set if Signal is available for Provisioning.
     */
    int8_t mIsEnabled;

    /**
     * @brief Default Signal Timeout, to be used if Client specifies a duration
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
     * @brief List of Actual Resource which will be Provisioned and the
     *        Values to be configured for the Resources.
     */
    std::vector<Resource*>* mSignalResources;

} SignalInfo;

/**
 * @brief SignalRegistry
 * @details Stores information Relating to all the Signals available for Tuning.
 *          Note: This information is extracted from Config YAML files.
 */
class SignalRegistry {
private:
    static std::shared_ptr<SignalRegistry> signalRegistryInstance;
    int32_t mTotalSignals;
    std::vector<SignalInfo*> mSignalsConfigs;

    std::unordered_map<uint32_t, int32_t> mSystemIndependentLayerMappings;

    SignalRegistry();

public:
    ~SignalRegistry();

    /**
     * @brief Used to register a Config specified (through YAML) Signal with Resource Tuner
     * @details The Signal Info is parsed from YAML files. If the SignalInfo provided is
     *          Malformed, then it will be freed as part of this routine, else it will
     *          be added to the "mSignalsConfigs" vector.
     */
    void registerSignal(SignalInfo* signalInfo, int8_t isBuSpecified=false);

    int8_t isSignalConfigMalformed(SignalInfo* sConf);

    std::vector<SignalInfo*> getSignalConfigs();

   /**
    * @brief Get the SignalInfo object corresponding to the given Resource ID.
    * @param resourceId An unsigned 32 bit integer, representing the Signal ID.
    * @return SignalInfo*:\n
    *          - A pointer to the SignalInfo object
    *          - nullptr, if no SignalInfo object with the given Signal ID exists.
    */
    SignalInfo* getSignalConfigById(uint32_t signalCode);

    int32_t getSignalsConfigCount();
    int32_t getSignalTableIndex(uint32_t signalCode);
    void displaySignals();

    static std::shared_ptr<SignalRegistry> getInstance() {
        if(signalRegistryInstance == nullptr) {
            try {
                signalRegistryInstance = std::shared_ptr<SignalRegistry>(new SignalRegistry());
            } catch(const std::bad_alloc& e) {
                LOGE("RESTUNE_SIGNAL_REGISTRY",
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
    ~SignalInfoBuilder();

    ErrCode setSignalID(const std::string& signalOpIdString);
    ErrCode setSignalCategory(const std::string& categoryString);
    ErrCode setName(const std::string& signalName);
    ErrCode setTimeout(const std::string& timeoutString);
    ErrCode setIsEnabled(const std::string& isEnabledString);
    ErrCode addTarget(int8_t isEnabled, const std::string& target);
    ErrCode addPermission(const std::string& permissionString);
    ErrCode addDerivative(const std::string& derivative);
    ErrCode addResource(Resource* resource);

    SignalInfo* build();
};

class ResourceBuilder {
private:
    Resource* mResource;

public:
    ResourceBuilder();

    ErrCode setResCode(const std::string& resCodeString);
    ErrCode setResInfo(const std::string& resInfoString);
    ErrCode setNumValues(int32_t valuesCount);
    ErrCode addValue(const std::string& value);

    Resource* build();
};

#endif
