// Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
// SPDX-License-Identifier: BSD-3-Clause-Clear

/**
 * @brief Sample Usage of Resource Tuner APIs.
 *
 * @example ResourceTunerCoreAPIs.cpp
 * This files covers examples of the following APIs:
 * - tuneResources
 * - retuneResources
 * - untuneResources
 */
#include <iostream>

#include <ResourceTuner/ResourceTunerAPIs.h>

// EXAMPLE #1
// In the following Example the Client Sends:
// - A Resource Provisioning (Tune) Request to Tune 1 Resource
//   i.e. We try to Provision a Single Resource as part of this Request,
//   The Resource Config for this Resource is as follows (note, Configs are specified via YAML files):

/*
Resource:
  - ResType: "0x03"
    ResID: "0x0000"
    Name: "/proc/sys/kernel/sched_util_clamp_min"
    Supported: true
    HighThreshold: 1024
    LowThreshold: 0
    Permissions: "third_party"
    Modes: ["display_on", "doze"]
    Policy: "higher_is_better"
    CoreLevelConflict: false
*/
void func1() {
    // First Create a List of Resources to be Provisioned as part of this Request
    // Note multiple Resources can be part of a single Resource Tuner Request

    // Specify the duration as a 64-bit Signed Integer. Note, this value specifies the
    // Time Interval in milliseconds.
    int64_t duration = 5000; // Equivalent to 5 seconds

    // Create a 32 bit integer which specifies the Request Properties
    // This field actually encodes two values:
    // - Priority (8 bits: 25 - 32)
    // - Background Processing Status (8 bits: 17 - 24):
    //   Should the Request be Processed in Background (i.e. when the display is in
    //   Off or Doze Mode).
    int32_t properties = 0;

    // Default Value of 0, corresponds to a Priority of High
    // and Background Processing Status of False.

    // Create Helpers for these
    // To set the Priority as Low
    properties |= (1 << 0); // REQ_PRIORITY_LOW corresponds to a value of 1.

    // To set the Background Processing Status as True
    properties |= (1 << 8);

    // Create the List of Resources which need to be Provisioned
    // Resource Struct Creation
    SysResource* resourceList = new SysResource[1];

    // Initialize Resource struct Fields:

    // Field: mOpCode:
    // Resource Opcode is a unsigned 32 bit integer
    // The last 16 bits (17-32) are used to specify the ResId
    // The next 8 bits (9-16) are used to specify the ResType (type of the Resource)
    // In addition if you are using Custom Resources, then the MSB must be set to 1 as well.
    // In this case we are dealing with Default Resources, so need to set the MSB to 1.

    resourceList[0].mOpCode = 0x00030000;

    // Field: mOpInfo
    // This field is a 32-bit signed integer, which stores information
    // Regarding the Logical Core and Cluster values.
    // These logical Values will be Translated to their corresponding
    // Physical values in the background and the configured Resource Values
    // will only take effect on the Specified Physical Core and Cluster.
    // Note this Value is only meaningful for Resources for which the Config
    // Field "CoreLevelConflict" is set to True.
    // In this case since CoreLevelConflict is false for R1, hence this field
    // will not be processed by the Resource Tuner Server
    resourceList[0].mOpInfo = 0;
    // Note, above line of Code is not necessary, since the field is already initialized
    // to 0 via the Constructor.

    // Field: mOptionalInfo
    // TODO
    resourceList[0].mOptionalInfo = 0;
    // Note, above line of Code is not necessary, since the field is already initialized
    // to 0 via the Constructor.

    // Field: mNumValues
    // Number of Values to be Configured for this Resource
    // Resource Tuner supports both Single and Multi Valued Resources
    // Here we consider the example for a single Valued Resource:
    resourceList[0].mNumValues = 1;

    // Field: mConfigValue
    // The value to be Configured for this Resource Node.
    // mConfigValue is a union, which contains 2 fields:
    // int32_t singleValue [Use this field for single Valued Resources]
    // std::vector<int32_t>* valueArray [Use this field for Multi Valued Resources]
    // Here since we are dealing with a Single Valued Resource, hence we'll use
    // the 32 bit integer field (singleValue)
    // Let's say we want to configure a value of 750 for this Resource,
    // Notice from the Resource Config that the allowed Configurable Range for this
    // Resource is [0 - 1024].
    resourceList[0].mConfigValue.singleValue = 750;

    // Now our Resource List is fully constructed

    // Finally we can issue the Resource Provisioning (or Tune) Request
    int64_t handle = tuneResources(duration, properties, 1, resourceList);

    // Check the Returned Handle
    if(handle == -1) {
        std::cout<<"Request Could not be Sent to the Resource Tuner Server"<<std::endl;
    } else {
        std::cout<<"Handle Returned is: "<<handle<<std::endl;
    }

    // This handle Value can be used for Future Untune / Retune Requests.
    // Note the Memory allocations made for Resource List will be freed
    // automatically by the Client Library, and should not be done by the Client itself.
}


// EXAMPLE #2
// In the following Example the Client Sends:
// - A Resource Provisioning (Tune) Request to tune a resource, for an infinite duration.
// - Later the Client Sends an Untune Request to withdraw the previously issued Tune
//   Request, i.e. the Resource will be restored to its original Value.
void func2() {
    // Like func1, we first setup the API params
    // To Provision the Resources for an infinite duration, specify the duration
    // param in the tuneResources API as -1.
    int64_t duration = -1;

    // Setup Request Properties
    int32_t properties = 0;

    // Here the Priority as Low
    properties |= (1 << 0); // REQ_PRIORITY_LOW corresponds to a value of 1.

    // Set the Background Processing Status as True
    properties |= (1 << 8);

    // Create the List of Resources which need to be Provisioned
    // Resource Struct Creation
    SysResource* resourceList = new SysResource[1];

    // Initialize Resource struct Fields:

    // Field: mOpCode:
    // Refer func1 for details
    resourceList[0].mOpCode = 0x00030000;

    // Field: mOpInfo
    // Refer func1 for details
    resourceList[0].mOpCode = 0;
    // Note, above line of Code is not necessary, since the field is already initialized
    // to 0 via the Constructor.

    // Field: mOptionalInfo
    // Refer func1 for details
    resourceList[0].mOptionalInfo = 0;
    // Note, above line of Code is not necessary, since the field is already initialized
    // to 0 via the Constructor.

    // Field: mNumValues
    // Number of Values to be Configured for this Resource
    // Resource Tuner supports both Single and Multi Valued Resources
    // Here we consider the example for a single Valued Resource:
    resourceList[0].mNumValues = 1;

    // Field: mConfigValue
    // Refer func1 for details
    resourceList[0].mConfigValue.singleValue = 884;

    // Now our Resource struct is fully constructed

    // Finally we can issue the Resource Provisioning (or Tune) Request
    int64_t handle = tuneResources(duration, properties, 1, resourceList);

    // Check the Returned Handle
    if(handle == -1) {
        std::cout<<"Tune Request could not be sent to the Resource Tuner Server"<<std::endl;
        return;

    } else {
        std::cout<<"Handle Returned is: "<<handle<<std::endl;
    }

    // After some time, say the Client wishes to withdraw the previously issued
    // Resource Provisioning Request, i.e. restore the Resources to their original Value.

    // Issue an Untune Request
    int8_t status = untuneResources(handle);

    if(status == -1) {
        std::cout<<"Untune Request could not be sent to the Resource Tuner Server"<<std::endl;
    }
}


// EXAMPLE #3
// In the following Example the Client Sends:
// - A Resource Provisioning (Tune) Request to tune a resource, for some duration.
// - Later the Client wishes to Modify the duration of this Request, to do so
//   a Retune Request is issued to the Resource Tuner Server.
void func3() {
    // Provision for 8 seconds (8000 milliseconds)
    int64_t duration = 8000;

    // Setup Request Properties
    int32_t properties = 0;

    // Here the Priority as Low
    properties |= (1 << 0); // REQ_PRIORITY_LOW corresponds to a value of 1.

    // Set the Background Processing Status as True
    properties |= (1 << 8);

    // Create the List of Resources which need to be Provisioned
    // Resource Struct Creation
    SysResource* resourceList = new SysResource[1];

    // Initialize Resource struct Fields:

    // Field: mOpCode:
    // Refer func1 for details
    resourceList[0].mOpCode = 0x00030000;

    // Field: mOpInfo
    // Refer func1 for details
    resourceList[0].mOpInfo = 0;
    // Note, above line of Code is not necessary, since the field is already initialized
    // to 0 via the Constructor.

    // Field: mOptionalInfo
    // Refer func1 for details
    resourceList[0].mOptionalInfo = 0;
    // Note, above line of Code is not necessary, since the field is already initialized
    // to 0 via the Constructor.

    // Field: mNumValues
    // Number of Values to be Configured for this Resource
    // Resource Tuner supports both Single and Multi Valued Resources
    // Here we consider the example for a single Valued Resource:
    resourceList[0].mNumValues = 1;

    // Field: mConfigValue
    // Refer func1 for details
    resourceList[0].mConfigValue.singleValue = 884;

    // Now our Resource struct is fully constructed

    // Finally we can issue the Resource Provisioning (or Tune) Request
    int64_t handle = tuneResources(duration, properties, 1, resourceList);

    // Check the Returned Handle
    if(handle == -1) {
        std::cout<<"Tune Request could not be sent to the Resource Tuner Server"<<std::endl;
        return;

    } else {
        std::cout<<"Handle Returned is: "<<handle<<std::endl;
    }

    // After some time, say the Client wishes to extend the duration of the previously
    // issued Resource Provisioning Request, they can do so by using the retuneResources API.

    // Issue a Retune Request
    int64_t newDuration = 20000;
    int8_t status = retuneResources(handle, newDuration);

    if(status == -1) {
        std::cout<<"Untune Request could not be sent to the Resource Tuner Server"<<std::endl;
    }
}


int32_t main(int32_t argc, char* argv[]) {
    func1();
}

// Compilation Notes:
// The executable needs to be linked to the ClientAPIs lib, where these APIs
// are defined. This can be done, as follows:
// GCC: g++ ResourceTunerCoreAPIs.cpp -o ResourceTunerCoreAPIs -lClientAPIs
// CMake: This can be done as part of the C/C++ project by adding the Library
// to the target link libraries. For example, if the executalbe is called clientExec,
// it can be linked as follows:
// target_link_libraries(clientExec ClientAPIs)
