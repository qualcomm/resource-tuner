\page config_files Configurations via YAML files

# Config Files Format
Resource-tuner utilises YAML files for configuration. This includes the resources, signal config files. Target can provide their own config files, which are specific to their use-case through the extension interface

## 1. Initialization Configs
Initialisation configs are mentioned in InitConfig.yaml file. This config enables resource-tuner to setup the required settings at the time of initialisation before any request processing happens.

### Common Initialization Configs
Common initialization configs are defined in /etc/urm/common/InitConfig.yaml

### Overriding Initialization Configs
Targets can override initialization configs (in addition to common init configs, i.e. overrides specific configs) by simply pushing its own InitConfig.yaml into /etc/urm/custom/InitConfig.yaml

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
Common resource configs are defined in /etc/urm/common/ResourcesConfig.yaml.

### Overriding Resource Configs
Targets can override resource cofigs (can fully override or selective resources) by simply pushing its own ResourcesConfig.yaml into /etc/urm/custom/ResourcesConfig.yaml

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
Common resource configs are defined in /etc/urm/common/PropertiesConfig.yaml.

### Overriding Properties Configs
Targets can override Properties cofigs (can fully override or selective resources) by simply pushing its own PropertiesConfig.yaml into /etc/urm/custom/PropertiesConfig.yaml

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
file need not necessarily be provided. uRM can dynamically fetch system info, like target name,
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
