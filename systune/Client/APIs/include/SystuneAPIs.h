// Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
// SPDX-License-Identifier: BSD-3-Clause-Clear

/*!
 * \file  SystuneAPIs.h
 */

/*!
 * \ingroup  SYSTUNE_CLIENT_APIS
 * \defgroup SYSTUNE_CLIENT_APIS Client APIs
 * \details This file contains all the Client-Facing Systune APIs. Using these APIs
 *          Clients, can send Requests to the Systune Server.
 *
 * @{
 */

#ifndef SYSTUNE_CLIENT_H
#define SYSTUNE_CLIENT_H

#include <cstdint>
#include <vector>
#include <cstring>
#include <cstdlib>
#include <unistd.h>
#include <memory>
#include <mutex>

#include "Request.h"
#include "Signal.h"
#include "ErrCodes.h"
#include "SafeOps.h"
#include "SystuneSocketClient.h"
#include "Utils.h"

/**
* @brief Tune Resource Values for finite or finite duration.
* @details Use this API to issue Resource Provisioning / Tuning Requests.
* @param duration Duration (in milliseconds) to provision the Resources for. A value of -1 denotes infinite duration.
* @param properties A 32 bit signed Integer storing the Properties of the Request.
*                   - The last 8 bits [25 - 32] store the Request Priority (HIGH / LOW)
*                   - The Next 8 bits [17 - 24] represent a Boolean Flag, which indicates
*                     if the Request should be processed in the background (in case of Display Off or Doze Mode).
* @param numRes Number of Resources to be tuned as part of the Request
* @param backgroundProcessing Boolean Flag to
* @param res List of Resources to be provisioned as part of the Request
* @return int64_t :
*              A Positive Unique Handle to identify the issued Request. The handle is used for future retune / untune APIs.\n
*              RC_REQ_SUBMISSION_FAILURE: If the Request could not be sent to the server.
*/

int64_t tuneResources(int64_t duration, int32_t prop, int32_t numRes, std::vector<Resource*>* res);

/**
* @brief Modify the duration of a previously issued Tune Request.
* @details Use this API to increase the duration (in milliseconds) of an existing Request issued via.
* @param handle Request Handle, returned by tuneResources.
* @param duration The new duration for the previously issued Tune request. A value of -1 denotes infinite duration.
* @return ErrCode:
*              RC_SUCCESS: If the Request was successfully submitted to the server.\n
*              RC_REQ_SUBMISSION_FAILURE: Otherwise.
*/
ErrCode retuneResources(int64_t handle, int64_t duration);

/**
* @brief Release (or free) the Request with the given handle.
* @details Use this API to issue Signal De-Provisioning Requests.
* @param handle Request Handle, returned by the tuneResources API call.
* @return ErrCode:
*              RC_SUCCESS: If the Request was successfully submitted to the server.\n
*              RC_REQ_SUBMISSION_FAILURE: Otherwise
*/
ErrCode untuneResources(int64_t handle);

/**
* @brief Fetches the requests sent by a particular client to the server and related information. If client is root, then all the requests received by server are returned
* @return std::string:
*              A string containing the request information
*/
//  Rename getData()
// Make it genertic, using Enums for example
// Pass a buffer to store the result
std::string getrequests();

/**
* @brief Gets a property from the Config Store.
* @details Use this API to fetch a SysConfig property by it's name, all of the properties were Parsed during Systune initialization.
* @param prop Name of the Property to be fetched.
* @param buffer A buffer to hold the result, i.e. the property value corresponding to the specified name.
* @param buffer_size Size of the buffer
* @param def_value Value to return in case a property with the specified Name is not found in the Config Store
* @return ErrCode:
*              RC_SUCCESS: If the Request was successfully Submitted to the Server.\n
*              RC_REQ_SUBMISSION_FAILURE: Otherwise
* Note: The result of the Query itself is stored in the buffer (IN / OUT arg).
*/
ErrCode getprop(const char* prop, char* buffer, size_t buffer_size, const char* def_value);

/**
* @brief Modifies an already existing property in the Config Store. // Modify: Only in the RAM (in-memory)
* @details Use this API to to change the value of a property.
* @param prop Name of the Property to be modified.
* @param value A buffer holding the new the property value.
* @return ErrCode:
*              RC_SUCCESS: If the Request was successfully Submitted to the Server.\n
*              RC_REQ_SUBMISSION_FAILURE: Otherwise
*/
ErrCode setprop(const char* prop, const char* value);

/**
* @brief Acquire the signal with the given ID.
* @details Use this API to issue Signal Provisioning Requests, for a certain duration of time.
* @param signalID ID of the Signal to be acquired.
* @param duration Duration (in milliseconds) to provision the Resources for. A value of -1 denotes infinite duration.
* @param properties A 32 bit signed Integer storing the Properties of the Request.
*                   - The last 8 bits [25 - 32] store the Request Priority (HIGH / LOW)
*                   - The Next 8 bits [17 - 24] represent a Boolean Flag, which indicates
*                     if the Request should be processed in the background (in case of Display Off or Doze Mode).
*
* @param appName Name of the Application that is issuing the Request
* @param scenario Name of the Scenario that is issuing the Request
* @param numArgs Number of Additional Arguments to be passed as part of the Request
* @param list List of Additional Arguments to be passed as part of the Request
* @return int64_t :
*              A Positive Unique Handle to identify the issued Request. The handle is used for freeing the acquired signal later.\n
*              RC_REQ_SUBMISSION_FAILURE: If the Request could not be sent to the server.
*/
int64_t tuneSignal(uint32_t signalID, int64_t duration, int32_t properties,
                   const char* appName, const char* scenario, int32_t numArgs, std::vector<uint32_t>* list);
// Use uint32* array // C

/**
* @brief Release (or free) the signal with the given handle.
* @details Use this API to issue Signal De-Provisioning Requests
* @param handle Request Handle, returned by the tuneSignal API call.
* @return ErrCode:
*              RC_SUCCESS: If the Request was successfully sent to the server.\n
*              RC_REQ_SUBMISSION_FAILURE: Otherwise
*/
ErrCode untuneSignal(int64_t handle);

#endif

/*! @} */
