#include "Resource.h"

int32_t Resource::getCoreValue() {
    return (int32_t)(this->mOpInfo) & ((1 << 8) - 1);
}

int32_t Resource::getClusterValue() {
    return (int32_t)(this->mOpInfo >> 8) & ((1 << 8) - 1);
}

int32_t Resource::getOperationalInfo() {
    return this->mOpInfo;
}

int32_t Resource::getOptionalInfo() {
    return this->mOptionalInfo;
}

uint32_t Resource::getOpCode() {
    return this->mOpCode;
}
int32_t Resource::getValuesCount() {
    return this->mNumValues;
}

void Resource::setCoreValue(int32_t core) {
    this->mOpInfo = (this->mOpInfo ^ this->getCoreValue()) | core;
}

void Resource::setClusterValue(int32_t cluster) {
    this->mOpInfo = (this->mOpInfo ^ (this->getClusterValue() << 8)) | (cluster << 8);
}

void Resource::setResourceID(int16_t resID) {
    this->mOpCode |= (uint32_t)resID;
}

void Resource::setResourceType(int8_t resType) {
    this->mOpCode |= ((uint32_t)resType << 16);
}

void Resource::setOpCode(uint32_t opCode) {
    this->mOpCode = opCode;
}

void Resource::setOperationalInfo(int32_t opInfo) {
    this->mOpInfo = opInfo;
}

void Resource::setOptionalInfo(int32_t optionalInfo) {
    this->mOptionalInfo = optionalInfo;
}

void Resource::setNumValues(int32_t numValues) {
    this->mNumValues = numValues;
}

void Resource::setAsCustom() {
    this->mOpCode |= (1 << 31);
}
