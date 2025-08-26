<div style="page-break-after: always;"></div>
# 1. How to add a custom resource
Resource tuner is configured with default [Resources](../Core/Configs/ResourcesConfig.yaml)  
The common resources are available on device at /etc/resource-tuner/common/ResourcesConfig.yaml.  
Developer can add new resources.  
Custom resource files are expected to be present on the device at /etc/resource-tuner/custom/ResourcesConfig.yaml.

## Sample Resource config file with two resources.
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

##### ResType - Resource Type
8 bit Resoure Type representing a Class of resources.
| Code    | Resource Type       | Description                        |
|---------|---------------------|------------------------------------|
| `0x03`  | Sched               | Scheduling-related resources       |
| `0x04`  | Power               | Power management resources         |
| `0x05`  | MPAM                | Memory Partitioning and Monitoring |
| `0x09`  | Cgroup              | Control group based resources      |

##### ResID - Resource ID
16 bit unique idenetifier represents a resource within the ResType group.
##### Name
Unique string representing the Resource
##### Path
Full resource path of sysfs or procfs file path (if applicable).
##### HighThreshold
Maximum value the can be set to this resource via resource-tuner.
##### LowThreshold
Minimum value the can be set to this resource via resource-tuner.
##### Permissions
Type of clients allowed to Provision this Resource.
Supported Permissions: system, third_party.
##### Modes
Mention the modes which this resouce is applicable for.
Supported modes: display_off, display_on, doze. (Bydefault the mode is display_on).
##### Policy
Policy to reolve conflict during the concurrent requests.
Supported policies: higher_is_better, lower_is_better, instant_apply, lazy_apply.
##### ApplyType
Supported types: global, core, cluster, cgroup.

<div style="page-break-after: always;"></div>
# 2. How to override the Resource tune and untune functionalities.
Resource tuner provides the default tune and untune functions.
However, developer can customize the resource tune and untune functionalities.
These overrides can be achieved by writing a new shared lib <>.so.<version>
These shared libs are expected to be present on the device at: /etc/resource-tuner/custom/
Please refer to [sample code](Examples/Extensions/plugin.cpp)

<div style="page-break-after: always;"></div>
# 3. How to add property

<div style="page-break-after: always;"></div>
# 4. How to add init configs
Resource tuner supports Cgroups as well, the default cgroup configs are defined [InitConfig](../Core/Configs/InitConfig.yaml)
Thes config file is used to create the Cgroups during the service startup.
Developer can supply additional init configs file via /etc/resource-tuner/custom/InitConfig.yaml
## Sample Cgroup config
    InitConfigs:
      - CgroupsInfo:
        - Name: "camera-cgroup"
          ID: 0

<div style="page-break-after: always;"></div>
# 5. How to enforce CPU architecture details
CPU architecture refers to
* No of clusters in the system
* No of cores in the each cluster 

Resource Tuner detects the CPU architecture based on cpu_capacity.
Developer can override this by providing the CPU architecture details via yaml file.
The override config file is expected at /etc/resource-tuner/custom/DeviceConfig.yaml
## Sample Device Config file
    TargetConfig:
      - TargetName: [TargetName1, TargetName2]
        ClusterInfo:
          - LgcId: 0
            PhyId: 1
          - LgcId: 1
            PhyId: 0
          - LgcId: 2
            PhyId: 2
          - LgcId: 3
            PhyId: 3
        ClusterSpread:
          - PhyId: 0
            NumCores: 4
          - PhyId: 1
            NumCores: 4
          - PhyId: 2
            NumCores: 4
          - PhyId: 3
            NumCores: 4
