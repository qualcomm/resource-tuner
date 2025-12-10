// Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
// SPDX-License-Identifier: BSD-3-Clause-Clear

#ifndef COMMON_UTILS_H
#define COMMON_UTILS_H

#ifdef __cplusplus
#include <cstdint>
#else
#include <stdint.h>
#endif

// Use these options to configure a well defined value
#define OPT_WITHMAX 0x00000001
#define OPT_WITHMIN 0x00000002

// Used to define the new config value, relative to the already configured value
#define OPT_WITHREL 0X00000004

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
     *        - In addition for Custom Resources, the MSB must be set to 1 as well
     */
    uint32_t mResCode;
    /**
     * @brief Holds Logical Core and Cluster Information:
     *        - The last 8 bits (25-32) hold the Logical Core Value
     *        - The next 8 bits (17-24) hold the Logical Cluster Value
     */
    int32_t mResInfo;
    int al;
    int32_t mOptionalInfo; //!< Field to hold optional information for Request Processing
    /**
     * @brief Number of values to be configured for the Resource,
     *        both single-valued and multi-valued Resources are supported.
     */
    int32_t mNumValues;

    union {
        int32_t value; //!< Use this field for single Valued Resources
        int32_t* values; //!< Use this field for Multi Valued Resources
        int32_t val;
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
    MODE_RESUME = 0x01, //!< Tuning allowed when the display is on.
    MODE_SUSPEND = 0x02, //!< Tuning allowed when the display is off.
    MODE_DOZE = 0x04 //!< Tuning allowed during doze (low-power idle) mode.
};

// Helper Macros to set Request and SysResource attributes.
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
    if(properties < 0 || mode < MODE_RESUME || mode > MODE_DOZE) {                            \
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

// Common ResCode and ResInfo codes
// These enums can be used directly as part of tuneResources API, to uniquely
// specify the resource to be tuned.
// For target-specific resources use the custom ResourcesConfig.yaml file for
// declaration. These values are only applicable for upstream resources.
enum ResCodesDef {
    // Resources
    RES_SCALE_MIN_FREQ           = 0x00040000,
    RES_SCALE_MAX_FREQ           = 0x00040001,
    RES_SCHED_UTIL_CLAMP_MIN     = 0x00030000,
    RES_SCHED_UTIL_CLAMP_MAX     = 0X00030001,
    RES_CPU_DMA_LATENCY          = 0X00010000,
    RES_PM_QOS_LATENCY           = 0X00010001,

    // cgroup resources
    RES_CGRP_MOVE_PID            = 0x00090000,
    RES_CGRP_MOVE_TID            = 0x00090001,
    RES_CGRP_RUN_CORES           = 0x00090002,
    RES_CGRP_RUN_CORES_EXCL      = 0x00090003,
    RES_CGRP_FREEZE              = 0x00090004,
    RES_CGRP_LIMIT_CPU_TIME      = 0x00090005,
    RES_CGRP_RUN_WHEN_CPU_IDLE   = 0x00090006,
    RES_CGRP_UCLAMP_MIN          = 0x00090007,
    RES_CGRP_UCLAMP_MAX          = 0x00090008,
    RES_CGRP_REL_CPU_WEIGHT      = 0x00090009,
    RES_CGRP_HIGH_MEM            = 0x0009000a,
    RES_CGRP_MAX_MEM             = 0x0009000b,
    RES_CGRP_LOW_MEM             = 0x0009000c,
    RES_CGRP_MIN_MEM             = 0x0009000d,
    RES_CGRP_SWAP_MAX_MEMORY     = 0x0009000e,
    RES_CGRP_IO_WEIGHT           = 0x0009000f,
    RES_CGRP_BFQ_IO_WEIGHT       = 0x00090010,
    RES_CGRP_CPU_LATENCY         = 0x00090011,

    // ResInfo defintions for common use cases
    // Cluster definitions (in the order of increasing capacity)

    // Little Cluster
    CLUSTER_LITTLE_ALL_CORE      = 0x00000000,
    CLUSTER_LITTLE_CORE_0        = 0x00000001, // First Core in little Cluster.
    CLUSTER_LITTLE_CORE_1        = 0x00000002,
    CLUSTER_LITTLE_CORE_2        = 0x00000003,
    CLUSTER_LITTLE_CORE_3        = 0x00000004,

    // Big Cluster
    CLUSTER_BIG_ALL_CORE         = 0x00000100,
    CLUSTER_BIG_CORE_0           = 0x00000101, // First Core in Big Cluster.
    CLUSTER_BIG_CORE_1           = 0x00000102,
    CLUSTER_BIG_CORE_2           = 0x00000103,
    CLUSTER_BIG_CORE_3           = 0x00000104,

    // Plus / Prime Cluster
    CLUSTER_PLUS_ALL_CORE        = 0x00000200,
    CLUSTER_PLUS_CORE_0          = 0x00000201, // First Core in Prime / Plus Cluster.
    CLUSTER_PLUS_CORE_1          = 0x00000202,
    CLUSTER_PLUS_CORE_2          = 0x00000203,
    CLUSTER_PLUS_CORE_3          = 0x00000204,
};

#endif
