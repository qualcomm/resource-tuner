// Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
// SPDX-License-Identifier: BSD-3-Clause-Clear

#ifndef FRAMEWORK_INTERNAL_H
#define FRAMEWORK_INTERNAL_H

#include <cstdint>

#include "ErrCodes.h"
#include "Request.h"
#include "RequestQueue.h"
#include "ThreadPool.h"
#include "CocoTable.h"
#include "RateLimiter.h"
#include "RequestManager.h"
#include "ResourceRegistry.h"
#include "PropertiesRegistry.h"
#include "ServerInternal.h"
#include "TargetRegistry.h"

/**
* @brief Initializes the SysConfig module.
* @details This function will read and Parse the Property Config YAML file.
* @return int8_t:
*              RC_SUCCESS: If the module was correct initialized.
*              Non-Zero Status Code indicating an Error: If initialization failed
*/
ErrCode fetchProperties();

void toggleDisplayModes();

/**
* @brief Submit a Resource Provisioning Request from a Client for processing.
* @details Note: This API acts an interface for other Resource Tuner components like Signals
*          to submit a Resource Provisioning Request to the Resource Tuner Server, and
*          subsequently provision the desired Resources.
* @param request A buffer holding the Request.
*/
ErrCode submitResProvisionRequest(void* request);

ErrCode submitResProvisionRequest(Request* request, int8_t isVerified);

/**
* @brief Gets a property from the Config Store.
* @details Note: This API is an meant to be used internally, i.e. by other Resource Tuner componenets like Signals
*          and not the End-Client Directly. Client Facing APIs are provided in Client/APIs/
* @param prop Name of the Property to be fetched.
* @param buffer A buffer to hold the result, i.e. the property value corresponding to the specified name.
* @param buffer_size Size of the buffer
* @param def_value Value to return in case a property with the specified Name is not found in the Config Store
* @return int8_t:
*              1: If the Property was found in the store
*              0: Otherwise
*/
int8_t submitPropGetRequest(const std::string& prop, std::string& buffer, const std::string& def_value);

/**
* @brief Modifies an already existing property in the Config Store.
* @details Note: This API is an meant to be used internally i.e. by other Resource Tuner componenets like Signals
*          and not the End-Client Directly. Client Facing APIs are provided in Client/APIs/
* @param prop Name of the Property to be modified.
* @param value A buffer holding the new the property value.
* @return int8_t:
*              1: If the Property with the specified name was found in the store, and was updated successfully.
*              0: Otherwise
*/
int8_t submitPropSetRequest(const std::string& prop, const std::string& value);

/**
* @brief Submit an incoming SysConfig Request from a Client, for processing.
* @details This API accepts both SysConfig Get and Set Requests from the Client, and processes them accordingly.
* @param resultBuf A buffer to hold the result, i.e. the property value corresponding to the specified name.
* @param clientReq A buffer holding the SysConfig Request from the Client.
* @return int8_t:
*              1: If the Property with the specified name was found in the store, and was updated successfully.
*              0: Otherwise
*/
ErrCode submitPropRequest(void* request);

#endif
