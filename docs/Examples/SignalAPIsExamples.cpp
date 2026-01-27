// Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
// SPDX-License-Identifier: BSD-3-Clause-Clear

/**
 * @brief Sample Usage of Resource Tuner Signal APIs.
 *
 * @example SignalAPIsExamples.cpp
 * This files covers examples of the following APIs:
 * - tuneSignal
 * - untuneSignal
 * - relaySignal
 */

#include <iostream>

#include <Urm/UrmAPIs.h>

/*
Signal:
  - SigId: "0x0003"
    Category: "0x0d"
    Name: "SMOOTH_SCROLL"
    Enable: false
    TargetsEnabled: ["sun"]
    Permissions: ["third_party"]
    Derivatives: ["solar"]
    Timeout: 4000
    Resources:
      - {ResCode: "0x00000008", ResInfo: "0x00000000", Values: [300, 400]}
      - {ResCode: "0x000000f1", ResInfo: "0x00000400", Values: [12, 45, 67]}
      - {ResCode: "0x0000abcd", ResInfo: "0x00000020", Values: [5]}
      - {ResCode: "0x0000ea0d", ResInfo: "0x00000200", Values: [87]}
*/
void func1() {
    // Use the tuneSignal to provision or acquire a signal
    // Signature:
    /*
        int64_t tuneSignal(uint32_t signalCode,
                           int64_t duration,
                           int32_t properties,
                           const char* appName,
                           const char* scenario,
                           int32_t numArgs,
                           uint32_t* list);
    */

    // First generate the Signal Code (unsigned 32-bit identifier)

    // Here the SigID "0x0003"
    // and the Category of Signal is "0x0d"
    // The SigCode is a combination of the above fields
    // Let's say this is a custom signal (i.e. one defined by the user)
    // then the SigCode will be as follows:
    // 0x 80 [MSB value of 1 indicates custom Signal] 0d [Category] 0003 [SigID]
    uint32_t sigCode = 0x8000030d;

    /*
     * Alternative Approach to construct opcode
     * sigCode = CUSTOM(CONSTRUCT_SIG_CODE(0x0d, 0x0003))
     */

    // Duration to tune the signal for
    int64_t duration = 20000; // 20 seconds

    int32_t properties = 0;
    // The properties field signifies the Request Priority and the Background Processing status
    // A value of 0 corresponds to: Priority: HIGH, Enabled for Background Processing: False

    // appName and scenario fields are specific to the user-case

    // Finally we have no additional arguments hence, the list argument can be kept as nullptr.

    int64_t handle = tuneSignal(sigCode, duration, properties, "", "", 0, nullptr);
}
