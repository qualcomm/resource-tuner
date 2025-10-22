\page requests_formulation "Tune" Requests Formulation Guide

**API Signature:**
```cpp
int64_t tuneResources(int64_t duration,
                      int32_t prop,
                      int32_t numRes,
                      SysResource* resourceList);

```

The tuneResources API is designed to work will all Resource Classes, i.e. Cluster, Core, CGroup, MPAM etc. Following code snippets illustrate this:

- Cluster Apply Type Resources
Resources which can have different configured value across different clusters. To tune such a resource, along with the resource "ResCode", the desired cluster must also be passed, i.e. the cluster on which the value should take effect. Here, logical values are used by Resource Tuner to decouple the calling code from the device specifics.

=> Suppose, we are tuning a single resource as part of the resource. This resource with the ResCode: 0x0002a0be is a Cluster Apply Type Resource. The following example shows how to tune it on the logical cluster ID: 1
=> Internally, this logical cluster ID will be mapped to the appropriate physical cluster ID, and the configured value will be applied to that node.

```cpp
    SysResource* resourceList = new SysResource[1];
    memset(&resourceList[0], 0, sizeof(SysResource));
    resourceList[0].mResCode = 0x0002a0be;
    resourceList[0].mNumValues = 1;
    resourceList[0].mResValue.value = 999;

    // Populate the logical cluster
    resourceList[0].mResInfo = SET_RESOURCE_CLUSTER_VALUE(resourceList[0].mResInfo, 1);

    int64_t handle = tuneResources(-1, RequestPriority::REQ_PRIORITY_HIGH, 1, resourceList);
    // .....

```

Note: Setting the logical cluster ID only effectuates change, if the targeted resource actually has an ApplyType of "cluster" (this can be verified by referring "ResourcesConfig.yaml").

- Core Apply Type Resources

Resources which can have different configured value on different cores. To tune such a resource, along with the resource "ResCode", the desired core must also be passed, i.e. the core on which the value should take effect. Here, logical values are used.

=> Suppose, we are tuning a single resource as part of the resource. This resource with the ResCode: 0x00250dfc is a Core Apply Type Resource. The following example shows how to tune it on 2nd core in the second logical cluster.
=> Internally, this logical core ID will be mapped to the appropriate physical core ID, and the configured value will be applied to that core.

```cpp
    SysResource* resourceList = new SysResource[1];
    memset(&resourceList[0], 0, sizeof(SysResource));
    resourceList[0].mResCode = 0x00250dfc;
    resourceList[0].mNumValues = 1;
    resourceList[0].mResValue.value = 800;

    // Populate the logical cluster
    resourceList[0].mResInfo = SET_RESOURCE_CLUSTER_VALUE(resourceList[0].mResInfo, 2);
    resourceList[0].mResInfo = SET_RESOURCE_CORE_VALUE(resourceList[0].mResInfo, 2);

    int64_t handle = tuneResources(-1, RequestPriority::REQ_PRIORITY_HIGH, 1, resourceList);
    // .....

```

Note: Setting the logical core ID only effectuates change, if the targeted resource actually has an ApplyType of "core" (this can be verified by referring "ResourcesConfig.yaml").


- CGroup Apply Type Resources

Resources which can have different configured value across different cgroups, for example the node "memory.max" can have different values for cgrpA and cgrpB. To tune such a resource, along with the resource "ResCode", the desired cgroup identifier must also be passed, i.e. the cgroup on which the value should take effect.

=> To obtain the correct cgroup identifier, refer the config file "InitConfigs.yaml", which maps identifiers (integer values) to cgroup names.
=> Resource Tuner, supports multi-valued resources, and expects the first value to be the cgroup identifier, the following values are treated as values, and can differ according to the resource node.
=> Internally, this identifier will be used to identify the correct cgroup node to which the configured value should be applied.

=> Suppose, we are tuning a single resource as part of the resource. This resource with the ResCode: 0x00090007 is a CGroup Apply Type Resource. The following example shows how to tune it for the cgroup with the identifier: 1

```cpp
    SysResource* resourceList = new SysResource[1];
    memset(&resourceList[0], 0, sizeof(SysResource));

    resourceList[0].mResCode = 0x00090007;
    resourceList[0].mNumValues = 2;
    resourceList[0].mResValue.values = new int32_t[2];
    resourceList[0].mResValue.values[0] = 0;
    resourceList[0].mResValue.values[1] = 55;

    int64_t handle = tuneResources(-1, RequestPriority::REQ_PRIORITY_HIGH, 1, resourceList);
    // .....

```

The following example, illustrates how to tune the "cpuset.cpus" node for a particular cgroup

```cpp
    SysResource* resourceList = new SysResource[1];
    memset(&resourceList[0], 0, sizeof(SysResource));

    resourceList[0].mResCode = 0x00090002;
    resourceList[0].mNumValues = 4;
    resourceList[0].mResValue.values = new int32_t[resourceList[0].mNumValues];
    resourceList[0].mResValue.values[0] = 1;
    resourceList[0].mResValue.values[1] = 0;
    resourceList[0].mResValue.values[2] = 1;
    resourceList[0].mResValue.values[3] = 3;
```

This will update the cpuset.cpus node for the cgroup with identifier: 1, to: "0-1,3"

The following example, illustrates how to tune the "memory.max" node for a particular cgroup

```cpp
    SysResource* resourceList = new SysResource[1];
    memset(&resourceList[0], 0, sizeof(SysResource));

    resourceList[0].mResCode = 0x0009000b;
    resourceList[0].mNumValues = 2;
    resourceList[0].mResValue.values = new int32_t[2];
    resourceList[0].mResValue.values[0] = 1;
    resourceList[0].mResValue.values[1] = 512 * 1024 * 1024;
```

A similar strategy can be adopted for other cgroup resources as well.
