// Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
// SPDX-License-Identifier: BSD-3-Clause-Clear

/*!
 * \file  Utils.h
 */

#ifndef UTILS_H
#define UTILS_H

#include <cstdint>
#include <vector>

#include "ErrCodes.h"
#include "Message.h"
#include "Logger.h"

/**
 * @enum RequestType
 * @brief Enumeration for different types of Resource-Tuner Requests.
 */
enum RequestType {
    REQ_RESOURCE_TUNING,
    REQ_RESOURCE_RETUNING,
    REQ_RESOURCE_UNTUNING,
    REQ_PROP_GET,
    REQ_SIGNAL_TUNING,
    REQ_SIGNAL_UNTUNING,
    REQ_SIGNAL_RELAY
};

/**
 * @enum Permissions
 * @brief Certain resources can be accessed only by system clients and some which have
 *        no such restrictions and can be accessed even by third party clients.
 */
enum Permissions {
    PERMISSION_SYSTEM, //!< Special permission level for system clients.
    PERMISSION_THIRD_PARTY, //!< Third party clients. Default value.
    NUMBER_PERMISSIONS //!< Total number of permissions currently supported.
};

/**
 * @enum RequestPriority
 * @brief Requests can have 2 levels of Priorities, HIGH or LOW.
 */
enum RequestPriority {
    REQ_PRIORITY_HIGH = 0,
    REQ_PRIORITY_LOW,
    NUMBER_REQUEST_PRIORITIES
};

/**
 * @enum PriorityLevel
 * @brief Resource Tuner Priority Levels
 * @details Each Request will have a priority level. This is used to determine the order in which
 *          requests are processed for a specific Resource. The Requests with higher Priority will be prioritized.
 */
enum PriorityLevel {
    SYSTEM_HIGH = 0, // Highest Level of Priority
    SYSTEM_LOW,
    THIRD_PARTY_HIGH,
    THIRD_PARTY_LOW,
    TOTAL_PRIORITIES
};

/**
 * @enum Modes
 * @brief Represents the operational modes based on the device's display state.
 * @details Certain system resources are optimized only when the device display is active,
 *          primarily to conserve power. However, for critical components, tuning may be
 *          performed regardless of the display state, including during doze mode.
 */
enum Modes {
    MODE_DISPLAY_ON  = 0x001, //!< Tuning allowed when the display is on.
    MODE_DISPLAY_OFF = 0x002, //!< Tuning allowed when the display is off.
    MODE_DOZE        = 0x004  //!< Tuning allowed during doze (low-power idle) mode.
};

/**
 * @enum Policy
 * @brief Different Resource Policies supported by Resource Tuner
 * @details Resource Policies determine the order in which the Tuning Requests will be processed for
 *          a particular resource. Currently 4 types of Policies are supported:
 */
enum Policy {
    INSTANT_APPLY, //!< This policy is for resources where the latest request needs to be honored.
    HIGHER_BETTER, //!< This policy first applies the request writing the highest value to the node.
    LOWER_BETTER, //!< Self-explanatory. Works exactly opposite of the higher is better policy.
    LAZY_APPLY //!< The requests are applied in a first-in-first-out manner.
};

typedef struct {
    char* buffer;
    uint64_t bufferSize;
    int64_t handle;
} MsgForwardInfo;

typedef struct {
    std::string mPropName;
    std::string mResult;
    uint64_t mBufferSize;
} PropConfig;

// Global Typedefs: Declare Function Pointers as types
typedef ErrCode (*EventCallback)(void*);
typedef int8_t (*ServerOnlineCheckCallback)();
typedef void (*ResourceTunerMessageReceivedCallback)(int32_t, MsgForwardInfo*);

#define HIGH_TRANSFER_PRIORITY -1
#define SERVER_CLEANUP_TRIGGER_PRIORITY -2

// Config Names
#define COMMON_RESOURCE "Common-Resource"
#define CUSTOM_RESOURCE "Custom-Resource"
#define COMMON_PROPERTIES "Common-Properties"
#define CUSTOM_PROPERTIES "Custom-Properties"
#define CUSTOM_TARGET "Custom-Target"
#define COMMON_INIT "Common-Init"
#define CUSTOM_INIT "Custom-Init"
#define COMMON_SIGNAL "Common-Signal"
#define CUSTOM_SIGNAL "Custom-Signal"
#define CUSTOM_EXT_FEATURE "Ext-Features"

// System Properties
#define MAX_CONCURRENT_REQUESTS "resource_tuner.maximum.concurrent.requests"
#define MAX_RESOURCES_PER_REQUEST "resource_tuner.maximum.resources.per.request"
#define PULSE_MONITOR_DURATION "resource_tuner.pulse.duration"
#define GARBAGE_COLLECTOR_DURATION "resource_tuner.garbage_collection.duration"
#define GARBAGE_COLLECTOR_BATCH_SIZE "resource_tuner.garbage_collection.batch_size"
#define RATE_LIMITER_DELTA "resource_tuner.rate_limiter.delta"
#define RATE_LIMITER_PENALTY_FACTOR "resource_tuner.penalty.factor"
#define RATE_LIMITER_REWARD_FACTOR "resource_tuner.reward.factor"
#define LOGGER_LOGGING_LEVEL "resource_tuner.logging.level"
#define LOGGER_LOGGING_LEVEL_TYPE "resource_tuner.logging.level.exact"
#define LOGGER_LOGGING_OUTPUT_REDIRECT "resource_tuner.logging.redirect_to"

#endif
