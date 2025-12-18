// Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
// SPDX-License-Identifier: BSD-3-Clause-Clear

/**
 * @brief Sample Usage of Resource Tuner's Extension APIs.
 *
 * @example Plugin.cpp
 * This files covers examples of the following APIs:
 * - RESTUNE_REGISTER_APPLIER_CB
 * - RESTUNE_REGISTER_CONFIG
 */

#include "Extensions.h"

void customApplyCB(void* context) {
    // Actual Processing
}

void customTearCB(void* context) {
    // Actual Processing
}

__attribute__((constructor))
void registerWithUrm() {
    // Associate the callback (handler) to the desired Resource (ResCode).
    RESTUNE_REGISTER_APPLIER_CB(0x00030000, customApplyCB);
    RESTUNE_REGISTER_TEAR_CB(0x00040a22, customTearCB);
}

/*
 * Compilation Notes:
 * To build the above code, it needs to be linked with RestuneExtAPIs lib exposed by Resource Tuner,
 * and built as a shared lib:
 * => Create the shared lib:
 *    "g++ -fPIC -shared -o libplugin.so Plugin.cpp -lExtAPIs"
 *    This creates a shared lib, libplugin.so
 * => Copy this lib to "/etc/resource-tuner/Custom", the location where Resource Tuner expects
 *    the custom Extensions lib to be placed.
 * => Make sure the lib file has appropriate permissions:
 *    "sudo chmod o+r /etc/resource-tuner/Custom/libplugin.so"
 */
