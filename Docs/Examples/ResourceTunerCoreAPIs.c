// Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
// SPDX-License-Identifier: BSD-3-Clause-Clear

/**
 * @brief Resource Tuner APIs can be integrated and called as part of a C-based program
 *        as well. This file provides an example of using the APIs with C.
 * @example ResourceTunerCoreAPIs.c
 */

#include <stdio.h>
#include <stdlib.h>

#include <ResourceTuner/ResourceTunerAPIs.h>

static void func1() {
    // Refer ResourceTunerCoreAPIs.cpp func1 for details regarding each of
    // the following fields.
    int64_t duration = 5000;
    int32_t properties = 0;
    properties |= (1 << 0);
    properties |= (1 << 8);

    SysResource* resourceList = (SysResource*)calloc(1, sizeof(SysResource));

    resourceList[0].mOpCode = 0x00030000;
    resourceList[0].mOpInfo = 0;
    resourceList[0].mOptionalInfo = 0;
    resourceList[0].mNumValues = 1;
    resourceList[0].mConfigValue.singleValue = 750;

    int64_t handle = tuneResources(duration, properties, 1, resourceList);

    // Check the Returned Handle
    if(handle == -1) {
        printf("Request Could not be Sent to the Resource Tuner Server\n");
    } else {
        printf("Handle Returned is: %ld\n", handle);
    }
}

// Similary other ResourceTuner APIs can be used as well by C-based programs.

int32_t main(int32_t argc, char* argv[]) {
    func1();
}

// Compilation Notes:
// The executable needs to be linked to the ClientAPIs lib, where these APIs
// are defined. This can be done, as follows:
// GCC: gcc ResourceTunerCoreAPIs.c -o ResourceTunerCoreAPIs -lClientAPIs
// CMake: This can be done as part of the C/C++ project by adding the Library
// to the target link libraries. For example, if the executalbe is called clientExec,
// it can be linked as follows:
// target_link_libraries(clientExec ClientAPIs)
