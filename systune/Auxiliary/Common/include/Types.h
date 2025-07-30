// Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
// SPDX-License-Identifier: BSD-3-Clause-Clear

/*!
 * \file  Types.h
 */

#ifndef TYPES_H
#define TYPES_H

#include <vector>
#include <cstdint>

#include "ErrCodes.h"
#include "Message.h"

/**
 * @class Resource
 * @brief Represents a sysfs node that needs to be written to by the client.
 * @details If the number of values to write is just one, a single value is stored
 *          otherwise a pointer to a vector of values is stored.
 * @struct
 */
class Resource {
public:
    uint32_t mOpId;
    int32_t mOpInfo;
    int32_t mOptionalInfo;
    int32_t mNumValues;
    union {
        int32_t singleValue;
        std::vector<int32_t>* valueArray;
    } mConfigValue;
};

#define EXTRACT_RESOURCE_CORE_VALUE(optionalInfo) ({ \
    (int32_t)(optionalInfo) & ((1 << 8) - 1); \
})

#define EXTRACT_RESOURCE_CLUSTER_VALUE(optionalInfo)({ \
    (int32_t) (optionalInfo >> 8) & ((1 << 8) - 1); \
}) \

#define SET_RESOURCE_CORE_VALUE(optionalInfo, newValue)({ \
    (int32_t) (optionalInfo ^ EXTRACT_RESOURCE_CORE_VALUE(optionalInfo)) | newValue;  \
}) \

#define SET_RESOURCE_CLUSTER_VALUE(optionalInfo, newValue)({ \
    (int32_t) (optionalInfo ^ (EXTRACT_RESOURCE_CLUSTER_VALUE(optionalInfo) << 8)) | (newValue << 8);  \
}) \

typedef struct _cocoNode {
    Resource* mResource; //<! Pointer to the resource.
    _cocoNode* next; //<! Pointer to the next node in the list.
    _cocoNode* prev; //<! Pointer to the previous node in the list.
} CocoNode;

// Declare Function Pointers as types
typedef ErrCode (*ModuleCallback)();
typedef void (*ModuleMessageHandlerCallback)(void*);
typedef int64_t (*SystuneMessageAsyncCallback)(int32_t, Message*);
typedef int8_t (*SystuneMessageSyncCallback)(int8_t, void*, char*, uint64_t);
typedef void (*ResourceApplierCallback)(void*);

#endif
