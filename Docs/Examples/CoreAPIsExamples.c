// Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
// SPDX-License-Identifier: BSD-3-Clause-Clear

/**
 * @brief Resource Tuner APIs can be integrated and called as part of a C-based program
 *        as well. This file provides an example of using the APIs with C.
 * @example CoreAPIsExamples.c
 */

#include <stdio.h>
#include <stdlib.h>

#include <ResourceTuner/ResourceTunerAPIs.h>

#define UCLAMP_MIN_RES 0x00030000
#define UCLAMP_BOOST_VAL 750
#define TUNE_DURATION 5000 #duration in msec
#define FAIL -1

static void tune_uclamp_min() {
    int32_t properties = 0;

    properties = SET_REQUEST_PRIORITY(properties, REQ_PRIORITY_LOW);
    properties = ADD_ALLOWED_MODE(properties, MODE_SUSPEND);

    SysResource resourceList[] = {
        {
            .mResCode = UCLAMP_MIN_RES,
            .mResInfo = 0,
            .mOptionalInfo = 0,
            .mNumValues = 1,
            .mResValue = {
                .value = UCLAMP_BOOST_VAL,
            }
        }
    };

    int64_t handle = tuneResources(TUNE_DURATION, properties, 1, resourceList);
    if(handle == FAIL) {
        printf("Request Could not be Sent to the Resource Tuner Server\n");
    } else {
        printf("Handle Returned is: %ld\n", handle);
    }
}

// Similary other ResourceTuner APIs can be used as well by C-based programs.

int32_t main(int32_t argc, char* argv[]) {
    tune_uclamp_min();
}

// Compilation Notes:
// The executable needs to be linked to the ClientAPIs lib, where these APIs
// are defined. This can be done, as follows:
// GCC: gcc ResourceTunerCoreAPIs.c -o ResourceTunerCoreAPIs -lClientAPIs
// CMake: This can be done as part of the C/C++ project by adding the Library
// to the target link libraries. For example, if the executalbe is called clientExec,
// it can be linked as follows:
// target_link_libraries(clientExec ClientAPIs)
