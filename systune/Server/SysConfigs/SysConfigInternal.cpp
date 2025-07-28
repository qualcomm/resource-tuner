// Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
// SPDX-License-Identifier: BSD-3-Clause-Clear

#include "SysConfigInternal.h"

int8_t sysConfigGetProp(const char* prop, char* buffer, uint64_t buffer_size, const char* def_value) {
    std::string propertyName(prop);
    std::string result;

    int8_t propFound = false;
    if((propFound = SysConfigPropRegistry::getInstance()->queryProperty(propertyName, result)) == false) {
        result = def_value;
    }

    memset(buffer, 0, buffer_size);
    buffer[buffer_size - 1] = '\0';
    strncpy(buffer, result.data(), std::min(buffer_size - 1, strlen(result.data())));

    return propFound;
}

int8_t sysConfigSetProp(const char* prop, const char* value) {
    std::string propertyName(prop);
    std::string propertyValue(value);

    return SysConfigPropRegistry::getInstance()->modifyProperty(propertyName, propertyValue);
}

int8_t submitSysConfigRequest(char* resultBuf, SysConfig* clientReq) {
    int8_t status = false;
    switch(clientReq->getRequestType()) {
        case REQ_SYSCONFIG_GET_PROP:
            status = sysConfigGetProp(clientReq->getProp(), resultBuf,
                                      clientReq->getBufferSize(),
                                      clientReq->getDefaultValue());
            break;
        case REQ_SYSCONFIG_SET_PROP:
            status = sysConfigSetProp(clientReq->getProp(), clientReq->getValue());
            break;
        default:
            break;
    }
    return status;
}
