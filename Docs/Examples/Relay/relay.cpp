// Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
// SPDX-License-Identifier: BSD-3-Clause-Clear

#include <iostream>

extern "C" void initFeature() {
    // Use this function for initialization
}

extern "C" void tearFeature() {
    // Use this function for cleanup
}

extern "C" void relayFeature() {
    // This function will be called when a relaySignal request is called for any Signal
    // which is subscribed to this feature.
    // For example:
    // signal |  features
    // s1     |  f1, f2, f5
    // s2     |  f5, f7, f8
    // s3     |  f9, f10

    // When a relaySignal API request is received for s2, it will be relayed (forwarded) to
    // features: f5, f7 and f8.
}

/*
 * Compilation Notes:
 * To build the above code, it needs to be linked with ExtAPIs lib exposed by Resource Tuner,
 * and built as a shared lib:
 * => Create the shared lib:
 *    "g++ -fPIC -shared -o libplugin.so plugin.cpp -lExtAPIs"
 *    This creates a shared lib, libplugin.so
 * => This lib can be placed in any location, the full path to the lib needs to be specified
 *    in the ExtFeaturesConfig.yaml file.
 * => Make sure the lib file has appropriate permissions:
 *    "sudo chmod o+r <path_to_lib>"
*/
