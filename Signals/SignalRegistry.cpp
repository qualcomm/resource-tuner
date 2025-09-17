// Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
// SPDX-License-Identifier: BSD-3-Clause-Clear

#include "SignalRegistry.h"

std::shared_ptr<SignalRegistry> SignalRegistry::signalRegistryInstance = nullptr;

SignalRegistry::SignalRegistry() {
    this->mTotalSignals = 0;
}

int8_t SignalRegistry::isSignalConfigMalformed(SignalInfo* sConf) {
    if(sConf == nullptr) return true;
    if(sConf->mSignalCategory == 0) return true;
    return false;
}

void SignalRegistry::registerSignal(SignalInfo* signalInfo, int8_t isBuSpecified) {
    if(this->isSignalConfigMalformed(signalInfo)) {
        if(signalInfo != nullptr) {
            delete signalInfo;
        }
        return;
    }

    uint32_t signalBitmap = 0;
    signalBitmap |= ((uint32_t)signalInfo->mSignalID);
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

SignalInfo* SignalRegistry::getSignalConfigById(uint32_t signalCode) {
    if(this->mSystemIndependentLayerMappings.find(signalCode) == this->mSystemIndependentLayerMappings.end()) {
        TYPELOGV(SIGNAL_REGISTRY_SIGNAL_NOT_FOUND, signalCode);
        return nullptr;
    }

    int32_t mResourceTableIndex = this->mSystemIndependentLayerMappings[signalCode];
    return this->mSignalsConfigs[mResourceTableIndex];
}

int32_t SignalRegistry::getSignalsConfigCount() {
    return this->mTotalSignals;
}

void SignalRegistry::displaySignals() {
    for(int32_t i = 0; i < this->mTotalSignals; i++) {
        auto& signal = this->mSignalsConfigs[i];

        LOGD("RESTUNE_SIGNAL_REGISTRY", "Signal Name: " + signal->mSignalName);
        LOGD("RESTUNE_SIGNAL_REGISTRY", "Signal OpID: " + std::to_string(signal->mSignalID));
        LOGD("RESTUNE_SIGNAL_REGISTRY", "Signal SignalCategory: " + std::to_string(signal->mSignalCategory));

        LOGD("RESTUNE_SIGNAL_REGISTRY", "====================================");
    }
}

int32_t SignalRegistry::getSignalTableIndex(uint32_t signalCode) {
    if(this->mSystemIndependentLayerMappings.find(signalCode) == this->mSystemIndependentLayerMappings.end()) {
        return -1;
    }

    return this->mSystemIndependentLayerMappings[signalCode];
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
    if(this->mSignalInfo == nullptr) {
        return;
    }

    this->mSignalInfo->mSignalID = 0;
    this->mSignalInfo->mSignalCategory = 0;
    this->mSignalInfo->mSignalName = "";
    this->mSignalInfo->mIsEnabled = false;
    this->mSignalInfo->mTimeout = 1;

    this->mSignalInfo->mTargetsEnabled = nullptr;
    this->mSignalInfo->mTargetsDisabled = nullptr;
    this->mSignalInfo->mDerivatives = nullptr;
    this->mSignalInfo->mPermissions = nullptr;
    this->mSignalInfo->mSignalResources = nullptr;
}

ErrCode SignalInfoBuilder::setSignalID(const std::string& signalIdString) {
    if(this->mSignalInfo == nullptr) {
        return RC_MEMORY_ALLOCATION_FAILURE;
    }

    this->mSignalInfo->mSignalID = 0;
    try {
        this->mSignalInfo->mSignalID = (uint16_t)stoi(signalIdString, nullptr, 0);

    } catch(const std::invalid_argument& e) {
        TYPELOGV(SIGNAL_REGISTRY_PARSING_FAILURE, e.what());
        return RC_INVALID_VALUE;

    } catch(const std::out_of_range& e) {
        TYPELOGV(SIGNAL_REGISTRY_PARSING_FAILURE, e.what());
        return RC_INVALID_VALUE;
    }

    return RC_SUCCESS;
}

ErrCode SignalInfoBuilder::setSignalCategory(const std::string& categoryString) {
    if(this->mSignalInfo == nullptr) {
        return RC_MEMORY_ALLOCATION_FAILURE;
    }

    this->mSignalInfo->mSignalCategory = 0;
    try {
        this->mSignalInfo->mSignalCategory = (uint8_t)stoi(categoryString, nullptr, 0);
    } catch(const std::invalid_argument& e) {
        TYPELOGV(SIGNAL_REGISTRY_PARSING_FAILURE, e.what());
        return RC_INVALID_VALUE;

    } catch(const std::out_of_range& e) {
        TYPELOGV(SIGNAL_REGISTRY_PARSING_FAILURE, e.what());
        return RC_INVALID_VALUE;
    }

    return RC_SUCCESS;
}

ErrCode SignalInfoBuilder::setName(const std::string& signalName) {
    if(this->mSignalInfo == nullptr) {
        return RC_MEMORY_ALLOCATION_FAILURE;
    }

    this->mSignalInfo->mSignalName = signalName;
    return RC_SUCCESS;
}

ErrCode SignalInfoBuilder::setTimeout(const std::string& timeoutString) {
    if(this->mSignalInfo == nullptr) {
        return RC_MEMORY_ALLOCATION_FAILURE;
    }

    try {
        this->mSignalInfo->mTimeout = std::stoi(timeoutString);
        return RC_SUCCESS;

    } catch(const std::exception& e) {
        return RC_INVALID_VALUE;
    }

    return RC_INVALID_VALUE;
}

ErrCode SignalInfoBuilder::setIsEnabled(const std::string& isEnabledString) {
    if(this->mSignalInfo == nullptr) {
        return RC_MEMORY_ALLOCATION_FAILURE;
    }

    this->mSignalInfo->mIsEnabled = (isEnabledString == "true");
    return RC_SUCCESS;
}

ErrCode SignalInfoBuilder::addPermission(const std::string& permissionString) {
    if(this->mSignalInfo == nullptr) {
        return RC_MEMORY_ALLOCATION_FAILURE;
    }

    if(this->mSignalInfo->mPermissions == nullptr) {
        this->mSignalInfo->mPermissions = new(std::nothrow) std::vector<enum Permissions>;
    }

    enum Permissions permission = PERMISSION_THIRD_PARTY;
    if(permissionString == "system") {
        permission = PERMISSION_SYSTEM;
    } else if(permissionString == "third_party") {
        permission = PERMISSION_THIRD_PARTY;
    } else {
        if(permissionString.length() != 0) {
            return RC_INVALID_VALUE;
        }
    }

    if(this->mSignalInfo->mPermissions != nullptr) {
        try {
            this->mSignalInfo->mPermissions->push_back(permission);
        } catch(const std::bad_alloc& e) {
            return RC_INVALID_VALUE;
        }
    } else {
        return RC_INVALID_VALUE;
    }

    return RC_SUCCESS;
}

ErrCode SignalInfoBuilder::addTargetEnabled(const std::string& target) {
    if(this->mSignalInfo == nullptr) {
        return RC_MEMORY_ALLOCATION_FAILURE;
    }

    std::string targetName(target);
    if(this->mSignalInfo->mTargetsEnabled == nullptr) {
        this->mSignalInfo->mTargetsEnabled = new(std::nothrow) std::unordered_set<std::string>;
    }

    if(this->mSignalInfo->mTargetsEnabled != nullptr) {
        std::transform(targetName.begin(), targetName.end(), targetName.begin(),
            [](unsigned char ch) {return std::tolower(ch);});
        this->mSignalInfo->mTargetsEnabled->insert(targetName);
    } else {
        return RC_MEMORY_ALLOCATION_FAILURE;
    }

    return RC_SUCCESS;
}

ErrCode SignalInfoBuilder::addTargetDisabled(const std::string& target) {
    if(this->mSignalInfo == nullptr) {
        return RC_MEMORY_ALLOCATION_FAILURE;
    }

    std::string targetName(target);
    if(this->mSignalInfo->mTargetsDisabled == nullptr) {
        this->mSignalInfo->mTargetsDisabled = new(std::nothrow) std::unordered_set<std::string>;
    }

    if(this->mSignalInfo->mTargetsDisabled != nullptr) {
        std::transform(targetName.begin(), targetName.end(), targetName.begin(),
            [](unsigned char ch) {return std::tolower(ch);});
        this->mSignalInfo->mTargetsDisabled->insert(targetName);
    } else {
        return RC_MEMORY_ALLOCATION_FAILURE;
    }

    return RC_SUCCESS;
}

ErrCode SignalInfoBuilder::addDerivative(const std::string& derivative) {
    if(this->mSignalInfo == nullptr) {
        return RC_MEMORY_ALLOCATION_FAILURE;
    }

    if(this->mSignalInfo->mDerivatives == nullptr) {
        this->mSignalInfo->mDerivatives = new(std::nothrow) std::vector<std::string>();
    }

    if(this->mSignalInfo->mDerivatives != nullptr) {
        try {
            this->mSignalInfo->mDerivatives->push_back(derivative);
        } catch(const std::bad_alloc& e) {
            return RC_INVALID_VALUE;
        }
    } else {
        return RC_INVALID_VALUE;
    }

    return RC_SUCCESS;
}

ErrCode SignalInfoBuilder::addResource(Resource* resource) {
    if(this->mSignalInfo == nullptr) {
        return RC_MEMORY_ALLOCATION_FAILURE;
    }

    if(this->mSignalInfo->mSignalResources == nullptr) {
        this->mSignalInfo->mSignalResources = new(std::nothrow) std::vector<Resource*>();
    }

    if(this->mSignalInfo->mSignalResources != nullptr) {
        try {
            this->mSignalInfo->mSignalResources->push_back(resource);
        } catch(const std::bad_alloc& e) {
            return RC_INVALID_VALUE;
        }
    } else {
        return RC_INVALID_VALUE;
    }

    return RC_SUCCESS;
}

SignalInfo* SignalInfoBuilder::build() {
    return this->mSignalInfo;
}

SignalInfoBuilder::~SignalInfoBuilder() {}

ResourceBuilder::ResourceBuilder() {
    this->mResource = new(std::nothrow) Resource;
}

ErrCode ResourceBuilder::setResCode(const std::string& resCodeString) {
    if(this->mResource == nullptr) {
        return RC_MEMORY_ALLOCATION_FAILURE;
    }

    uint32_t resCode = 0;
    try {
        resCode = (uint32_t)stol(resCodeString, nullptr, 0);
        this->mResource->setResCode(resCode);

    } catch(const std::invalid_argument& e) {
        TYPELOGV(SIGNAL_REGISTRY_PARSING_FAILURE, e.what());
        return RC_INVALID_VALUE;

    } catch(const std::out_of_range& e) {
        TYPELOGV(SIGNAL_REGISTRY_PARSING_FAILURE, e.what());
        return RC_INVALID_VALUE;
    }

    return RC_SUCCESS;
}

ErrCode ResourceBuilder::setResInfo(const std::string& resInfoString) {
    if(this->mResource == nullptr) {
        return RC_MEMORY_ALLOCATION_FAILURE;
    }

    int32_t resourceResInfo = 0;
    try {
        resourceResInfo = (int32_t)stoi(resInfoString, nullptr, 0);
        this->mResource->setResInfo(resourceResInfo);

    } catch(const std::invalid_argument& e) {
        TYPELOGV(SIGNAL_REGISTRY_PARSING_FAILURE, e.what());
        return RC_INVALID_VALUE;

    } catch(const std::out_of_range& e) {
        TYPELOGV(SIGNAL_REGISTRY_PARSING_FAILURE, e.what());
        return RC_INVALID_VALUE;
    }

    return RC_SUCCESS;
}

ErrCode ResourceBuilder::setNumValues(int32_t valuesCount) {
    if(this->mResource == nullptr) {
        return RC_MEMORY_ALLOCATION_FAILURE;
    }

    this->mResource->setNumValues(valuesCount);
    return RC_SUCCESS;
}

ErrCode ResourceBuilder::addValue(const std::string& valueString) {
    if(this->mResource == nullptr) {
        return RC_MEMORY_ALLOCATION_FAILURE;
    }

    int32_t value = -1;
    try {
        value = std::stoi(valueString);

    } catch(const std::exception& e) {
        return RC_INVALID_VALUE;
    }

    if(this->mResource->getValuesCount() == 1) {
        this->mResource->mResValue.value = value;
    } else {
        if(this->mResource->mResValue.values == nullptr) {
            this->mResource->mResValue.values = new(std::nothrow) std::vector<int32_t>;
            if(this->mResource->mResValue.values == nullptr) {
                return RC_INVALID_VALUE;
            }
        }
        try {
            this->mResource->mResValue.values->push_back(value);
        } catch(const std::bad_alloc& e) {
            return RC_INVALID_VALUE;
        }
    }

    return RC_SUCCESS;
}

Resource* ResourceBuilder::build() {
    return this->mResource;
}
