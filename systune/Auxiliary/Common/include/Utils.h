// Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
// SPDX-License-Identifier: BSD-3-Clause-Clear

#ifndef UTILS_H
#define UTILS_H

#include <cstdint>
#include <vector>

#include "Logger.h"

#define PROVISIONER "Provisioner"
#define SYSSIGNAL "SysSignal"

/**
 * @enum RequestType
 * @brief Enumeration for different types of Provisioner and SysConfig requests.
 */
enum RequestType {
    REQ_RESOURCE_TUNING,
    REQ_RESOURCE_RETUNING,
    REQ_RESOURCE_UNTUNING,
    REQ_CLIENT_GET_REQUESTS,
    REQ_SYSCONFIG_GET_PROP,
    REQ_SYSCONFIG_SET_PROP,
};

/**
 * @enum SignalType
 * @brief Enumeration for different types of SysSignal requests.
 */
enum SignalRequestType {
    SIGNAL_ACQ = 10,
    SIGNAL_FREE,
    SIGNAL_RELAY
};

/**
 * @enum Permissions
 * @brief Certain resources can be accessed only by system clients and some which have
 *        no such restrictions and can be accessed even by third party clients.
 */
enum Permissions {
    PERMISSION_SYSTEM, //<! Special permission level for system clients.
    PERMISSION_THIRD_PARTY, //<! Third party clients. Default value.
    NUMBER_OF_PERMISSIONS //<! Trick to get total number of permissions currently supported. Value = 2 at the moment.
};

/**
 * @enum PriorityLevel
 * @brief Each Request will have a priority level. This is used to determine the order in which
 *        requests are processed for a specific Resource. The Requests with higher Priority will be prioritized.
 */
enum PriorityLevel {
    SYSTEM_HIGH, // Highest Level of Priority
    SYSTEM_LOW,
    THIRD_PARTY_HIGH,
    THIRD_PARTY_LOW,
};

#define TOTAL_PRIORITIES 4 // Total number of priority levels currently supported. Value = 4 at the moment.
#define HIGH_TRANSFER_PRIORITY -1
#define SERVER_CLEANUP_TRIGGER_PRIORITY -2

/**
 * @enum Modes
 * @brief Represents the operational modes based on the device's display state.
 *
 * Certain system resources are optimized only when the device display is active,
 * primarily to conserve power. However, for critical components, tuning may be
 * performed regardless of the display state, including during doze mode.
 */
enum Modes {
    MODE_DISPLAY_ON  = 0x001, //<! Tuning allowed when the display is on.
    MODE_DISPLAY_OFF = 0x002, //<! Tuning allowed when the display is off.
    MODE_DOZE        = 0x004  //<! Tuning allowed during doze (low-power idle) mode.
};

/**
 * - Resource Policies determine the order in which the Tuning Requests will be processed for
 *   particular resource. Currently 4 types of Policies are supported:
 * - 1) Instant Apply (or Always Apply): This policy is for resources where the latest request needs to be honored.
 *   This is kept as the default policy.
 * - 2) Higher is better: This policy honors the request writing the highest value to the node.
 *   One of the cases where this makes sense is for resources that describe the upper bound value.
 *   By applying the higher-valued request, the lower-valued request is implicitly honored.
 * - 3) Lower is better: Self-explanatory. Works exactly opposite of the higher is better policy.
 * - 4) Lazy Apply: Sometimes, you want the resources to apply requests in a first-in-first-out manner.
 */
enum Policy {
    INSTANT_APPLY,
    HIGHER_BETTER,
    LOWER_BETTER,
    LAZY_APPLY
};

/**
 * @enum ConfigType
 * @brief Different Config (via JSON) types supported. Note, the Config File corresponding to each config type
 *        can be altered via the Extensions interface.
 */
enum ConfigType {
    RESOURCE_CONFIG,
    PROPERTIES_CONFIG,
    SIGNALS_CONFIG,
    EXT_FEATURES_CONFIG,
    TARGET_CONFIG,
    TOTAL_CONFIGS_COUNT
};

enum ClusterTypes {
    BIG = 0,
    LITTLE,
    PRIME,
    TITANIUM,
    TOTAL_CLUSTER_COUNT
};

#define SERVER_ONLINE_CHECK_CALLBACK 150
#define PROVISIONER_MESSAGE_RECEIVER_CALLBACK 180
#define SYSCONFIG_MESSAGE_RECEIVER_CALLBACK 181
#define PROVISIONER_SEND_DATA_TO_BUFFER_CALLBACK 182
#define SIGNAL_MESSAGE_RECEIVER_CALLBACK 200

#endif
