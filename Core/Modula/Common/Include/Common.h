// Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
// SPDX-License-Identifier: BSD-3-Clause-Clear

#ifndef COMMON_UTILS_H
#define COMMON_UTILS_H

#ifdef __cplusplus
#include <cstdint>
#else
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

/**
 * @enum RequestPriority
 * @brief Requests can have 2 levels of Priorities, HIGH or LOW.
 */
enum RequestPriority {
    REQ_PRIORITY_HIGH = 0,
    REQ_PRIORITY_LOW,
    NUMBER_OF_RQUEST_PRIORITIES
};

/**
 * @enum Modes
 * @brief Represents the operational modes based on the device's display state.
 * @details Certain system resources are optimized only when the device display is active,
 *          primarily to conserve power. However, for critical components, tuning may be
 *          performed regardless of the display state, including during doze mode.
 */
enum Modes {
    MODE_DISPLAY_ON = 0x01, //!< Tuning allowed when the display is on.
    MODE_DISPLAY_OFF = 0x02, //!< Tuning allowed when the display is off.
    MODE_DOZE = 0x04 //!< Tuning allowed during doze (low-power idle) mode.
};

#define SET_REQUEST_PRIORITY(properties, priority)({                                          \
    int32_t retVal;                                                                           \
    if(properties < 0 || priority < 0 || priority >= NUMBER_OF_RQUEST_PRIORITIES) {           \
        retVal = -1;                                                                          \
    } else {                                                                                  \
        retVal = (int32_t) ((properties | priority));                                         \
    }                                                                                         \
    retVal;                                                                                   \
})                                                                                            \

#define ADD_ALLOWED_MODE(properties, mode)({                                                  \
    int32_t retVal;                                                                           \
    if(properties < 0 || mode < MODE_DISPLAY_ON || mode > MODE_DOZE) {                        \
        retVal = -1;                                                                          \
    } else {                                                                                  \
        retVal = (int32_t) (properties | (((properties >> 8) | mode) << 8));                  \
    }                                                                                         \
    retVal;                                                                                   \
})                                                                                            \

#define EXTRACT_REQUEST_PRIORITY(properties)({                                                \
    (int8_t) ((properties) & ((1 << 8) - 1));                                                 \
})

#define EXTRACT_ALLOWED_MODES(properties)({                                                   \
    (int8_t) ((properties >> 8) & ((1 << 8) - 1));                                            \
})                                                                                            \

// Define Utilities to parse and set the mResInfo field in Resource struct.
#define EXTRACT_RESOURCE_CORE_VALUE(resInfo)({                                                \
    (int8_t) ((resInfo) & ((1 << 8) - 1));                                                    \
})                                                                                            \

#define EXTRACT_RESOURCE_CLUSTER_VALUE(resInfo)({                                             \
    (int8_t) ((resInfo >> 8) & ((1 << 8) - 1));                                               \
})                                                                                            \

#define EXTRACT_RESOURCE_MPAM_VALUE(resInfo)({                                                \
    (int8_t) ((resInfo >> 16) & ((1 << 8) - 1));                                              \
})                                                                                            \

#define SET_RESOURCE_CORE_VALUE(resInfo, newValue)({                                          \
    (int32_t) ((resInfo ^ EXTRACT_RESOURCE_CORE_VALUE(resInfo)) | newValue);                  \
})                                                                                            \

#define SET_RESOURCE_CLUSTER_VALUE(resInfo, newValue)({                                       \
    (int32_t) ((resInfo ^ (EXTRACT_RESOURCE_CLUSTER_VALUE(resInfo) << 8)) | (newValue << 8)); \
})                                                                                            \

#define SET_RESOURCE_MPAM_VALUE(resInfo, newValue)({                                          \
    (int32_t) ((resInfo ^ (EXTRACT_RESOURCE_MPAM_VALUE(resInfo) << 16)) | (newValue << 16));  \
})                                                                                            \

#endif
