/**
 * @brief Sample Usage of Systune APIs.
 *
 * @example SystuneCoreAPIs.cpp
 * This files covers examples of the following APIs:
 * - tuneResources
 * - retuneResources
 * - untuneResources
 */
#include <SystuneAPIs.h>

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

    // Create a 32 bit integer which specifies the Resource Properties
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
    // In this Example we  2 Resources
    // First Resource

    Resource* resource = (Resource*) malloc(sizeof(Resource));
    // Initialize Resource struct Fields:

    // Field: mOpCode:
    // Resource Opcode is a unsigned 32 bit integer
    // The last 16 bits (17-32) are used to specify the ResId
    // The next 8 bits (9-16) are used to specify the ResType (type of the Resource)
    // In addition if you are using Custom Resources, the the MSB must be set to 1 as well.

    // Here, for the First Resource the configuration is as follows:
    uint32_t resourceOpcode = 0;
    resourceOpcode = SET_RESOURCE_ID(resourceOpcode, 0);
    resourceOpcode = SET_RESOURCE_TYPE(resourceOpcode, 1);
    resource->setOpCode(resourceOpcode);

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
    resource->mOpInfo = 0;  // Not necessary, since the field is already initialized to 0
                            // via the Constructor.

    // Field: mOptionalInfo
    // TODO
    resource->mOptionalInfo = 0;   // Not necessary, since the field is already initialized to 0
                                   // via the Constructor.

    // Field: mNumValues
    // Number of Values to be Configured for this Resource
    // Systune supports both Single and Multi Valued Resources
    // Here we consider the example for a single Valued Resource:
    resource->mNumValues = 1;

    // Field: mConfigValue
    // The value to be Configured for this Resource Node.
    // mConfigValue is a union, which contains 2 fields:
    // int32_t singleValue [Use this field for single Value Resources]
    // std::vector<int32_t>* valueArray [Use this field for Multi Valued Resources]
    // Here since we are dealing with a Single Valued exaple, hence we'll use
    // the 32 bit integer field (singleValue)
    // Let's say we want to configure a value of 750 for this Resource,
    // Notice the allowed Configurable Range for this Resource is [0 - 1024].
    resource->mConfigValue.singleValue = 750;

    // Now our Resource struct is fully constructed
    // Next, create a list to hold the Resources to be provisioned as part of this Request
    std::vector<Resource*>* resources = std::vector<Resource*>();
    resources->push_back(resource);

    // Finally we can issue  the Resource Provisioning (or Tune) Request
    int64_t handle = tuneResources(duration, properties, resources->size(), resources);

    // Check the Returned Handle
    if(RC_IS_OK(handle)) {
        std::cout<<"Handle Returned is: "<<handle<<std::endl;
    } else {
        std::cout<<"Request Could not be Sent to the Systune Server"<<std::endl;
    }

    // This handle Value can be used for Future Untune / Retune Requests.
    // Not the Memory allocations made for Resource and Resource List will be freed
    // automatically by the Client Library, and should not be done by the Client itself.
}

int32_t main(int32_t argc, char* argv[]) {
    func1();
}
