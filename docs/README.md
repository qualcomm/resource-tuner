# Resource Tuner: A System Resource Provisioning Framework

## Table of Contents

- [Introduction](#introduction)
- [Resource Tuner Key Points](#resource-tuner-key-points)
- [Resource Tuner APIs](#resource-tuner-apis)
- [Config Files Format](#config-files-format)
- [Resource Format](#resource-format)
- [Example Usage](#example-usage-of-resource-tuner-apis)
- [Extension Interface](#extension-interface)
- [Resource Tuner Features](#resource-tuner-features)
- [Client CLI](#client-cli)

<div style="page-break-after: always;"></div>

# Introduction

Resource-tuner is a lightweight framework that helps to dynamically provision system resources like CPU, memory, Gpu, I/O, etc for user-space processes. It leverages kernel interfaces like procfs, sysfs and cgroups to infleunce the resource usage to ensure power and performance of applications or usecases in embedded and resource-constrained environments.

Gaining control over system resources such as the CPU, caches, and GPU is a powerful capability in any developer’s toolkit. By fine-tuning these components, developers can optimize the system’s operating point to make more efficient use of hardware resources and significantly enhance the user experience while saving power.

For example, increasing the CPU's dynamic clock and voltage scaling (DCVS) minimum frequency to 1 GHz can boost performance during demanding tasks. Conversely, capping the maximum frequency at 1.5 GHz can help conserve power during less intensive operations.

Resource-tuner framework supports `Signals` which is dynamic provisioning of system resources in response to specific signals —such as app launches or app installations —based on configurations defined in YAML. It allows other software modules or applications to register extensions and add custom functionality tailored to their specific needs.

---

<div style="page-break-after: always;"></div>

## Getting Started

To get started with the project:
[Build and install](../README.md#build-and-install-instructions)

Refer the Examples Tab for guidance on resource-tuner API usage.

---

## Flexible Packaging: Packaging required modules
- Core -> Core module which contains server, client, framwork and helper libraries.
- Signals -> Contains support for provisioning system for the recieved signal
- Tests -> Unit tests and module level tests
- CLI -> Command Line Interface to interact with service for debug and development purpose.

Resource-tuner offers flexibility to select modules through the build system at compile time to make it suitable for devices which have stringent memory requirements. While Tests and Cli are debug and develoment modules which can be removed in the final product config. However Core module is mandatory.

Alter options in corresponding build file like below (ex. cmake options)
```cmake
option(BUILD_TESTS "Testing" OFF)
```

---

<div style="page-break-after: always;"></div>

## Project Structure

```text
/
├── Core
│   ├── Framework                # Core Resource Provisioning Request Flow Logic
│   ├── Modula                   # Common Utilities and Components used across Resource Tuner Modules
│   ├── Client                   # Exposes the Client Facing APIs, and Defines the Client Communication Endpoint
│   ├── Configs                  # Resources Config, Properties Config, Init Config
│   └── Server                   # Defines the Server Communication Endpoint and other Common Server-Side Utils
├── Signals                      # Optional Module, exposes Signal Tuning / Relay APIs
│   └── Configs                  # Signal Configs, Ext Feature Configs
├── Tests                        # Unit and System Wide Tests
└── docs                         # Documentation
```

---

<div style="page-break-after: always;"></div>

# Resource-tuner Key Points
- Resource-tuner exposes a variery of APIs for resource provisioning. These APIs can be used by apps, features and other modules.
- Using these APIs, client can tune any system resource parameters like cpu, dcvs, min / max frequencies etc.
- A client library is provided, which takes care of encoding and sending the request message across to the server for further processing.
- Set of Yaml config files provides extensive configuration capability
- Tuning resources provides control over system resources like CPU, Caches, GPU, etc for. Example changing the operating point of CPU DCVS min-freq to 1GHz to improve performance or limiting its max frequency to 1.5GHz to save power
- Tuning Signals dynamically provisions the system resources for a use case or scenario such as apps launches, installations, etc. in response to received signal. Resources can be configured in yaml for signals.
- Signals pick resources related to signal from SignalsConfig.yaml
- Extension interface provides a way to customize resource-tuner behaviour, by specifying custom resources, custom signals and features.
- Resource-tuner uses YAML based config files, for fetching information relating to resources/signals and properties.

---

<div style="page-break-after: always;"></div>

# Resource Tuner APIs
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

## getProp

**Description:**
Gets a property from the config store

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

# Config Files Format
Resource-tuner utilises YAML files for configuration. This includes the resources, signal config files. Target can provide their own config files, which are specific to their use-case through the extension interface

## 1. Initialization Configs
Initialisation configs are mentioned in InitConfig.yaml file. This config enables resource-tuner to setup the required settings at the time of initialisation before any request processing happens.

### Common Initialization Configs
Common initialization configs are defined in /etc/resource-tuner/common/InitConfig.yaml

### Overriding Initialization Configs
Targets can override initialization configs (in addition to common init configs, i.e. overrides specific configs) by simply pushing its own InitConfig.yaml into /etc/resource-tuner/custom/InitConfig.yaml

### Overiding with Custom Extension File
RESTUNE_REGISTER_CONFIG(INIT_CONFIG, "/bin/InitConfigCustom.yaml");

### 1. Logical Cluster Map
Configs of cluster map in InitConfigs->ClusterMap section
| LgcId  |     Name   |
|--------|------------|
|   0    |   "little" |
|   1    |   "big"    |
|   2    |   "prime"  |

### 2. Cgroups map
Configs of cgroups map in InitConfigs->CgroupsInfo section
| Lgc Cgrp No | Cgrp Name  |
|-------------|------------|
|      0      |  "default" |
|      1      |  "bg-app"  |
|      2      |  "top-app" |
|      3      |"camera-app"|

### 3. Mpam Groups Map
Configs of mpam grp map in InitConfigs->MpamGroupsInfo section

| Num Cache Blocks  |    Cache Type  | Prio Aware|
|-------------------|----------------|-----------|
|         2         |      "L2"      |     0     |
|         1         |      "L3"      |     1     |

| LgcId  |    Mpam grp Name  | prio |
|--------|-------------------|------|
|   0    |      "default"    |   0  |
|   1    |       "video"     |   1  |
|   2    |       "camera"    |   2  |

<div style="page-break-after: always;"></div>

#### Fields Description for CGroup Config

| Field           | Type       | Description | Default Value |
|----------------|------------|-------------|-----------------|
| `ID`        | `int32_t` (Mandatory)   | A 32-bit unique identifier for the CGroup | Not Applicable |
| `Create`       | `boolean` (Optional)  | Boolean flag indicating if the CGroup needs to be created by the resource-tuner server | False |
| `IsThreaded`       | `boolean` (Optional)  | Boolean flag indicating if the CGroup is threaded | False |
| `Name`          | `string` (Optional)   | Descriptive name for the CGroup | `Empty String` |

<div style="page-break-after: always;"></div>

#### Fields Description for Mpam Group Config

| Field           | Type       | Description | Default Value |
|----------------|------------|-------------|-----------------|
| `ID`        | `int32_t` (Mandatory)   | A 32-bit unique identifier for the Mpam Group | Not Applicable |
| `Priority`       | `int32_t` (Optional)  | Mpam Group Priority | 0 |
| `Name`          | `string` (Optional)   | Descriptive name for the Mpam Group | `Empty String` |

<div style="page-break-after: always;"></div>

#### Fields Description for Cache Info Config

| Field           | Type       | Description | Default Value |
|----------------|------------|-------------|-----------------|
| `Type`        | `string` (Mandatory)   | Type of cache (L2 or L3) for which config is intended | Not Applicable |
| `NumCacheBlocks`  | `int32_t` (Mandatory)  | Number of Cache blocks for the above mentioned type, to be managed by resource-tuner | Not Applicable |
| `PriorityAware`          | `boolean` (Optional)   | Boolean flag indicating if the Cache Type supports different Priority Levels. | `false` |

<div style="page-break-after: always;"></div>

#### Example

```yaml
InitConfigs:
  # Logical IDs should always be arranged from lower to higher cluster capacities
  - ClusterMap:
    - Id: 0
      Type: little
    - Id: 1
      Type: big
    - Id: 2
      Type: prime

  - CgroupsInfo:
    - Name: "camera-cgroup"
      Create: true
      ID: 0
    - Name: "audio-cgroup"
      Create: true
      ID: 1
    - Name: "video-cgroup"
      Create: true
      IsThreaded: true
      ID: 2

  - MPAMgroupsInfo:
    - Name: "camera-mpam-group"
      ID: 0
      Priority: 0
    - Name: "audio-mpam-group"
      ID: 1
      Priority: 1
    - Name: "video-mpam-group"
      ID: 2
      Priority: 2

  - CacheInfo:
    - Type: L2
      NumCacheBlocks: 2
      PriorityAware: 0
    - Type: L3
      NumCacheBlocks: 1
      PriorityAware: 1
```

---
<div style="page-break-after: always;"></div>

## 2. Resource Configs
Tunable resources are specified via ResourcesConfig.yaml file.

### Common Resource Configs
Common resource configs are defined in /etc/resource-tuner/common/ResourcesConfig.yaml.

### Overriding Resource Configs
Targets can override resource cofigs (can fully override or selective resources) by simply pushing its own ResourcesConfig.yaml into /etc/resource-tuner/custom/ResourcesConfig.yaml

### Overiding with Custom Extension File
RESTUNE_REGISTER_CONFIG(RESOURCE_CONFIG, "/bin/targetResourceConfigCustom.yaml");

Each resource is defined with the following fields:

#### Fields Description

| Field           | Type       | Description | Default Value |
|----------------|------------|-------------|-----------------|
| `ResID`        | `Integer` (Mandatory)   | unsigned 16-bit Resource Identifier, unique within the Resource Type. | Not Applicable |
| `ResType`       | `Integer` (Mandatory)  | unsigned 8-bit integer, indicating the Type of the Resource, for example: cpu / dcvs | Not Applicable |
| `Name`          | `string` (Optional)   | Descriptive name | `Empty String` |
| `Path`          | `string` (Optional)   | Full resource path of sysfs or procfs file path (if applicable). | `Empty String` |
| `Supported`     | `boolean` (Optional)  | Indicates if the Resource is Eligible for Provisioning. | `False` |
| `HighThreshold` | `integer (int32_t)` (Mandatory)   | Upper threshold value for the resource. | Not Applicable |
| `LowThreshold`  | `integer (int32_t)` (Mandatory)   | Lower threshold value for the resource. | Not Applicable |
| `Permissions`   | `string` (Optional)   | Type of client allowed to Provision this Resource (`system` or `third_party`). | `third_party` |
| `Modes`         | `array` (Optional)    | Display modes applicable (`"display_on"`, `"display_off"`, `"doze"`). | 0 (i.e. not supported in any Mode) |
| `Policy`        | `string`(Optional)   | Concurrency policy (`"higher_is_better"`, `"lower_is_better"`, `"instant_apply"`, `"lazy_apply"`). | `lazy_apply` |
| `Unit`        | `string`(Optional)   | Translation Unit (`"MB"`, `"GB"`, `"KHz"`, `"Hz"` etc). | `NA (multiplier = 1)` |
| `ApplyType` | `string` (Optional)  | Indicates if the resource can have different values, across different cores, clusters or cgroups. | `global` |
| `TargetsEnabled`          | `array` (Optional)   | List of Targets on which this Resource should be available for tuning | `Empty List` |
| `TargetsDisabled`          | `array` (Optional)   | List of Targets on which this Resource should not be available for tuning | `Empty List` |

<div style="page-break-after: always;"></div>

#### Example

```yaml
ResourceConfigs:
  - ResType: "0x1"
    ResID: "0x0"
    Name: "RESTUNE_SCHED_UTIL_CLAMP_MIN"
    Path: "/proc/sys/kernel/sched_util_clamp_min"
    Supported: true
    HighThreshold: 1024
    LowThreshold: 0
    Permissions: "third_party"
    Modes: ["display_on", "doze"]
    Policy: "higher_is_better"

  - ResType: "0x1"
    ResID: "0x1"
    Name: "RESTUNE_SCHED_UTIL_CLAMP_MAX"
    Path: "/proc/sys/kernel/sched_util_clamp_max"
    Supported: true
    HighThreshold: 1024
    LowThreshold: 0
    Permissions: "third_party"
    Modes: ["display_on", "doze"]
    Policy: "lower_is_better"
```

---
<div style="page-break-after: always;"></div>

## 3. Properties Config
PropertiesConfig.yaml file stores various properties which are used by resource-tuner modules internally. For example, to allocate sufficient amount of memory for different types, or to determine the Pulse Monitor duration. Client can also use this as a property store to store their properties which gives it flexibility to control properties depending on the target.

### Common Properties Configs
Common resource configs are defined in /etc/resource-tuner/common/PropertiesConfig.yaml.

### Overriding Properties Configs
Targets can override Properties cofigs (can fully override or selective resources) by simply pushing its own PropertiesConfig.yaml into /etc/resource-tuner/custom/PropertiesConfig.yaml

### Overiding with Custom Properties File
RESTUNE_REGISTER_CONFIG(PROPERTIES_CONFIG, "/bin/targetPropertiesConfigCustom.yaml"); if Client have no specific extensions like custom resources or features only want to change the config then the above method (using the same file name and pushing it to custom folder) is the best method to go for.

#### Field Descriptions

| Field          | Type       | Description | Default Value  |
|----------------|------------|-------------|----------------|
| `Name`         | `string` (Mandatory)   | Unique name of the property | Not Applicable
| `Value`        | `integer` (Mandatory)   | The value for the property. | Not Applicable


#### Example

```yaml
PropertyConfigs:
  - Name: resource_tuner.maximum.concurrent.requests
    Value: "60"
  - Name: resource_tuner.maximum.resources.per.request
    Value: "64"
  - Name: resource_tuner.pulse.duration
    Value: "60000"
```
<div style="page-break-after: always;"></div>

## 4. Signal Configs
The file SignalsConfig.yaml defines the signal configs.

#### Field Descriptions

| Field           | Type       | Description | Default Value |
|----------------|------------|-------------|---------------|
| `SigId`          | `Integer` (Mandatory)   | 16 bit unsigned Signal Identifier, unique within the signal category | Not Applicable |
| `Category`          | `Integer` (Mandatory)   | 8 bit unsigned integer, indicating the Category of the Signal, for example: Generic, App Lifecycle. | Not Applicable |
| `Name`          | `string` (Optional)  | |`Empty String` |
| `Enable`          | `boolean` (Optional)   | Indicates if the Signal is Eligible for Provisioning. | `False` |
| `TargetsEnabled`          | `array` (Optional)   | List of Targets on which this Signal can be Tuned | `Empty List` |
| `TargetsDisabled`          | `array` (Optional)   | List of Targets on which this Signal cannot be Tuned | `Empty List` |
| `Permissions`          | `array` (Optional)   | List of acceptable Client Level Permissions for tuning this Signal | `third_party` |
|`Timeout`              | `integer` (Optional) | Default Signal Tuning Duration to be used in case the Client specifies a value of 0 for duration in the tuneSignal API call. | `1 (ms)` |
| `Resources` | `array` (Mandatory) | List of Resources. | Not Applicable |

<div style="page-break-after: always;"></div>

#### Example

```yaml
SignalConfigs:
  - SigId: "0x0"
    Category: "0x1"
    Name: INSTALL
    Enable: true
    TargetsEnabled: ["sun", "moon"]
    Permissions: ["system", "third_party"]
    Derivatives: ["solar"]
    Timeout: 4000
    Resources:
      - {ResId: "0x0", ResType: "0x1", OpInfo: 0, Values: [700]}

  - SigId: "0x1"
    Category: "0x1"
    Name: EARLY_WAKEUP
    Enable: true
    TargetsDisabled: ["sun"]
    Permissions: ["system"]
    Derivatives: ["solar"]
    Timeout: 5000
    Resources:
      - {ResId: "0", ResType: "0x1", OpInfo: 0, Values: [300, 400]}
      - {ResId: "1", ResType: "0x1", OpInfo: 1024, Values: [12, 45]}
      - {ResId: "2", ResType: "0x2", OpInfo: 32, Values: [5]}
      - {ResId: "3", ResType: "0x4", OpInfo: 256, Values: [23, 90]}
      - {ResId: "4", ResType: "0x1", OpInfo: 512, Values: [87]}
```
<div style="page-break-after: always;"></div>

## 5. (Optional) Target Configs
The file TargetConfig.yaml defines the target configs, note this is an optional config, i.e. this
file need not necessarily be provided. Resource-tuner can dynamically fetch system info, like target name,
logical to physical core / cluster mapping, number of cores etc. Use this file, if you want to
provide this information explicitly. If the TargetConfig.yaml is provided, resource-tuner will always
overide default dynamically generated target information and use it. Also note, there are no field-level default values available if the TargetConfig.yaml is provided. Hence if you wish to provide this file, then you'll need to provide all the complete required information.

#### Field Descriptions

| Field           | Type       | Description | Default Value |
|----------------|------------|-------------|---------------|
| `TargetName`          | `string` | Target Identifier | Not Applicable |
| `ClusterInfo`          | `array` | Cluster ID to Type Mapping | Not Applicable |
| `ClusterSpread`          | `array` |  Cluster ID to Per Cluster Core Count Mapping | Not Applicable |

<div style="page-break-after: always;"></div>

#### Example

```yaml
TargetConfig:
  - TargetName: ["QCS9100"]
    ClusterInfo:
      - LgcId: 0
        PhyId: 4
      - LgcId: 1
        PhyId: 0
      - LgcId: 2
        PhyId: 9
      - LgcId: 3
        PhyId: 7
    ClusterSpread:
      - PhyId: 0
        NumCores: 4
      - PhyId: 4
        NumCores: 3
      - PhyId: 7
        NumCores: 2
      - PhyId: 9
        NumCores: 1
```
<div style="page-break-after: always;"></div>

## 6. (Optional) ExtFeatures Configs
The file ExtFeaturesConfig.yaml defines the Extension Features, note this is an optional config, i.e. this
file need not necessarily be provided. Use this file to specify your own custom features. Each feature is associated with it's own library and an associated list of signals. Whenever a relaySignal API request is received for any of these signals, resource-tuner will forward the request to the corresponding library.
The library is required to implement the following 3 functions:
- initFeature
- tearFeature
- relayFeature
Refer the Examples section for more details on how to use the relaySignal API.

#### Field Descriptions

| Field           | Type       | Description | Default Value |
|----------------|------------|-------------|---------------|
| `FeatId`          | `Integer` | unsigned 32-bit Unique Feature Identifier | Not Applicable |
| `LibPath`         | `string` | Path to the associated library | Not Applicable |
| `Signals`         | `array` |  List of signals to subscribe the feature to | Not Applicable |

<div style="page-break-after: always;"></div>

#### Example

```yaml
FeatureConfigs:
  - FeatId: "0x00000001"
    Name: "FEAT-1"
    LibPath: "rlib2.so"
    Description: "Simple Algorithmic Feature, defined by the BU"
    Signals: ["0x00050000", "0x00050001"]

  - FeatId: "0x00000002"
    Name: "FEAT-2"
    LibPath: "rlib1.so"
    Description: "Simple Observer-Observable Feature, defined by the BU"
    Subscribers: ["0x00050000", "0x00050002"]
```

---

<div style="page-break-after: always;"></div>

# Resource Format

As part of the tuneResources APIs, the resources (which need to be provisioned) are specified by using
a List of `Resource` structures. The format of the `Resource` structure is as follows:

```cpp
typedef struct {
    uint32_t mResCode;
    int32_t mResInfo;
    int32_t mOptionalInfo;
    int32_t mNumValues;

    union {
        int32_t value;
        int32_t* values;
    } mResValue;
} SysResource;
```

**mResCode**: An unsigned 32-bit unique identifier for the resource. It encodes essential information that is useful in abstracting away the system specific details.

**mResInfo**: Encodes operation-specific information such as the Logical cluster and Logical core ID, and MPAM part ID.

**mOptionalInfo**: Additional optional metadata, useful for custom or extended resource configurations.

**mNumValues**: Number of values associated with the resource. If multiple values are needed, this must be set accordingly.

**Value / Values**: It is a single value when the resource requires a single value or a pointer to an array of values for multi-value configurations.

<div style="page-break-after: always;"></div>

## Notes on Resource ResCode

As mentioned above, the resource code is an unsigned 32 bit integer. This section describes how this code can be constructed. Resource-tuner implements a System Independent Layer(SIL) which provides a transparent and consistent way for indexing resources. This makes it easy for the clients to identify the resource they want to provision, without needing to worry about portability issues across targets or about the order in which the resources are defined in the YAML files.

Essentially, the resource code (unsigned 32 bit) is composed of two fields:
- ResID (last 16 bits, 17 - 32)
- ResType (next 8 bits, 9 - 16)
- Additionally MSB should be set to '1' if customer or other modules or target chipset is providing it's own custom resource config files, indicating this is a custom resource else it shall be treated as a default resource. This bit doesn't influence resource processing, just to aid debugging and development.

These fields can uniquely identify a resource across targets, hence making the code operating on these resources interchangable. In essence, we ensure that the resource with code "x", refers to the same tunable resource across different targets.

Examples:
- The ResCode: 65536 (0x00010000) [00000000 00000001 00000000 00000000], Refers to the Default Resource with ResID 0 and ResType 1.
- The ResCode: 2147549185 (0x80010001) [10000000 00000001 00000000 00000001], Refers to the Custom Resource with ResID 1 and ResType 1.

#### List Of Resource Types (Use this table to get the value of ResType for a Resource)

| Name           | ResType  | Examples |
|----------------|----------|----------|
|    LPM       |    `1`   | `/dev/cpu_dma_latency`, `/sys/devices/system/cpu/cpu<>/power/pm_qos_resume_latency_us` |
|    CACHES    |    `2`   | |
|    CPU_SCHED   |    `3`   | `/proc/sys/kernel/sched_util_clamp_min`, `/proc/sys/kernel/sched_util_clamp_max` |
|    CPU_DCVS    |    `4`   | `/sys/devices/system/cpu/cpufreq/policy<>/scaling_min_freq`, `/sys/devices/system/cpu/cpufreq/policy<>/scaling_max_freq` |
|    GPU         |    `5`   | |
|    NPU         |    `6`   | |
|    MEMORY      |    `7`   | |
|    MPAM        |    `8`   | |
| Cgroup         |    `9`   | `/sys/fs/cgroup/%s/cgroup.procs`, `/sys/fs/cgroup/%s/cgroup.threads`, `/sys/fs/cgroup/%s/cpuset.cpus`, `/sys/fs/cgroup/%s/cpuset.cpus.partition`, `/sys/fs/cgroup/%s/cgroup.freeze`, `/sys/fs/cgroup/%s/cpu.max`, `/sys/fs/cgroup/%s/cpu.idle`, `/sys/fs/cgroup/%s/cpu.uclamp.min`, `/sys/fs/cgroup/%s/cpu.uclamp.max`, `/sys/fs/cgroup/%s/cpu.weight`, `/sys/fs/cgroup/%s/memory.max`, `/sys/fs/cgroup/%s/memory.min`, `/sys/fs/cgroup/%s/cpu.weight.nice` |
|    STORAGE      |    `a`   | |

---

<div style="page-break-after: always;"></div>

# Example Usage of Resource Tuner APIs

## tuneResources

Note the following code snippets showcase the use of resource-tuner APIs. For more in-depth examples
refer "link to examples dir"

This example demonstrates the use of tuneResources API for resource provisioning.
```cpp
#include <iostream>
#include <ResourceTuner/UrmAPIs.h>

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

# Extension Interface

The Resource-tuner framework allows target chipsets to extend its functionality and customize it to their use-case. Extension interface essentially provides a series of hooks to the targets or other modules to add their own custom behaviour. This is achieved through a lightweight extension interface. This happens in the initialisation phase before the service is ready for requests.

Specifically the extension interface provides the following capabilities:
- Registering custom resource handlers
- Registering custom configuration files (This includes resource configs, signal configs and property configs). This allows, for example the specification of custom resources.

## Extension APIs

### RESTUNE_REGISTER_APPLIER_CB

Registers a custom resource Applier handler with the system. This allows the framework to invoke a user-defined callback when a tune request for a specific resource opcode is encountered. A function pointer to the callback is to be registered.
Now, instead of the default resource handler (provided by resource-tuner), this callback function will be called when a Resource Provisioning Request for this particular resource opcode arrives.

### Usage Example
```cpp
void applyCustomCpuFreqCustom(void* res) {
    // Custom logic to apply CPU frequency
    return 0;
}

RESTUNE_REGISTER_APPLIER_CB(0x00010001, applyCustomCpuFreqCustom);
```

---

### RESTUNE_REGISTER_TEAR_CB

Registers a custom resource teardown handler with the system. This allows the framework to invoke a user-defined callback when an untune request for a specific resource opcode is encountered. A function pointer to the callback is to be registered.
Now, instead of the normal resource handler (provided by resource-tuner), this callback function will be called when a Resource Deprovisioning Request for this particular resource opcode arrives.

### Usage Example
```cpp
void resetCustomCpuFreqCustom(void* res) {
    // Custom logic to clear currently applied CPU frequency
    return 0;
}

RESTUNE_REGISTER_TEAR_CB(0x00010001, resetCustomCpuFreqCustom);
```

---

### RESTUNE_REGISTER_CONFIG

Registers a custom configuration YAML file. This enables target chipset to provide their own config files, i.e. allowing them to provide their own custom resources for example.

### Usage Example
```cpp
RESTUNE_REGISTER_CONFIG(RESOURCE_CONFIG, "/etc/bin/targetResourceConfigCustom.yaml");
```
The above line of code, will indicate to resource-tuner to read the resource configs from the file
"/etc/bin/targetResourceConfigCustom.yaml" instead of the default file. note, the target chipset must honour the structure of the YAML files, for them to be read and registered successfully.

Custom signal config file can be specified similarly:

### Usage Example
```cpp
RESTUNE_REGISTER_CONFIG(SIGNALS_CONFIG, "/etc/bin/targetSignalConfigCustom.yaml");
```

---
<div style="page-break-after: always;"></div>


# Resource Tuner Features

<img src="design_resource_tuner.png" alt="Resource Tuner Design" width="70%"/>

Resource-tuner architecture is captured above.

## Initialization
- During the server initialization phase, the YAML config files are read to build up the resource registry, property store etc.
- If the target chipset has registered any custom resources, signals or custom YAML files via the extension interface, then these changes are detected during this phase itself to build up a consolidated system view, before it can start serving requests.
- During the initialization phase, memory is pre-allocated for commonly used types (via MemoryPool), and worker (thread) capacity is reserved in advance via the ThreadPool, to avoid any delays during the request processing phase.
- Resource-tuner will also fetch the target details, like target name, total number of cores, logical to physical cluster / core mapping in this phase.
- If the Signals module is plugged in, it will be initialized as well and the signal configs will be parsed similarly to resource configs.
- Once all the initialization is completed, the server is ready to serve requests, a new listener thread is created for handling requests.

<div style="page-break-after: always;"></div>

## Request Processing
- The client can use the resource-tuner client library to send their requests.
- Resource-tuner supports sockets and binders for client-server communication.
- As soon as the request is received on the server end, a handle is generated and returned to the client. This handle uniquely identifies the request and can be used for subsequent retune (retuneResources) or untune (untuneResources) API calls.
- The request is submitted to the ThreadPool for async processing.
- When the request is picked up by a worker thread (from the ThreadPool), it will decode the request message and then validate the request.
- The request verifier, will run a series of checks on the request like permission checks, and on the resources part of the request, like config value bounds check.
- Once request is verified, a duplicate check is performed, to verify if the client has already submitted the same request before. This is done so as to the improve system efficiency and performace.
- Next the request is added to an queue, which is essentially PriorityQueue, which orders requests based on their priorities (for more details on Priority Levels, refer the next Section). This is done so that the request with the highest priority is always served first.
- To handle concurrent requests for the same resource, we maintain resource level linked lists of pending requests, which are ordered according to the request priority and resource policy. This ensures that the request with the higher priority will always be applied first. For two requests with the same priority, the application order will depend on resource policy. For example, in case of resource with "higher is better" policy, the request with a higher configuration value for the resource shall take effect first.
- Once a request reaches the head of the resource level linked list, it is applied, i.e. the config value specified by this request for the resource takes effect on the corresponding sysfs node.
- A timer is created and used to keep track of a request, i.e. check if it has expired. Once it is detected that the request has expired an untune request for the same handle as this request, is automatically generated and submitted, it will take care of resetting the effected resource nodes to their original values.
- Client modules can provide their own custom resource actions for any resource. The default action provided by resource-tuner is writing to the resource sysfs node.

---
<div style="page-break-after: always;"></div>

Here is a more detailed explanation of the key features discussed above:

## 1. Client-Level Permissions
Certain resources can be tuned only by system clients and some which have no such restrictions and can be tuned even by third party clients. The client permissions are dynamically determined, the first time it makes a request. If a client with third party permissions tries to tune a resource, which allows only clients with system permissions to tune it, then the request shall be dropped.

## 2. Resource-Level Policies
To ensure efficient and predictable handling of concurrent requests, each system resource is governed by one of four predefined policies. Selecting the appropriate policy helps maintain system stability, optimize performance/power, and align resource behavior with application requirements.

- Instant Apply: This policy is for resources where the latest request needs to be honored. This is kept as the default policy.
- Higher is better: This policy honors the request writing the highest value to the node. One of the cases where this makes sense is for resources that describe the upper bound value. By applying the higher-valued request, the lower-valued request is implicitly honored.
- Lower is better: Works exactly opposite of the higher is better policy.
- Lazy Apply: Sometimes, you want the resources to apply requests in a first-in-first-out manner.

## 3. Request-Level Priorities
As part of the tuneResources API call, client is allowed to specify a desired priority level for the request. Resource-tuner supports 2 priority levels:
- High
- Low

However when multiplexed with client-level permissions, effetive request level priorities would be
- System High [SH]
- System Low [SL]
- Third Party High (or Regular High) [TPH]
- Third Party Low (or Regular Low) [TPL]

Requests with a higher priority will always be prioritized, over another request with a lower priority. Note, the request priorities are related to the client permissions. A client with system permission is allowed to acquire any priority level it wants, however a client with third party permissions can only acquire either third party high (TPH) or third party low (TPL) level of priorities. If a client with third party permissions tries to acquire a System High or System Low level of priority, then the request will not be honoured.

## 4. Pulse Monitor: Detection of Dead Clients and Subsequent Cleanup
To improve efficiency and conserve memory, it is essential to regularly check for dead clients and free up any system resources associated with them. This includes, untuning all (if any) ongoing tune request issued by this client and freeing up the memory used to store client specific data (Example: client's list of requests (handles), health, permissions, threads associated with the client etc). resource-tuner ensures that such clients are detected and cleaned up within 90 seconds of the client terminated.

Resource-tuner performs these actions by making use of two components:
- Pulse check: scans the list of the active clients, and checks if any of the client (PID) is dead. If it finds a dead client, it schedules the cleanup by adding this PID to a queue.
- Garbage collection: When the thread runs it iterates over the GC queue and performs the cleanup.

Pulse Monitor runs on a seperate thread peroidically.

## 5. Rate Limiter: Preventing System Abuse
Resource-tuner has rate limiter module that prevents abuse of the system by limiting the number of requests a client can make within a given time frame. This helps to prevent clients from overwhelming the system with requests and ensures that the system remains responsive and efficient. Rate limiter works on a reward/punishment methodology. Whenever a client requests the system for the first time, it is assigned a "Health" of 100. A punishment is handed over if a client makes subsequent new requests in a very short time interval (called delta, say 2 ms).
A Reward results in increasing the health of a client (not above 100), while a punishment involves decreasing the health of the client. If at any point this value of Health reaches zero then any further requests from this client wil be dropped. Value of delta, punishment and rewards are target-configurable.

## 6. Duplicate Checking
Resource-tuner's RequestManager component is responsible for detecting any duplicate requests issued by a client, and dropping them. This is done by checking against a list of all the requests issued by a clientto identify a duplicate. If it is, then the request is dropped. If it is not, then the request is added and processed. Duplicate checking helps to improve system efficiency, by saving wasteful CPU time on processing duplicates.

## 7. Dynamic Mapper: Logical to Physical Mapping
Logical to physical core/cluster mapping helps to achieve application code portability across different chipsets on client side. Client can specify logical values for core and cluster. Resource-tuner will translate these values to their physical counterparts and apply the request accordingly. Logical to physical mapping helps to create system independent layer and helps to make the same client code interchangable across different targets.

Logical mapping entries can be found in InitConfig.yaml and can be modified if required.

Logical layer values always arranged from lower to higher cluster capacities.
If no names assigned to entries in the dynamic mapping table then cluster'number' will be the name of the cluster
for ex. LgcId 4 named as "cluster4"

below table present in InitConfigs->ClusterMap section
| LgcId  |     Name   |
|--------|------------|
|   0    |   "little" |
|   1    |   "big"    |
|   2    |    "prime" |

resource-tuner reads machine topology and prepares logical to physical table dynamically in the init phase, similar to below one
| LgcId  |  PhyId  |
|--------|---------|
|   0    |     0   |
|   1    |     1   |
|   2    |     3   |
|   3    |     2   |


## 8. Display-Aware Operational Modes
The system's operational modes are influenced by the state of the device's display. To conserve power, certain system resources are optimized only when the display is active. However, for critical components that require consistent performance—such as during background processing or time-sensitive tasks, resource tuning can still be applied even when the display is off, including during low-power states like doze mode. This ensures that essential operations maintain responsiveness without compromising overall energy efficiency.

## 9. Crash Recovery
In case of server crash, resource-tuner ensures that all the resource sysfs nodes are restored to a sane state, i.e. they are reset to their original values. This is done by maintaining a backup of all the resource's original values, before any modification was made on behalf of the clients by resource tuner. In the event of server crash, reset to their original values in the backup.

## 10. Flexible Packaging
The Users are free to pick and choose the resource-tuner modules they want for their use-case and which fit their constraints. The Framework Module is the core/central module, however if the users choose they can add on top of it other Modules: signals and profiles.

## 11. Pre-Allocate Capacity for efficiency
Resource Tuner provides a MemoryPool component, which allows for pre-allocation of memory for certain commonly used type at the time of server initialization. This is done to improve the efficiency of the system, by reducing the number of memory allocations and deallocations that are required during the processing of requests. The allocated memory is managed as a series of blocks which can be recycled without any system call overhead. This reduces the overhead of memory allocation and deallocation, and improves the performance of the system.

Further, a ThreadPool component is provided to pre-allocate processing capacity. This is done to improve the efficiency of the system, by reducing the number of thread creation and destruction required during the processing of Requests, further ThreadPool allows for the threads to be repeatedly reused for processing different tasks.

---
<div style="page-break-after: always;"></div>

# Client CLI
Resource-tuner provides a minimal CLI to interact with the server. This is provided to help with development and debugging purposes.

## Usage Examples

### 1. Send a Tune Request
```bash
./resource_tuner_cli --tune --duration <> --priority <> --num <> --res <>
```
Where:
- `duration`: Duration in milliseconds for the tune request
- `priority`: Priority level for the tune request (HIGH: 0 or LOW: 1)
- `num`: Number of resources
- `res`: List of resource ResCode, ResInfo (optional) and Values to be tuned as part of this request

Example:
```bash
# Single Resource in a Request
./resource_tuner_cli --tune --duration 5000 --priority 0 --num 1 --res "65536:700"

# Multiple Resources in single Request
./resource_tuner_cli --tune --duration 4400 --priority 1 --num 2 --res "0x80030000:700,0x80040001:155667"

# Multi-Valued Resource
./resource_tuner_cli --tune --duration 9500 --priority 0 --num 1 --res "0x00090002:0,0,1,3,5"

# Specifying ResInfo (useful for Core and Cluster type Resources)
./resource_tuner_cli --tune --duration 5000 --priority 0 --num 1 --res "0x00040000#0x00000100:1620438"

# Everything at once
./resource_tuner_cli --tune --duration 6500 --priority 0 --num 2 --res "0x00030000:800;0x00040011#0x00000101:50000,100000"
```

### 2. Send an Untune Request
```bash
./resource_tuner_cli --untune --handle <>
```
Where:
- `handle`: Handle of the previously issued tune request, which needs to be untuned

Example:
```bash
./resource_tuner_cli --untune --handle 50
```

### 3. Send a Retune Request
```bash
./resource_tuner_cli --retune --handle <> --duration <>
```
Where:
- `handle`: Handle of the previously issued tune request, which needs to be retuned
- `duration`: The new duration in milliseconds for the tune request

Example:
```bash
./resource_tuner_cli --retune --handle 7 --duration 8000
```

### 4. Send a getProp Request

```bash
./resource_tuner_cli --getProp --key <>
```
Where:
- `key`: The Prop name of which the corresponding value needs to be fetched

Example:
```bash
./resource_tuner_cli --getProp --key "resource_tuner.logging.level"
```

<div style="page-break-after: always;"></div>

# Contact

For questions, suggestions, or contributions, feel free to reach out:

- **Email**: maintainers.resource-tuner-moderator@qti.qualcomm.com

# License

This project is licensed under the BSD 3-Clause Clear License.

<div style="page-break-after: always;"></div>
