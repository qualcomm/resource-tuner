// Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
// SPDX-License-Identifier: BSD-3-Clause-Clear

#include "SignalRegistry.h"

std::shared_ptr<SignalRegistry> SignalRegistry::signalRegistryInstance = nullptr;

SignalRegistry::SignalRegistry() {
    this->mTotalSignals = 0;
}

int8_t SignalRegistry::isSignalConfigMalformed(SignalInfo* sConf) {
    if(sConf == nullptr) return true;
    if(sConf->mSignalOpId < 0 || sConf->mSignalCategory < 0) return true;
    return false;
}

void SignalRegistry::registerSignal(SignalInfo* signalInfo, int8_t isBuSpecified) {
    if(this->isSignalConfigMalformed(signalInfo)) {
        delete signalInfo;
        return;
    }

    uint32_t signalBitmap = 0;
    signalBitmap |= ((uint32_t)signalInfo->mSignalOpId);
    signalBitmap |= ((uint32_t)signalInfo->mSignalCategory << 16);

    // Check for any conflict
    if(this->mSystemIndependentLayerMappings.find(signalBitmap) !=
        this->mSystemIndependentLayerMappings.end()) {
        // Signal with the specified Category and SigID already exists
        // Overwrite it.

        int32_t signalTableIndex = getSignalTableIndex(signalBitmap);
        this->mSignalsConfigs[signalTableIndex] = signalInfo;

        if(isBuSpecified) {
            this->mSystemIndependentLayerMappings.erase(signalBitmap);
            signalBitmap |= (1 << 31);

            this->mSystemIndependentLayerMappings[signalBitmap] = signalTableIndex;
        }
    } else {
        if(isBuSpecified) {
            signalBitmap |= (1 << 31);
        }

        this->mSystemIndependentLayerMappings[signalBitmap] = this->mTotalSignals;
        this->mSignalsConfigs.push_back(signalInfo);

        this->mTotalSignals++;
    }
}

std::vector<SignalInfo*> SignalRegistry::getSignalConfigs() {
    return this->mSignalsConfigs;
}

SignalInfo* SignalRegistry::getSignalConfigById(uint32_t signalID) {
    if(this->mSystemIndependentLayerMappings.find(signalID) == this->mSystemIndependentLayerMappings.end()) {
        LOGE("RTN_SIGNAL_PROCESSOR", "Resource ID not found in the registry");
        return nullptr;
    }

    int32_t mResourceTableIndex = this->mSystemIndependentLayerMappings[signalID];
    return this->mSignalsConfigs[mResourceTableIndex];
}

int32_t SignalRegistry::getSignalsConfigCount() {
    return this->mTotalSignals;
}

void SignalRegistry::displaySignals() {
    for(int32_t i = 0; i < this->mTotalSignals; i++) {
        auto& signal = this->mSignalsConfigs[i];

        LOGD("RTN_SIGNAL_REGISTRY", "Signal Name: " + signal->mSignalName);
        LOGD("RTN_SIGNAL_REGISTRY", "Signal OpID: " + std::to_string(signal->mSignalOpId));
        LOGD("RTN_SIGNAL_REGISTRY", "Signal SignalCategory: " + std::to_string(signal->mSignalCategory));

        LOGD("RTN_SIGNAL_REGISTRY", "====================================");
    }
}

int32_t SignalRegistry::getSignalTableIndex(uint32_t signalID) {
    if(this->mSystemIndependentLayerMappings.find(signalID) == this->mSystemIndependentLayerMappings.end()) {
        return -1;
    }

    return this->mSystemIndependentLayerMappings[signalID];
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
        this->mSignalInfo->mSignalResources = nullptr;
    }
}

SignalInfoBuilder* SignalInfoBuilder::setOpID(const std::string& signalOpIdString) {
    if(this->mSignalInfo == nullptr) {
        return this;
    }

    this->mSignalInfo->mSignalOpId = -1;
    try {
        this->mSignalInfo->mSignalOpId = (int16_t)stoi(signalOpIdString, nullptr, 0);
    } catch(const std::invalid_argument& ex) {
        LOGE("RTN_SIGNAL_REGISTRY",
             "Signal Parsing Failed with error: " + std::string(ex.what()));
    } catch(const std::out_of_range& ex) {
        LOGE("RTN_SIGNAL_REGISTRY",
             "Signal Parsing Failed with error: " + std::string(ex.what()));
    }
    return this;
}

SignalInfoBuilder* SignalInfoBuilder::setCategory(const std::string& categoryString) {
    if(this->mSignalInfo == nullptr) {
        return this;
    }

    this->mSignalInfo->mSignalCategory = -1;
    try {
        this->mSignalInfo->mSignalCategory = (int8_t)stoi(categoryString, nullptr, 0);
    } catch(const std::invalid_argument& ex) {
        LOGE("RTN_SIGNAL_REGISTRY",
             "Signal Parsing Failed with error: " + std::string(ex.what()));
    } catch(const std::out_of_range& ex) {
        LOGE("RTN_SIGNAL_REGISTRY",
             "Signal Parsing Failed with error: " + std::string(ex.what()));
    }
    return this;
}

SignalInfoBuilder* SignalInfoBuilder::setName(const std::string& signalName) {
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

SignalInfoBuilder* SignalInfoBuilder::addPermission(const std::string& permissionString) {
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

SignalInfoBuilder* SignalInfoBuilder::addTarget(int8_t isEnabled, const std::string& target) {
    if(this->mSignalInfo == nullptr) {
        return this;
    }

    std::string targetName(target);

    if(isEnabled) {
        if(this->mSignalInfo->mTargetsEnabled == nullptr) {
            this->mSignalInfo->mTargetsEnabled = new(std::nothrow) std::unordered_set<std::string>;
        }

        if(this->mSignalInfo->mTargetsEnabled != nullptr) {
            std::transform(targetName.begin(), targetName.end(), targetName.begin(),
                [](unsigned char ch) {return std::tolower(ch);});
            this->mSignalInfo->mTargetsEnabled->insert(targetName);
        }
    } else {
        if(this->mSignalInfo->mTargetsDisabled == nullptr) {
            this->mSignalInfo->mTargetsDisabled = new(std::nothrow) std::unordered_set<std::string>;
        }

        if(this->mSignalInfo->mTargetsDisabled != nullptr) {
            std::transform(targetName.begin(), targetName.end(), targetName.begin(),
                [](unsigned char ch) {return std::tolower(ch);});
            this->mSignalInfo->mTargetsDisabled->insert(targetName);
        }
    }

    return this;
}

SignalInfoBuilder* SignalInfoBuilder::addDerivative(const std::string& derivative) {
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

SignalInfoBuilder* SignalInfoBuilder::addResource(Resource* resource) {
    if(this->mSignalInfo == nullptr) {
        return this;
    }

    if(this->mSignalInfo->mSignalResources == nullptr) {
        this->mSignalInfo->mSignalResources = new(std::nothrow) std::vector<Resource*>();
    }

    if(this->mSignalInfo->mSignalResources != nullptr) {
        this->mSignalInfo->mSignalResources->push_back(resource);
    }

    return this;
}

SignalInfo* SignalInfoBuilder::build() {
    return this->mSignalInfo;
}

ResourceBuilder::ResourceBuilder() {
    this->mResource = new Resource;
}

ResourceBuilder* ResourceBuilder::setResCode(const std::string& resCodeString) {
    if(this->mResource == nullptr) return this;

    uint32_t resourceOpCode = 0;
    try {
        resourceOpCode = (uint32_t)stol(resCodeString, nullptr, 0);
        this->mResource->setOpCode(resourceOpCode);
    } catch(const std::invalid_argument& ex) {

    } catch(const std::out_of_range& ex) {

    }

    return this;
}

ResourceBuilder* ResourceBuilder::setOpInfo(const std::string& opInfoString) {
    if(this->mResource == nullptr) return this;

    int32_t resourceOpInfo = 0;
    try {
        resourceOpInfo = (int32_t)stoi(opInfoString, nullptr, 0);
        this->mResource->setOperationalInfo(resourceOpInfo);
    } catch(const std::invalid_argument& ex) {

    } catch(const std::out_of_range& ex) {

    }

    return this;
}

ResourceBuilder* ResourceBuilder::setNumValues(int32_t valuesCount) {
    if(this->mResource == nullptr) return this;

    this->mResource->setNumValues(valuesCount);

    return this;
}

ResourceBuilder* ResourceBuilder::addValue(int32_t value) {
    if(this->mResource == nullptr) return this;

    if(this->mResource->getValuesCount() == 1) {
        this->mResource->mConfigValue.singleValue = value;
    } else {
        if(this->mResource->mConfigValue.valueArray == nullptr) {
            this->mResource->mConfigValue.valueArray = new std::vector<int32_t>;
        }
        this->mResource->mConfigValue.valueArray->push_back(value);
    }

    return this;
}

Resource* ResourceBuilder::build() {
    return this->mResource;
}
