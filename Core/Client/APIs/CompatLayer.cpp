// Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
// SPDX-License-Identifier: BSD-3-Clause-Clear

#include <cstdint>
#include <cstring>
#include <vector>

#include "Logger.h"
#include "CompatLayer.h"

#define MULTI_VALUE_BIT 29
#define MINOR_TP_LR_BIT 14
#define MINOR_TP_HR_BIT 19
#define MAJOR_TP_LR_BIT 22
#define MAJOR_TP_HR_BIT 28
#define CORE_VAL_LR_BIT 4
#define CORE_VAL_HR_BIT 6
#define CLUSTER_VAL_LR_BIT 8
#define CLUSTER_VAL_HR_BIT 11

static const int32_t majorMask = ((1 << (MAJOR_TP_HR_BIT + 1)) - 1) - ((1 << (MAJOR_TP_LR_BIT)) - 1);
static const int32_t minorMask = ((1 << (MINOR_TP_HR_BIT + 1)) - 1) - ((1 << (MINOR_TP_LR_BIT)) - 1);
static const int32_t coreMask = ((1 << (CORE_VAL_HR_BIT + 1)) - 1) - ((1 << (CORE_VAL_LR_BIT)) - 1);
static const int32_t clusterMask = ((1 << (CLUSTER_VAL_HR_BIT + 1)) - 1) - ((1 << (CLUSTER_VAL_LR_BIT)) - 1);

#define REQ_SEND_ERR(e) "Failed to send Request to Server, Error: " + std::string(e)
#define CONN_SEND_FAIL "Failed to send Request to Server"
#define CONN_INIT_FAIL "Failed to initialize Connection to resource-tuner Server"

int32_t perf_lock_acq(int32_t handle, int32_t duration, int32_t list[], int32_t numArgs) {
    try {
        // If the handle is positive, then this is a renew request
        if(handle > 0) {
            return retuneResources(handle, duration);
        }

        // New Resource Provisioning Request
        // Decode the list array into the format which tuneResources accepts
        std::vector<SysResource> resourceVector;

        // Used to track if we are dealing with resource opcode or config value
        int32_t currResValCount = 0;
        int32_t currValIndex = 0;
        int32_t i = 0;

        while(i < numArgs) {
            if(currResValCount == 0) {
                // Start of a Resource
                SysResource currResource;
                currResource.mResCode = 0;
                currResource.mResCode |= (int32_t)(list[i] & minorMask);
                currResource.mResCode |= ((int32_t)(list[i] & majorMask)) << 16;

                currResource.mResInfo = 0;
                currResource.mResInfo =
                    SET_RESOURCE_CLUSTER_VALUE(currResource.mResInfo, (list[i] & clusterMask));
                currResource.mResInfo =
                    SET_RESOURCE_CORE_VALUE(currResource.mResInfo, (list[i] & coreMask));

                currResource.mNumValues = 1;

                if(list[i] & (1 << MULTI_VALUE_BIT)) {
                    // Multi-valed resource
                    if(i + 1 < numArgs) {
                        currResource.mNumValues = list[i + 1];
                        currResource.mResValue.values = new int32_t[currResource.mNumValues];

                        // Move to the values
                        i++;
                        currValIndex = 0;
                    } else {
                        LOGE("RESTUNE_CLIENT", "Malformed Request");
                        return -1;
                    }
                }

                currResValCount = currResource.mNumValues;
                resourceVector.push_back(currResource);

            } else {
                if(resourceVector.empty()) {
                    LOGE("RESTUNE_CLIENT", "Malformed Request");
                    return -1;
                }

                if(resourceVector.back().mNumValues == 1) {
                    resourceVector.back().mResValue.value = list[i];
                } else {
                    resourceVector.back().mResValue.values[currValIndex] = list[i];
                    currValIndex++;
                }
                currResValCount--;
            }
            i++;
        }

        return tuneResources(duration == 0 ? -1 : duration,
                             RequestPriority::REQ_PRIORITY_HIGH,
                             resourceVector.size(),
                             resourceVector.data());

    } catch(const std::exception& e) {
        LOGE("RESTUNE_CLIENT", REQ_SEND_ERR(e.what()));
        return -1;
    }
}

int32_t perf_lock_rel(int32_t handle) {
    return (int32_t) untuneResources(handle);
}

PropVal perf_get_prop(const char* prop , const char* defVal) {
    int8_t status = 0;
    PropVal rMsg = {""};
    status = getProp(prop, rMsg.value, sizeof(PropVal), defVal);

    if(status < 0) {
        strncpy(rMsg.value, defVal, sizeof(PropVal));
        rMsg.value[sizeof(PropVal) - 1] = '\0';
    }

    return rMsg;
}

int32_t perf_lock_acq_rel(int32_t handle, int32_t duration, int32_t list[], int32_t numArgs, int32_t reserveNumArgs) {
    if(untuneResources(handle) == 0) {
        return perf_lock_acq(0, duration, list, numArgs);
    }

    return -1;
}
