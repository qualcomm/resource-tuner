// Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
// SPDX-License-Identifier: BSD-3-Clause-Clear

#include "SignalExtFeatureMapper.h"

std::shared_ptr<SignalExtFeatureMapper> SignalExtFeatureMapper::signalExtFeatureMapperInstance = nullptr;

int8_t SignalExtFeatureMapper::addFeature(uint32_t signalID, int32_t feature) {
    if(SignalRegistry::getInstance()->getSignalConfigById(signalID) == nullptr) {
        return false;
    }
    this->mSignalTofeaturesMap[signalID].push_back(feature);
    return true;
}

int8_t SignalExtFeatureMapper::getFeatures(uint32_t signalID, std::vector<uint32_t>& features) {
    if(this->mSignalTofeaturesMap.find(signalID) == this->mSignalTofeaturesMap.end()) {
        return false;
    }

    features = mSignalTofeaturesMap[signalID];
    return true;
}
