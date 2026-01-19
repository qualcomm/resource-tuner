\page apis URM Api's Description

# APIs
This API suite allows you to manage system resource provisioning through tuning requests. You can issue, modify, or withdraw resource tuning requests with specified durations and priorities.

## tuneResources

**Description:**
Issues a resource provisioning (or tuning) request for a finite or infinite duration.

**API Signature:**
```cpp
int64_t tuneResources(int64_t duration,
                      int32_t prop,
                      int32_t numRes,
                      SysResource* resourceList);

```

**Parameters:**

- `duration` (`int64_t`): Duration in milliseconds for which the Resource(s) should be Provisioned. Use `-1` for an infinite duration.

- `properties` (`int32_t`): Properties of the Request.
                            - The last 8 bits [25 - 32] store the Request Priority (HIGH / LOW)
                            - The Next 8 bits [17 - 24] represent a Boolean Flag, which indicates
                              if the Request should be processed in the background (in case of Display Off or Doze Mode).

- `numRes` (`int32_t`): Number of resources to be tuned as part of the Request.

- `resourceList` (`SysResource*`): List of Resources to be provisioned as part of the Request.

**Returns:**
`int64_t`
- **A positive unique handle** identifying the issued request (used for future `retune` or `untune` operations)
- `-1` otherwise.

---
<div style="page-break-after: always;"></div>

## retuneResources

**Description:**
Modifies the duration of an existing tune request.

**API Signature:**
```cpp
int8_t retuneResources(int64_t handle,
                       int64_t duration);
```

**Parameters:**

- `handle` (`int64_t`): Handle of the original request, returned by the call to `tuneResources`.
- `duration` (`int64_t`): New duration in milliseconds. Use `-1` for an infinite duration.

**Returns:**
`int8_t`
- `0` if the request was successfully submitted.
- `-1` otherwise.

---

<div style="page-break-after: always;"></div>

## untuneResources

**Description:**
Withdraws a previously issued resource provisioning (or tune) request.

**API Signature:**
```cpp
int8_t untuneResources(int64_t handle);
```

**Parameters:**

- `handle` (`int64_t`): Handle of the original request, returned by the call to `tuneResources`.

**Returns:**
`int8_t`
- `0` if the request was successfully submitted.
- `-1` otherwise.

---
<div style="page-break-after: always;"></div>

## tuneSignal

**Description:**
Tune the signal with the given ID.

**API Signature:**
```cpp
int64_t tuneSignal(uint32_t signalCode,
                   int64_t duration,
                   int32_t properties,
                   const char* appName,
                   const char* scenario,
                   int32_t numArgs,
                   uint32_t* list);
```

**Parameters:**

- `signalCode` (`uint32_t`): A uniqued 32-bit (unsigned) identifier for the Signal
                              - The last 16 bits (17-32) are used to specify the SigID
                              - The next 8 bits (9-16) are used to specify the Signal Category
                              - In addition for Custom Signals, the MSB must be set to 1 as well
- `duration` (`int64_t`): Duration (in milliseconds) to tune the Signal for. A value of -1 denotes infinite duration.
- `properties` (`int32_t`): Properties of the Request.
                            - The last 8 bits [25 - 32] store the Request Priority (HIGH / LOW)
                            - The Next 8 bits [17 - 24] represent a Boolean Flag, which indicates
                              if the Request should be processed in the background (in case of Display Off or Doze Mode).
- `appName` (`const char*`): Name of the Application that is issuing the Request
- `scenario` (`const char*`): Use-Case Scenario
- `numArgs` (`int32_t`): Number of Additional Arguments to be passed as part of the Request
- `list` (`uint32_t*`): List of Additional Arguments to be passed as part of the Request

**Returns:**
`int64_t`
- A Positive Unique Handle to identify the issued Request. The handle is used for freeing the Provisioned signal later.
- `-1`: If the Request could not be sent to the server.

---
<div style="page-break-after: always;"></div>

## untuneSignal

**Description:**
Release (or free) the signal with the given handle.

**API Signature:**
```cpp
int8_t untuneSignal(int64_t handle);
```

**Parameters:**

- `handle` (`int64_t`): Request Handle, returned by the tuneSignal API call.

**Returns:**
`int8_t`
- `0`: If the Request was successfully sent to the server.
- `-1`: Otherwise

---
<div style="page-break-after: always;"></div>

## relaySignal

**Description:**
Tune the signal with the given ID.

**API Signature:**
```cpp
int64_t relaySignal(uint32_t signalCode,
                    int64_t duration,
                    int32_t properties,
                    const char* appName,
                    const char* scenario,
                    int32_t numArgs,
                    uint32_t* list);
```

**Parameters:**

- `signalCode` (`uint32_t`): A uniqued 32-bit (unsigned) identifier for the Signal
                              - The last 16 bits (17-32) are used to specify the SigID
                              - The next 8 bits (9-16) are used to specify the Signal Category
                              - In addition for Custom Signals, the MSB must be set to 1 as well
- `duration` (`int64_t`): Duration (in milliseconds)
- `properties` (`int32_t`): Properties of the Request.
- `appName` (`const char*`): Name of the Application that is issuing the Request
- `scenario` (`const char*`): Name of the Scenario that is issuing the Request
- `numArgs` (`int32_t`): Number of Additional Arguments to be passed as part of the Request
- `numArgs` (`uint32_t*`): List of Additional Arguments to be passed as part of the Request

**Returns:**
`int8_t`
- `0`: If the Request was successfully sent to the server.
- `-1`: Otherwise

---
<div style="page-break-after: always;"></div>

## getProp

**Description:**
Gets a property from the config file, this is used as property config file used for enabling or disabling internal features in uRM, can also be used by modules or clients to enable/disable features in their software based on property configs in uRM.

**API Signature:**
```cpp
int8_t getProp(const char* prop,
               char* buffer,
               size_t buffer_size,
               const char* def_value);
```

**Parameters:**

- `prop` (`const char*`): Name of the Property to be fetched.
- `buffer` (`char*`): Pointer to a buffer to hold the result, i.e. the property value corresponding to the specified name.
- `buffer_size` (`size_t`): Size of the buffer.
- `def_value` (`const char*`): Value to be written to the buffer in case a property with the specified Name is not found in the Config Store

**Returns:**
`int8_t`
- `0` If the Property was found in the store, and successfully fetched
- `-1` otherwise.

---

<div style="page-break-after: always;"></div>

# Example Usage of Resource Tuner APIs

## tuneResources

Note the following code snippets showcase the use of resource-tuner APIs. For more in-depth examples
refer "link to examples dir"

This example demonstrates the use of tuneResources API for resource provisioning.
```cpp
#include <iostream>
#include <ResourceTuner/ResourceTunerAPIs.h>

void sendRequest() {
    // Define resources
    SysResource* resourceList = new SysResource[1];
    resourceList[0].mResCode = 65536;
    resourceList[0].mNumValues = 1;
    resourceList[0].mResValue.value = 980;

    // Issue the Tune Request
    int64_t handle = tuneResources(5000, 0, 1, resourceList);

    if(handle < 0) {
        std::cerr<<"Failed to issue tuning request."<<std::endl;
    } else {
        std::cout<<"Tuning request issued. Handle: "<<handle<<std::endl;
    }
}
```

The memory allocated for the resourceList will be freed by the tuneResources API. The user of
this API should not free this memory.

<div style="page-break-after: always;"></div>

## retuneResources

The below example demonstrates the use of the retuneResources API for modifying a request's duration.
```cpp
void sendRequest() {
    // Modify the duration of a previously issued Tune Request to 20 seconds
    // Let's say we stored the handle returned by the tuneResources API in
    // a variable called "handle". Then the retuneResources API can be simply called like:
    if(retuneResources(20000, handle) < 0) {
        std::cerr<<"Failed to Send retune request to Resource Tuner Server"<<std::endl;
    }
}
```

<div style="page-break-after: always;"></div>

## untuneResources

The below example demonstrates the use of the untuneResources API for untuning a previously issued tune Request.
```cpp
void sendRequest() {
    // Withdraw a Previously issued tuning request
    if(untuneResources(handle) == -1) {
        std::cerr<<"Failed to Send untune request to Resource Tuner Server"<<std::endl;
    }
}
```

---

<div style="page-break-after: always;"></div>
