#include "SysConfig.h"

int8_t SysConfig::getRequestType() {
    return this->mReqType;
}

const char* SysConfig::getProp() {
    return this->mProp;
}

const char* SysConfig::getValue() {
    return this->mValue;
}

const char* SysConfig::getDefaultValue() {
    return this->mDefValue;
}

int32_t SysConfig::getClientPID() {
    return this->mClientPID;
}

int32_t SysConfig::getClientTID() {
    return this->mClientTID;
}

uint64_t SysConfig::getBufferSize() {
    return this->mBufferSize;
}

void SysConfig::setRequestType(int8_t reqType) {
    this->mReqType = reqType;
}

void SysConfig::setProp(const char* prop) {
    this->mProp = prop;
}

void SysConfig::setValue(const char* value) {
    this->mValue = value;
}

void SysConfig::setDefaultValue(const char* defValue) {
    this->mDefValue = defValue;
}

void SysConfig::setClientPID(int32_t clientPID) {
    this->mClientPID = clientPID;
}

void SysConfig::setClientTID(int32_t clientTID) {
    this->mClientTID = clientTID;
}

void SysConfig::setBufferSize(uint64_t bufferSize) {
    this->mBufferSize = bufferSize;
}
