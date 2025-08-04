/**
 * @brief Sample Usage of Systune APIs.
 *
 * @example SystuneCoreAPIs.cpp
 * This files covers examples of the following APIs:
 * - tuneResources
 * - retuneResources
 * - untuneResources
 */
#include <Systune/SystuneAPIs.h>

// EXAMPLE #1
// In the following Example the Client Sends:
// - A Resource Provisioning (Tune) Request to Tune 1 Resource
//   i.e. We try to Provision a Single Resource as part of this Request,
//   The Resource Config for this Resource is as follows (note Configs are specified as JSON files):

/*
Resource:
    {
        "ResType": "0x1",
        "ResID":"0x0",
        "Name":"/proc/sys/kernel/sched_util_clamp_min",
        "Supported":true,
        "HighThreshold": 1024,
        "LowThreshold": 0,
        "Permissions": "third_party",
        "Modes": ["display_on", "doze"],
        "Policy": "higher_is_better",
        "CoreLevelConflict": false
    }
*/
void func1() {
    // First Create a List of Resources to be Provisioned as part of this Request
    // Note multiple Resources can be part of a single Systune Request

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

    Resource* resource = new Resource;
    // Initialize Resource struct Fields:

    // Field: mOpCode:
    // Resource Opcode is a unsigned 32 bit integer
    // The last 16 bits (17-32) are used to specify the ResId
    // The next 8 bits (9-16) are used to specify the ResType (type of the Resource)
    // In addition if you are using Custom Resources, then the MSB must be set to 1 as well.
    // In this case we are dealing with Default Resources, so need to set the MSB to 1.

    // To set the Resource OpCode
    // Option 1: 1) Generate the OpCode (unsigned 32 bit integer) according to the above rules
    //           2) Call resource->setOpCode(<generate_opcode>).
    // Option 2: 1) Call resource->setResourceID(<res_id>).
    //           2) Call resource->setResourceType(<res_type>).
    //           3) Call resource->setAsCustom() in case of Custom Resource.
    // The second option will internally generate and set the Required OpCode.
    // In this case we'll use Option 2:
    resource->setResourceType(1);
    resource->setResourceID(0);
    // We don't need to call resource->setAsCustom(), since we are dealing
    // with Default Resources in this example.

    // Field: mOpInfo
    // This field is a 32-bit signed integer, which stores information
    // Regarding the Logical Core and Cluster values.
    // These logical Values will be Translated to their corresponding
    // Physical values in the background and the configured Resource Values
    // will only take effect on the Specified Physical Core and Cluster.
    // Note this Value is only meaningful for Resources for which the Config
    // Field "CoreLevelConflict" is set to True.
    // In this case since CoreLevelConflict is false for R1, hence this field
    // will not be processed by the Systune Server
    resource->setOperationalInfo(0);
    // Note, above line of Code is not necessary, since the field is already initialized
    // to 0 via the Constructor.

    // Field: mOptionalInfo
    // TODO
    resource->setOptionalInfo(0);
    // Note, above line of Code is not necessary, since the field is already initialized
    // to 0 via the Constructor.

    // Field: mNumValues
    // Number of Values to be Configured for this Resource
    // Systune supports both Single and Multi Valued Resources
    // Here we consider the example for a single Valued Resource:
    resource->setNumValues(1);

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
    resource->mConfigValue.singleValue = 750;

    // Now our Resource struct is fully constructed
    // Next, create a list to hold the Resources to be provisioned as part of this Request
    std::vector<Resource*>* resources = new std::vector<Resource*>();
    resources->push_back(resource);

    // Finally we can issue the Resource Provisioning (or Tune) Request
    int64_t handle = tuneResources(duration, properties, resources->size(), resources);

    // Check the Returned Handle
    if(handle == -1) {
        std::cout<<"Request Could not be Sent to the Systune Server"<<std::endl;
    } else {
        std::cout<<"Handle Returned is: "<<handle<<std::endl;
    }

    // This handle Value can be used for Future Untune / Retune Requests.
    // Not the Memory allocations made for Resource and Resource List will be freed
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
    Resource* resource = new Resource;

    // Initialize Resource struct Fields:

    // Field: mOpCode:
    // Refer func1 for details
    resource->setResourceType(1);
    resource->setResourceID(0);

    // Field: mOpInfo
    // Refer func1 for details
    resource->setOperationalInfo(0);
    // Note, above line of Code is not necessary, since the field is already initialized
    // to 0 via the Constructor.

    // Field: mOptionalInfo
    // Refer func1 for details
    resource->setOptionalInfo(0);
    // Note, above line of Code is not necessary, since the field is already initialized
    // to 0 via the Constructor.

    // Field: mNumValues
    // Number of Values to be Configured for this Resource
    // Systune supports both Single and Multi Valued Resources
    // Here we consider the example for a single Valued Resource:
    resource->setNumValues(1);

    // Field: mConfigValue
    // Refer func1 for details
    resource->mConfigValue.singleValue = 884;

    // Now our Resource struct is fully constructed
    // Next, create a list to hold the Resources to be provisioned as part of this Request
    std::vector<Resource*>* resources = new std::vector<Resource*>();
    resources->push_back(resource);

    // Finally we can issue the Resource Provisioning (or Tune) Request
    int64_t handle = tuneResources(duration, properties, resources->size(), resources);

    // Check the Returned Handle
    if(handle == -1) {
        std::cout<<"Tune Request could not be sent to the Systune Server"<<std::endl;
        return;

    } else {
        std::cout<<"Handle Returned is: "<<handle<<std::endl;
    }

    // After some time, say the Client wishes to withdraw the previously issued
    // Resource Provisioning Request, i.e. restore the Resources to their original Value.

    // Issue an Untune Request
    ErrCode errCode = untuneResources(handle);

    if(RC_IS_NOTOK(errCode)) {
        std::cout<<"Untune Request could not be sent to the Systune Server"<<std::endl;
    }
}


// EXAMPLE #3
// In the following Example the Client Sends:
// - A Resource Provisioning (Tune) Request to tune a resource, for some duration.
// - Later the Client wishes to Modify the duration of this Request, to do so
//   a Retune Request is issued to the Systune Server.
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
    Resource* resource = new Resource;

    // Initialize Resource struct Fields:

    // Field: mOpCode:
    // Refer func1 for details
    resource->setResourceType(1);
    resource->setResourceID(0);

    // Field: mOpInfo
    // Refer func1 for details
    resource->setOperationalInfo(0);
    // Note, above line of Code is not necessary, since the field is already initialized
    // to 0 via the Constructor.

    // Field: mOptionalInfo
    // Refer func1 for details
    resource->setOptionalInfo(0);
    // Note, above line of Code is not necessary, since the field is already initialized
    // to 0 via the Constructor.

    // Field: mNumValues
    // Number of Values to be Configured for this Resource
    // Systune supports both Single and Multi Valued Resources
    // Here we consider the example for a single Valued Resource:
    resource->setNumValues(1);

    // Field: mConfigValue
    // Refer func1 for details
    resource->mConfigValue.singleValue = 884;

    // Now our Resource struct is fully constructed
    // Next, create a list to hold the Resources to be provisioned as part of this Request
    std::vector<Resource*>* resources = new std::vector<Resource*>();
    resources->push_back(resource);

    // Finally we can issue the Resource Provisioning (or Tune) Request
    int64_t handle = tuneResources(duration, properties, resources->size(), resources);

    // Check the Returned Handle
    if(handle == -1) {
        std::cout<<"Tune Request could not be sent to the Systune Server"<<std::endl;
        return;

    } else {
        std::cout<<"Handle Returned is: "<<handle<<std::endl;
    }

    // After some time, say the Client wishes to extend the duration of the previously
    // issued Resource Provisioning Request, they can do by using the retuneResources API.

    // Issue a Retune Request
    ErrCode errCode = retuneResources(handle);

    if(RC_IS_NOTOK(errCode)) {
        std::cout<<"Untune Request could not be sent to the Systune Server"<<std::endl;
    }
}

int32_t main(int32_t argc, char* argv[]) {
    func1();
}
