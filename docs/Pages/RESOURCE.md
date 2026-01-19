\page resource Resource Specifications

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
