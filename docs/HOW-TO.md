\page usage General Usage Guide

# 1. How to add a custom resource
Resource tuner is configured with a default set of [Resources](../Core/Configs/ResourcesConfig.yaml),
called the Common Resources. These resources are available at at /etc/resource-tuner/common/ResourcesConfig.yaml.
On top of it, resource-tuner allows the addition of Custom Resources.
To add Custom Resources, developers can follow one of the following two strategies.
1. Add the Custom ResourcesConfig.yaml at /etc/resource-tuner/custom. Note the file name must exactly match "ResourcesConfig.yaml". As part of initialization, resource-tuner will check if this file is present, if it is, it will be parsed alongside the Common Resources.

2. The custom Resources file can also be placed in a different location other than /etc/resource-tuner/custom. This can be done through Resource Tuner's Extension Interface. For example if the file is present at /opt/custom/ResourcesConfig.yaml, then the RESTUNE_REGISTER_CONFIG macro can be used as follows:

```cpp
RESTUNE_REGISTER_CONFIG(RESOURCE_CONFIG, "/opt/custom/ResourcesConfig.yaml")
```
This will indicate to resource-tuner, that Custom Resources are present at the above specified location so that they can be parsed as part of initialization.

Note:\n
1) If Custom Resources are provided, then the Common and Custom Resources shall be merged together in a unified tunable Resources dataset. If a resource specified in the Custom Resource Config has the exact same ResType and ResID as one in Common Resource Config, then the latter shall be overridden.

2) The Custom Resource Config file must have exactly the same structure as the Common Resource Config file, refer the following example on how to structure the Config files:

## Sample Resource config file with two resources.
```yaml
ResourceConfigs:
  - ResType: "0x03"
    ResID: "0x0000"
    Name: "SCHED_UTIL_CLAMP_MIN"
    Path: "/proc/sys/kernel/sched_util_clamp_min"
    Supported: true
    HighThreshold: 1024
    LowThreshold: 0
    Permissions: "third_party"
    Modes: ["display_on", "doze"]
    Policy: "lower_is_better"
    ApplyType: "global"

  - ResType: "0x03"
    ResID: "0x0001"
    Name: "SCHED_UTIL_CLAMP_MAX"
    Path: "/proc/sys/kernel/sched_util_clamp_max"
    Supported: true
    HighThreshold: 1024
    LowThreshold: 0
    Permissions: third_party
    Modes: ["display_on", "doze"]
    Policy: higher_is_better
    ApplyType: "global"
```

## Notes on Resource Indexing
Resource Tuner implements a System Independent Layer to make Resource Indexing consistent. Each Resource is uniquely identified by a unsigned 32 bit integer, called ResCode.
This integer is a combination of:
- ResID (last 16 bits, 17 - 32)
- ResType (next 8 bits, 9 - 16)
- Customer Specific Bit (1 bit, 31, i.e. MSB)

The Customer Specific Bit needs to be set to "1" in case of Custom Resources.
For example, if a custom resource has been specified with the ResID: "0x00dd" and ResType: "0x9f", then the resultant ResCode will be: 0x809f00dd
Notice the MSB has been set to 1.

---

# 2. How to override the Resource tune and untune functionalities.
Using Resource Tuner's Extension interface, the developer can modify the default Resource tune and untune functionalities. Resource tuner provides the default tune and untune functions. However, developer can customize these behaviours.
Refer: Plugin.cpp in Examples Tab for usage guidance

---

# 3. How to add init configs
Resource tuner supports Cgroups as well, InitConfig.yaml file is used to specify the Cgroups which need to created during resource-tuner initialization.
Developer can supply additional init configs via /etc/resource-tuner/custom/InitConfig.yaml
## Sample Cgroup config
```yaml
InitConfigs:
  - CgroupsInfo:
    - Name: "camera-cgroup"
      ID: 0
```

---

# 4. How to enforce CPU architecture details
CPU architecture refers to
* No of clusters in the system
* No of cores in the each cluster
* Logical Cluster ID to physical Cluster ID mapping

Resource Tuner detects the CPU architecture based on cpu_capacity.
Developer can override this by providing the CPU architecture details via TargetConfig.yaml file.
This file is expected at the location: /etc/resource-tuner/custom/TargetConfig.yaml

## Sample Device Config file
```yaml
TargetConfig:
  - TargetName: QCS9100
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

---

# 5. How to add a new Property
Resource Tuner provides a set of Common Properties that are applicable to all devices. Users can specify their own custom Properties on top of the common ones.
Custom Properties can be specified in a yaml file (similar to Common Properties).

Common Properties are defined by resource-tuner in the /etc/resource-tuner/common/PropertiesConfig.yaml file, to add your own custom properties one of the following 2 strategies can be followed:

1. Add the Custom PropertiesConfig.yaml at /etc/resource-tuner/custom. Note the file name must exactly match "PropertiesConfig.yaml". As part of initialization, resource-tuner will check if this file is present, if it is, it will be parsed alongside the Common Properties.

2. The custom Properties file can also be placed in a different location other than /etc/resource-tuner/custom/. This can be done through Resource Tuner's Extension Interface. For example if the file is present at /opt/custom/PropertiesConfig.yaml, then the RESTUNE_REGISTER_CONFIG macro can be used as follows:

```cpp
RESTUNE_REGISTER_CONFIG(PROPERTIES_CONFIG, "/opt/custom/PropertiesConfig.yaml")
```

Note:
Each Property is indexed by the Prop Name, if Custom Properties are provided then one of the following scenarios will take place:
- If a Property with the same name already exists (in the Common Properties), the new value will overwrite the older value.
- If a Property with the same name does not exist (in the Common Properties), then a new Property with the provided Name and Value shall be created and stored in the Property Registry.

## Sample Properties config file.
```yaml
PropertyConfigs:
  - Name: resource_tuner.penalty.factor
    Value: "2.0"

  - Name: resource_tuner.reward.factor
    Value: "0.4"

  - Name: urm.logging.level
    Value: "0"

  - Name: urm.logging.level.exact
    Value: "false"
```

# 6. How to create a Resource Opcode (ResCode) and Signal Opcode (SigCode)
Suppose we would like to tune the following resource
```yaml
ResourceConfigs:
  - ResType: "0xea"
    ResID: "0x11fc"
    Name: "A_DEMONSTRATION_RESOURCE"
    Path: "/proc/some_path_to_some_resource/A"
    Supported: true
    Permissions: "third_party"
    Modes: ["display_on", "doze"]
    Policy: "lower_is_better"
    ApplyType: "global"
```

The ResCode is an unsigned 32 bit integer composed of two fields:
- ResID (last 16 bits, 17 - 32)
- ResType (next 8 bits, 9 - 16)
- Additionally MSB should be set to '1' if customer or other modules or target chipset is providing it's own custom resource config files, indicating this is a custom resource else it shall be treated as a default resource. This bit doesn't influence resource processing, just to aid debugging and development.

To construct one for the above Resource, we first note that this is a custom Resource (or downstream resource). Hence, the MSB must be set 1, additionally as the yaml config indicates:
- The ResType is: "0xea" and
- The ResId is "0x11fc"

The ResCode would therefore be:
0x80ea11fc in hex notation, or:
10000000111010100001000111111100, in binary

The binary notation clearly tells the first bit is set to 1.

If the Resource above were a default (upstream) resource, the only difference in the ResCode would be the MSB being turned off, i.e.
0x00ea11fc in hex notation or:
00000000111010100001000111111100, in binary

The above rules are similar for generating SigCode:
The SigCode is an unsigned 32 bit integer composed of two fields:
- The last 16 bits (17-32) are used to specify the SigID
- The next 8 bits (9-16) are used to specify the Signal Category
- In addition for Custom Signals, the MSB must be set to 1 as well

Suppose we would like to tune the following signal
```yaml
SignalConfigs:
  - SigId: "0x0008"
    Category: "0x0d"
    Name: DEMO_SIGNAL
    Enable: true
    Permissions: ["system"]
    Timeout: 50000
    Resources:
      - {ResCode: "0x00090002", ResInfo: "0x00000000", Values: [2, 0,1,2,3,4]}
      - {ResCode: "0x00090009", ResInfo: "0x00000000", Values: [2, 170]}
      - {ResCode: "0x00090002", ResInfo: "0x00000000", Values: [3, 4,5,6]}
      - {ResCode: "0x00090002", ResInfo: "0x00000000", Values: [4, 0,1,2,3,4,5,6]}
      - {ResCode: "0x00090009", ResInfo: "0x00000000", Values: [4, 150]}
      - {ResCode: "0x00090011", ResInfo: "0x00000000", Values: [4, -20]}

```

Treating it as a custom or downstream Signal config (i.e. MSB should be set to 1), the SigCode can be generated as follows:
- The Category is: "0x0d" and
- The SigID is "0x0008"

Hence, the resulting SigCode for this signal is:

0x800d0008 in hex notation, or
10000000000011010000000000001000, in binary.

---
