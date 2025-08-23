// Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
// SPDX-License-Identifier: BSD-3-Clause-Clear

#include "Resource.h"

int32_t Resource::getCoreValue() {
    return (int32_t)(this->mResInfo) & ((1 << 8) - 1);
}

int32_t Resource::getClusterValue() {
    return (int32_t)(this->mResInfo >> 8) & ((1 << 8) - 1);
}

int32_t Resource::getResInfo() {
    return this->mResInfo;
}

int32_t Resource::getOptionalInfo() {
    return this->mOptionalInfo;
}

uint32_t Resource::getResCode() {
    return this->mResCode;
}
int32_t Resource::getValuesCount() {
    return this->mNumValues;
}

void Resource::setCoreValue(int32_t core) {
    this->mResInfo = (this->mResInfo ^ this->getCoreValue()) | core;
}

void Resource::setClusterValue(int32_t cluster) {
    this->mResInfo = (this->mResInfo ^ (this->getClusterValue() << 8)) | (cluster << 8);
}

void Resource::setResourceID(int16_t resID) {
    this->mResCode |= (uint32_t)resID;
}

void Resource::setResourceType(int8_t resType) {
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
