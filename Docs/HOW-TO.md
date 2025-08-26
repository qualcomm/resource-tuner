# How to add a custom resource
The common resources are available at /etc/resource-tuner/common/ResourcesConfig.yaml

Please follow below steps to add a new resource  
1. Create 

ResType =>
### Resource Type Codes

| Code    | Resource Type       | Description                        |
|---------|---------------------|------------------------------------|
| `0x03`  | Sched               | Scheduling-related resources       |
| `0x04`  | Power               | Power management resources         |
| `0x09`  | Cgroup              | Control group-based resources      |
| —       | MPAM                | Memory Partitioning and Monitoring |


ResID =>
        Unique Idenetifier represents a resource within the ResType group.
        Developer can 
Name => Unique string representing the Resource

Path => sysfs or procfs file path if applicable.

HighThreshold => 


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

# How to add a new feature

# How to add property

# how to add init configs

# How to enforce cpu arch details
