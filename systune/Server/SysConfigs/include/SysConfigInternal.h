// Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
// SPDX-License-Identifier: BSD-3-Clause-Clear

#ifndef SYSCONFIG_INTERNAL_H
#define SYSCONFIG_INTERNAL_H

#include <cstring>

#include "SysConfigPropRegistry.h"
#include "SysConfig.h"
#include "Types.h"
#include "Utils.h"

/**
* @brief Gets a property from the Config Store.
* @details Note: This API is an meant to be used internally, i.e. by other URM componenets like SysSignals
*          and not the End-Client Directly. Client Facing APIs are provided in Client/APIs/
* @param prop Name of the Property to be fetched.
* @param buffer A buffer to hold the result, i.e. the property value corresponding to the specified name.
* @param buffer_size Size of the buffer
* @param def_value Value to return in case a property with the specified Name is not found in the Config Store
* @return int8_t:
*              1: If the Property was found in the store
*              0: Otherwise
*/
int8_t sysConfigGetProp(const std::string& prop, std::string& buffer, size_t buffer_size, const std::string& def_value);

/**
* @brief Modifies an already existing property in the Config Store.
* @details Note: This API is an meant to be used internally i.e. by other URM componenets like Provisioner or Signals
*          and not the End-Client Directly. Client Facing APIs are provided in Client/APIs/
* @param prop Name of the Property to be modified.
* @param value A buffer holding the new the property value.
* @return int8_t:
*              1: If the Property with the specified name was found in the store, and was updated successfully.
*              0: Otherwise
*/
int8_t sysConfigSetProp(const std::string& prop, const std::string& value);

/**
* @brief Submit an incoming SysConfig Request from a Client, for processing.
* @details This API accepts both SysConfig Get and Set Requests from the Client, and processes them accordingly.
* @param resultBuf A buffer to hold the result, i.e. the property value corresponding to the specified name.
* @param clientReq A buffer holding the SysConfig Request from the Client.
* @return int8_t:
*              1: If the Property with the specified name was found in the store, and was updated successfully.
*              0: Otherwise
*/
int8_t submitSysConfigRequest(std::string& resultBuf, SysConfig* clientReq);

#endif
