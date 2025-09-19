// Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
// SPDX-License-Identifier: BSD-3-Clause-Clear

#include "Resource.h"

Resource::Resource(const Resource& resource) {
    this->mResValue.values = nullptr;

    this->mResCode = resource.getResCode();
    this->mNumValues = resource.getValuesCount();
    this->mResInfo = resource.getResInfo();
    this->mOptionalInfo = resource.getOptionalInfo();

    if(this->mNumValues == 1) {
        this->mResValue.value = resource.mResValue.value;
    } else if(this->mNumValues > 1) {
        for(int32_t i = 0; i < this->mNumValues; i++) {
            if(this->mResValue.values == nullptr) {
                this->mResValue.values = new (GetBlock<std::vector<int32_t>>())
                                              std::vector<int32_t>;
            }
            this->mResValue.values->push_back((*resource.mResValue.values)[i]);
        }
    }
}

int32_t Resource::getCoreValue() const {
    return (int32_t)(this->mResInfo) & ((1 << 8) - 1);
}

int32_t Resource::getClusterValue() const {
    return (int32_t)(this->mResInfo >> 8) & ((1 << 8) - 1);
}

int32_t Resource::getResInfo()const  {
    return this->mResInfo;
}

int32_t Resource::getOptionalInfo() const {
    return this->mOptionalInfo;
}

uint32_t Resource::getResCode() const {
    return this->mResCode;
}

int32_t Resource::getValuesCount() const {
    return this->mNumValues;
}

void Resource::setCoreValue(int32_t core) {
    this->mResInfo = (this->mResInfo ^ this->getCoreValue()) | core;
}

void Resource::setClusterValue(int32_t cluster) {
    this->mResInfo = (this->mResInfo ^ (this->getClusterValue() << 8)) | (cluster << 8);
}

void Resource::setResourceID(uint16_t resID) {
    this->mResCode |= (uint32_t)resID;
}

void Resource::setResourceType(uint8_t resType) {
    this->mResCode |= ((uint32_t)resType << 16);
}

void Resource::setResCode(uint32_t resCode) {
    this->mResCode = resCode;
}

void Resource::setResInfo(int32_t resInfo) {
    this->mResInfo = resInfo;
}

void Resource::setOptionalInfo(int32_t optionalInfo) {
    this->mOptionalInfo = optionalInfo;
}

void Resource::setNumValues(int32_t numValues) {
    this->mNumValues = numValues;
}

void Resource::setAsCustom() {
    this->mResCode |= (1 << 31);
}

Resource::~Resource() {
    if(this->mNumValues > 1 && this->mResValue.values != nullptr) {
        FreeBlock<std::vector<int32_t>>(this->mResValue.values);
        this->mResValue.values = nullptr;
    }
}
