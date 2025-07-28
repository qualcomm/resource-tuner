// Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
// SPDX-License-Identifier: BSD-3-Clause-Clear

#include "SignalRegistry.h"

std::shared_ptr<SignalRegistry> SignalRegistry::signalRegistryInstance = nullptr;
int32_t SignalRegistry::mTotalSignals = 0;

SignalRegistry::SignalRegistry() {
    this->customerBit = 0;
}

void SignalRegistry::initRegistry(int32_t size, int8_t customerBit) {
    if(mSignalsConfigs.size() == 0 && size > 0) {
        mSignalsConfigs.resize(size);
        this->customerBit = customerBit;
    }
}

void SignalRegistry::registerSignal(SignalInfo* signalInfo) {
    if(signalInfo == nullptr) return;

    uint32_t signalBitmap = 0;
    if(this->customerBit) {
        signalBitmap |= (1 << 31);
    }

    signalBitmap |= ((uint32_t)signalInfo->mSignalOpId);
    signalBitmap |= ((uint32_t)signalInfo->mSignalCategory << 16);

    this->mSystemIndependentLayerMappings[signalBitmap] = mTotalSignals;
    this->mSignalsConfigs[mTotalSignals] = signalInfo;

    mTotalSignals++;
}

std::vector<SignalInfo*> SignalRegistry::getSignalConfigs() {
    return this->mSignalsConfigs;
}

SignalInfo* SignalRegistry::getSignalConfigById(uint32_t signalID) {
    if(this->mSystemIndependentLayerMappings.find(signalID) == this->mSystemIndependentLayerMappings.end()) {
        LOGE("URM_RESOURCE_PROCESSOR", "Resource ID not found in the registry");
        return nullptr;
    }

    int32_t mResourceConfigTableIndex = this->mSystemIndependentLayerMappings[signalID];
    return this->mSignalsConfigs[mResourceConfigTableIndex];
}

int32_t SignalRegistry::getSignalsConfigCount() {
    return this->mTotalSignals;
}

void SignalRegistry::displaySignals() {
    for(int32_t i = 0; i < mTotalSignals; i++) {
        auto& signal = this->mSignalsConfigs[i];

        LOGD("URM_SIGNAL_REGISTRY", "Signal Name: " + signal->mSignalName);
        LOGD("URM_SIGNAL_REGISTRY", "Signal OpID: " + std::to_string(signal->mSignalOpId));
        LOGD("URM_SIGNAL_REGISTRY", "Signal SignalCategory: " + std::to_string(signal->mSignalCategory));

        LOGD("URM_SIGNAL_REGISTRY", "====================================");
    }
}

SignalRegistry::~SignalRegistry() {
    for(int32_t i = 0; i < this->mTotalSignals; i++) {
        if(this->mSignalsConfigs[i] != nullptr) {
            if(this->mSignalsConfigs[i]->mTargetsEnabled != nullptr) {
                delete this->mSignalsConfigs[i]->mTargetsEnabled;
                this->mSignalsConfigs[i]->mTargetsEnabled = nullptr;
            }

            if(this->mSignalsConfigs[i]->mTargetsDisabled != nullptr) {
                delete this->mSignalsConfigs[i]->mTargetsDisabled;
                this->mSignalsConfigs[i]->mTargetsDisabled = nullptr;
            }

            if(this->mSignalsConfigs[i]->mPermissions != nullptr) {
                delete this->mSignalsConfigs[i]->mPermissions;
                this->mSignalsConfigs[i]->mPermissions = nullptr;
            }

            if(this->mSignalsConfigs[i]->mDerivatives != nullptr) {
                delete this->mSignalsConfigs[i]->mDerivatives;
                this->mSignalsConfigs[i]->mDerivatives = nullptr;
            }

            delete(this->mSignalsConfigs[i]);
            this->mSignalsConfigs[i] = nullptr;
        }
    }
}

SignalInfoBuilder::SignalInfoBuilder() {
    this->mSignalInfo = new(std::nothrow) SignalInfo;

    if(this->mSignalInfo != nullptr) {
        this->mSignalInfo->mTargetsEnabled = nullptr;
        this->mSignalInfo->mTargetsDisabled = nullptr;
        this->mSignalInfo->mDerivatives = nullptr;
        this->mSignalInfo->mPermissions = nullptr;
        this->mSignalInfo->mLocks = nullptr;
    }
}

SignalInfoBuilder* SignalInfoBuilder::setOpID(std::string signalOpIdString) {
    if(this->mSignalInfo == nullptr) {
        return this;
    }

    this->mSignalInfo->mSignalOpId = -1;
    try {
        this->mSignalInfo->mSignalOpId = (int16_t)stoi(signalOpIdString, nullptr, 0);
    } catch(std::invalid_argument const& ex) {
        LOGE("URM_SIGNAL_REGISTRY",
             "Signal Parsing Failed with error: " + std::string(ex.what()));
    } catch(std::out_of_range const& ex) {
        LOGE("URM_SIGNAL_REGISTRY",
             "Signal Parsing Failed with error: " + std::string(ex.what()));
    }
    return this;
}

SignalInfoBuilder* SignalInfoBuilder::setCategory(std::string categoryString) {
    if(this->mSignalInfo == nullptr) {
        return this;
    }

    this->mSignalInfo->mSignalCategory = -1;
    try {
        this->mSignalInfo->mSignalCategory = (int8_t)stoi(categoryString, nullptr, 0);
    } catch(std::invalid_argument const& ex) {
        LOGE("URM_SIGNAL_REGISTRY",
             "Signal Parsing Failed with error: " + std::string(ex.what()));
    } catch(std::out_of_range const& ex) {
        LOGE("URM_SIGNAL_REGISTRY",
             "Signal Parsing Failed with error: " + std::string(ex.what()));
    }
    return this;
}

SignalInfoBuilder* SignalInfoBuilder::setName(std::string signalName) {
    if(this->mSignalInfo == nullptr) {
        return this;
    }

    this->mSignalInfo->mSignalName = signalName;
    return this;
}

SignalInfoBuilder* SignalInfoBuilder::setTimeout(int32_t timeout) {
    if(this->mSignalInfo == nullptr) {
        return this;
    }

    this->mSignalInfo->mTimeout = timeout;
    return this;
}

SignalInfoBuilder* SignalInfoBuilder::setIsEnabled(int8_t isEnabled) {
    if(this->mSignalInfo == nullptr) {
        return this;
    }

    this->mSignalInfo->mIsEnabled = isEnabled;
    return this;
}

SignalInfoBuilder* SignalInfoBuilder::addPermission(std::string permissionString) {
    if(this->mSignalInfo == nullptr) {
        return this;
    }

    if(this->mSignalInfo->mPermissions == nullptr) {
        this->mSignalInfo->mPermissions = new(std::nothrow) std::vector<enum Permissions>;
    }

    enum Permissions permission = PERMISSION_THIRD_PARTY;
    if(permissionString == "system") {
        permission = PERMISSION_SYSTEM;
    } else if(permissionString == "third_party") {
        permission = PERMISSION_THIRD_PARTY;
    }

    if(this->mSignalInfo->mPermissions != nullptr) {
        this->mSignalInfo->mPermissions->push_back(permission);
    }

    return this;
}

SignalInfoBuilder* SignalInfoBuilder::addTarget(int8_t isEnabled, std::string target) {
    if(this->mSignalInfo == nullptr) {
        return this;
    }

    if(isEnabled) {
        if(this->mSignalInfo->mTargetsEnabled == nullptr) {
            this->mSignalInfo->mTargetsEnabled = new(std::nothrow) std::unordered_set<std::string>;
        }

        if(this->mSignalInfo->mTargetsEnabled != nullptr) {
            std::transform(target.begin(), target.end(), target.begin(),
                [](unsigned char ch) {return std::tolower(ch);});
            this->mSignalInfo->mTargetsEnabled->insert(target);
        }
    } else {
        if(this->mSignalInfo->mTargetsDisabled == nullptr) {
            this->mSignalInfo->mTargetsDisabled = new(std::nothrow) std::unordered_set<std::string>;
        }

        if(this->mSignalInfo->mTargetsDisabled != nullptr) {
            std::transform(target.begin(), target.end(), target.begin(),
                [](unsigned char ch) {return std::tolower(ch);});
            this->mSignalInfo->mTargetsDisabled->insert(target);
        }
    }

    return this;
}

SignalInfoBuilder* SignalInfoBuilder::addDerivative(std::string derivative) {
    if(this->mSignalInfo == nullptr) {
        return this;
    }

    if(this->mSignalInfo->mDerivatives == nullptr) {
        this->mSignalInfo->mDerivatives = new(std::nothrow) std::vector<std::string>();
    }

    if(this->mSignalInfo->mDerivatives != nullptr) {
        this->mSignalInfo->mDerivatives->push_back(derivative);
    }
    return this;
}

SignalInfoBuilder* SignalInfoBuilder::addLock(uint32_t lockId) {
    if(this->mSignalInfo == nullptr) {
        return this;
    }

    if(this->mSignalInfo->mLocks == nullptr) {
        this->mSignalInfo->mLocks = new(std::nothrow) std::vector<uint32_t>();
    }

    if(this->mSignalInfo->mLocks != nullptr) {
        this->mSignalInfo->mLocks->push_back(lockId);
    }

    return this;
}

SignalInfo* SignalInfoBuilder::build() {
    return this->mSignalInfo;
}
