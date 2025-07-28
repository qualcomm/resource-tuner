// Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
// SPDX-License-Identifier: BSD-3-Clause-Clear

#include "SysConfigPropRegistry.h"

std::shared_ptr<SysConfigPropRegistry> SysConfigPropRegistry::sysConfigPropRegistryInstance = nullptr;
SysConfigPropRegistry::SysConfigPropRegistry() {}

int8_t SysConfigPropRegistry::createProperty(std::string& propertyName, std::string& propertyValue) {
    if(propertyName.length() == 0 || propertyValue.length() == 0) {
        return false;
    }
    if(this->mProperties.find(propertyName) != this->mProperties.end()) {
        return false;
    }
    this->mProperties[propertyName] = propertyValue;
    return true;
}

int8_t SysConfigPropRegistry::queryProperty(std::string& propertyName, std::string& result) {
    if(propertyName.length() == 0) {
        return false;
    }

    this->mPropRegistryMutex.lock_shared();
    if(this->mProperties.find(propertyName) == this->mProperties.end()) {
        this->mPropRegistryMutex.unlock_shared();
        return false;
    }

    result = this->mProperties[propertyName];
    this->mPropRegistryMutex.unlock_shared();

    return true;
}

int8_t SysConfigPropRegistry::modifyProperty(std::string& propertyName, std::string& propertyValue) {
    if(propertyName.length() == 0 || propertyValue.length() == 0) {
        return false;
    }

    std::string tmpResult;
    if(queryProperty(propertyName, tmpResult) == false) {
        return false;
    }

    this->mPropRegistryMutex.lock();
    this->mProperties[propertyName] = propertyValue;
    this->mPropRegistryMutex.unlock();

    return true;
}

int8_t SysConfigPropRegistry::deleteProperty(std::string& propertyName) {
    if(propertyName.length() == 0) {
        return false;
    }

    std::string tmpResult;
    if(queryProperty(propertyName, tmpResult) == false) {
        return false;
    }

    this->mPropRegistryMutex.lock();
    this->mProperties.erase(propertyName);
    this->mPropRegistryMutex.unlock();

    return true;
}

int32_t SysConfigPropRegistry::getPropertiesCount() {
    return this->mProperties.size();
}

SysConfigPropRegistry::~SysConfigPropRegistry() {}
