// Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
// SPDX-License-Identifier: BSD-3-Clause-Clear

/*!
 * \file ResourceTunerAPIs.h
 */

/*!
 * \ingroup  RESOURCE_TUNER_CLIENT_APIS
 * \defgroup RESOURCE_TUNER_CLIENT_APIS Client APIs
 * \details Resource Tuner's Client-Facing APIs.
 *
 * @{
 */

#ifndef RESOURCE_TUNER_CLIENT_H
#define RESOURCE_TUNER_CLIENT_H

#ifdef __cplusplus
#include <cstdint>
#include <cstring>
extern "C" {
#else
#include <stdio.h>
#include <stdint.h>
#endif

/**
 * @struct SysResource
 * @brief Used to store information regarding Resources / Tunables which need to be
 *        Provisioned as part of the tuneResources API.
 */
typedef struct {
    /**
     * @brief A uniqued 32-bit (unsigned) identifier for the Resource.
     *        - The last 16 bits (17-32) are used to specify the ResId
     *        - The next 8 bits (9-16) are used to specify the ResType (type of the Resource)
     *        - In addition for Custom Resources, then the MSB must be set to 1 as well
     */
    uint32_t mResCode;
    /**
     * @brief Holds Logical Core and Cluster Information:
     *        - The last 8 bits (25-32) hold the Logical Core Value
     *        - The next 8 bits (17-24) hold the Logical Cluster Value
     */
    int32_t mResInfo;
    int32_t mOptionalInfo; //!< Field to hold optional information for Request Processing
    /**
     * @brief Number of values to be configured for the Resource,
     *        both single-valued and multi-valued Resources are supported.
     */
    int32_t mNumValues;

    union {
        int32_t value; //!< Use this field for single Valued Resources
        int32_t* values; //!< Use this field for Multi Valued Resources
    } mResValue; //!< The value to be Configured for this Resource Node.
} SysResource;

// Define Utilities to parse and set the mResInfo field in Resource struct.
#define EXTRACT_RESOURCE_CORE_VALUE(resInfo)({                                               \
    (int32_t) (resInfo) & ((1 << 8) - 1);                                                    \
})                                                                                           \

#define EXTRACT_RESOURCE_CLUSTER_VALUE(resInfo)({                                            \
    (int32_t) (resInfo >> 8) & ((1 << 8) - 1);                                               \
})                                                                                           \

#define SET_RESOURCE_CORE_VALUE(resInfo, newValue)({                                         \
    (int32_t) (resInfo ^ EXTRACT_RESOURCE_CORE_VALUE(resInfo)) | newValue;                   \
})                                                                                           \

#define SET_RESOURCE_CLUSTER_VALUE(resInfo, newValue)({                                      \
    (int32_t) (resInfo ^ (EXTRACT_RESOURCE_CLUSTER_VALUE(resInfo) << 8)) | (newValue << 8);  \
})                                                                                           \

/**
 * @brief Tune Resource Values for finite or finite duration.
 * @details Use this API to issue Resource Provisioning / Tuning Requests.
 * @param duration Duration (in milliseconds) to provision the Resources for. A value of -1 denotes infinite duration.
 * @param properties A 32 bit signed Integer storing the Properties of the Request.
 *                   - The last 8 bits [25 - 32] store the Request Priority (HIGH / LOW)
 *                   - The Next 8 bits [17 - 24] represent a Boolean Flag, which indicates
 *                     if the Request should be processed in the background (in case of Display Off or Doze Mode).
 *
 * @param numRes Number of Resources to be tuned as part of the Request
 * @param resourceList List of Resources to be provisioned as part of the Request
 * @return int64_t:\n
 *            - A Positive Integer Handle which uniquely identifies the issued Request. The handle is used for future retune / untune APIs.\n
 *            - -1: If the Request could not be sent to the server.
 */
int64_t tuneResources(int64_t duration, int32_t prop, int32_t numRes, SysResource* resourceList);

/**
 * @brief Modify the duration of a previously issued Tune Request.
 * @details Use this API to increase the duration (in milliseconds) of an existing Request issued via.
 * @param handle Request Handle, returned by tuneResources.
 * @param duration The new duration for the previously issued Tune request. A value of -1 denotes infinite duration.
 * @return int8_t:\n
 *            - 0: If the Request was successfully submitted to the server.\n
 *            - -1: Otherwise.
 */
int8_t retuneResources(int64_t handle, int64_t duration);

/**
 * @brief Release (or free) the Request with the given handle.
 * @details Use this API to issue Signal De-Provisioning Requests.
 * @param handle Request Handle, returned by the tuneResources API call.
 * @return int8_t:\n
 *            - 0: If the Request was successfully submitted to the server.\n
 *            - -1: Otherwise
 */
int8_t untuneResources(int64_t handle);

/**
 * @brief Gets a property from the Config Store.
 * @details Use this API to fetch a Property by it's name, all the properties are Parsed during Resource Tuner Server initialization.
 * @param prop Name of the Property to be fetched.
 * @param buffer A buffer to hold the result, i.e. the property value corresponding to the specified name.
 * @param bufferSize Size of the buffer
 * @param defValue Value to return in case a property with the specified Name is not found in the Config Store
 * @return int8_t:\n
 *            - 0: If the Request was successfully Submitted to the Server.\n
 *            - -1: Otherwise\n\n
 * @note The result of the Query itself is stored in the buffer (IN / OUT arg).
 */
int8_t getProp(const char* prop, char* buffer, size_t bufferSize, const char* defValue);

/**
 * @brief Tune the signal with the given ID.
 * @details Use this API to issue Signal Provisioning Requests, for a certain duration of time.
 * @param signalCode A uniqued 32-bit (unsigned) identifier for the Signal
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
 * @return int64_t:\n
 *            - A Positive Unique Handle to identify the issued Request. The handle is used for freeing the Provisioned signal later.\n
 *            - -1: If the Request could not be sent to the server.
 */
int64_t tuneSignal(uint32_t signalCode, int64_t duration, int32_t properties,
                   const char* appName, const char* scenario, int32_t numArgs, uint32_t* list);

/**
 * @brief Relay the signal to all the features subscribed to the signal with the given ID.
 * @details Use this API to issue Signal Relay Requests.
 * @param signalCode A uniqued 32-bit (unsigned) identifier for the Signal
 * @param duration Duration (in milliseconds)
 * @param properties A 32 bit signed Integer storing the Properties of the Request.
 *                   - The last 8 bits [25 - 32] store the Request Priority (HIGH / LOW)
 *                   - The Next 8 bits [17 - 24] represent a Boolean Flag, which indicates
 *                     if the Request should be processed in the background (in case of Display Off or Doze Mode).
 *
 * @param appName Name of the Application that is issuing the Request
 * @param scenario Name of the Scenario that is issuing the Request
 * @param numArgs Number of Additional Arguments to be passed as part of the Request
 * @param list List of Additional Arguments to be passed as part of the Request
 * @return int8_t:\n
 *            - 0: If the Request was successfully sent to the server.\n
 *            - -1: Otherwise
 */
int8_t relaySignal(uint32_t signalCode, int64_t duration, int32_t properties,
                   const char* appName, const char* scenario, int32_t numArgs, uint32_t* list);

/**
 * @brief Release (or free) the signal with the given handle.
 * @details Use this API to issue Signal De-Provisioning Requests
 * @param handle Request Handle, returned by the tuneSignal API call.
 * @return int8_t:\n
 *            - 0: If the Request was successfully sent to the server.\n
 *            - -1: Otherwise
 */
int8_t untuneSignal(int64_t handle);

#ifdef __cplusplus
}
#endif

#endif

/*! @} */
