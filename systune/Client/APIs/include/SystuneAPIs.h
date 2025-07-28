// Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
// SPDX-License-Identifier: BSD-3-Clause-Clear

/*!
 * \file  SystuneClient.h
 *
 * \brief This file contains the top-level API of the clients to use.
 */

/*!
 * \ingroup  SYSTUNE_CLIENT
 * \defgroup SYSTUNE_CLIENT Client Main API
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
* @param prio Priority of the Request
* @param numRes Number of Resources to be tuned as part of the Request
* @param backgroundProcessing Boolean Flag to indicate if the Request is to be processed in the background.
* @param res List of Resources to be provisioned as part of the Request
* @return int64_t :
*              A Positive Unique Handle to identify the issued Request. The handle is used for future retune / untune APIs.
*              -1 If the Request could not be sent to the server.
*/

// Change prio to Properties / props
// Use 16 bits for backgroundProcessing mode, last 16 bits for prio
int64_t tuneResources(int64_t duration, int32_t prop, int32_t numRes, std::vector<Resource*>* res);

/**
* @brief Modify the duration of a previously issued Tune Request.
* @details Use this API to increase the duration (in milliseconds) of an existing Request issued via.
* @param handle Request Handle, returned by tuneResources.
* @param duration The new duration for the previously issued Tune request. A value of -1 denotes infinite duration.
* @return int8_t:
*              0: If the Request was successfully submitted to the server
*              -1: Otherwise
*/
int8_t retuneResources(int64_t handle, int64_t duration);

/**
* @brief Withdraw a previously issued Resource Provisioning Request.
* @details Use this API to delete a previously issued Tune Request.
* @param handle Request Handle, returned by tuneResources.
* @return int8_t:
*              0: If the Request was successfully submitted to the server
*              -1: Otherwise
*/
int8_t untuneResources(int64_t handle);

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
* @details Use this API to fetch a property by it's name, all of the properties were parsed during Systune initialization.
* @param prop Name of the Property to be fetched.
* @param buffer A buffer to hold the result, i.e. the property value corresponding to the specified name.
* @param buffer_size Size of the buffer
* @param def_value Value to return in case a property with the specified Name is not found in the Config Store
* @return int8_t:
*              0: If the Property was found in the store, and successfully fetched
*              -1: Otherwise
*/
int8_t getprop(const char* prop, char* buffer, size_t buffer_size, const char* def_value);

/**
* @brief Modifies an already existing property in the Config Store. // Modify: Only in the RAM (in-memory)
* @details Use this API to to change the value of a property.
* @param prop Name of the Property to be modified.
* @param value A buffer holding the new the property value.
* @return int8_t:
*              0: If the Property with the specified name was found in the store, and was updated successfully.
*              -1: Otherwise
*/
int8_t setprop(const char* prop, const char* value);
// Use c++ in second level APIs

/**
* @brief Acquire the signal with the given ID.
* @details Use this API to issue Signal Provisioning Requests, for a certain duration of time.
* @param signalID ID of the Signal to be acquired.
* @param duration Duration (in milliseconds) to provision the Resources for. A value of -1 denotes infinite duration.
* @param prio Priority of the Request
* @param appName Name of the Application that is issuing the Request
* @param scenario Name of the Scenario that is issuing the Request
* @param numArgs Number of Additional Arguments to be passed as part of the Request
* @param list List of Additional Arguments to be passed as part of the Request
* @return int64_t :
*              A Positive Unique Handle to identify the issued Request. The handle is used for freeing the acquired signal later.
*              -1 If the Request could not be sent to the server.
*/
int64_t tuneSignal(uint32_t signalID, int64_t duration, int32_t prio, // modify similar to tuneResources
                   const char* appName, const char* scenario, int32_t numArgs, std::vector<uint32_t>* list);
// Use uint32* array // C

/**
* @brief Release (or free) the signal with the given ID.
* @details Use this API to issue Signal De-Provisioning Requests
* @param handle Identifier of the Signal Provisioning Request, returned as part of tuneSignal call.
* @param duration Duration (in milliseconds) to provision the Resources for. A value of -1 denotes infinite duration.
* @param prio Priority of the Request
* @param appName Name of the Application that is issuing the Request
* @param scenario Name of the Scenario that is issuing the Request
* @param numArgs Number of Additional Arguments to be passed as part of the Request
* @param list List of Additional Arguments to be passed as part of the Request
* @return int8_t :
*              0 If the Request was successfully sent to the server.
*              -1 Otherwise
*/
// Just handle is needed for untuning
int8_t untuneSignal(int64_t handle);

#endif

/*! @} */
