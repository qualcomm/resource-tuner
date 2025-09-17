\page extensions Extension Interface Guide

Refer Plugin.cpp in Examples section, for basic code-guidance.

# 1. Making Use of Extensions Interface
Resource Tuner's Extension Interface exposes macros that can be used to customize callbacks and Configs. To make use of this capability client's need to create a shared library, called libPlugin.so and place it at /etc/resource-tuner/custom.
This library should contain all the customization logic. This strategy allows the Clients to keep their code wherever they want. The only constraints are in terms of library naming, and library location.

Refer Plugin.cpp for instructions on building the lib.
Note, the lib must have appropriate permissions so that Resource Tuner can read it.

# 2. Applier and Teardown Callbacks
Resource Tuner's Extension Interface allows for users to specify Custom Appliers and Teardown Callbacks for each Resource. Resource Tuner will provide default Applier and Teardown callbacks, but hte user can override them as per their needs.

=> Applier Callback: This callback is called when a Request to tune a Resource is about to be applied. The default applier writes the configured value to the associated Resource (sysfs / procfs, cgroup etc.) Node.

=> Teardown Callback: This callback is called when the currently applied Request for a Resource expires and there are no other Pending Requests. The default teardown callback resets the Resource Node value to its original value.

Using the Extensions Interface, Users can easily extend the Resource Tuner's functionality to tune New Resources and apply their own Custom Logic for Applier and Teardown Callbacks.

The following examples shows a sample custom Applier callback for Resource with the ResCode: 0x00090005.

```yaml
ResourceConfigs:
  - ResType: "0x09"
    ResID: "0x0005"
    Name: "RES_TUNER_CGROUP_LIMIT_CPU_TIME"
    Path: "/sys/fs/cgroup/%s/cpu.max"
    Supported: true
    Permissions: "system"
    ApplyType: "cgroup"
    Policy: higher_is_better
```

<br />

```cpp
static void limitCpuTime(void* context) {
    if(context == nullptr) return;
    Resource* resource = static_cast<Resource*>(context);

    if(resource->getValuesCount() != 3) return;
    if(resource->mResValue.values == nullptr) return;

    int32_t cGroupIdentifier = (*resource->mResValue.values)[0];
    int32_t maxUsageMicroseconds = (*resource->mResValue.values)[1];
    int32_t periodMicroseconds = (*resource->mResValue.values)[2];
    CGroupConfigInfo* cGroupConfig = TargetRegistry::getInstance()->getCGroupConfig(cGroupIdentifier);

    if(cGroupConfig != nullptr) {
        const std::string cGroupName = cGroupConfig->mCgroupName;

        if(cGroupName.length() > 0) {
            std::string controllerFilePath = getCGroupTypeResourceNodePath(resource, cGroupName);
            std::ofstream controllerFile(controllerFilePath);

            if(!controllerFile.is_open()) {
                TYPELOGV(ERRNO_LOG, "open", strerror(errno));
                return;
            }

            controllerFile<<maxUsageMicroseconds<<" "<<periodMicroseconds<<std::endl;

            if(controllerFile.fail()) {
                TYPELOGV(ERRNO_LOG, "write", strerror(errno));
            }
            controllerFile.close();
        }
    } else {
        TYPELOGV(VERIFIER_CGROUP_NOT_FOUND, cGroupIdentifier);
    }
}

// Register the Custom Applier Callback with Resource Tuner
RESTUNE_REGISTER_APPLIER_CB(0x00090005, limitCpuTime);
```

# 3. Custom Configs
Resource Tuner allows users to provide their own Config files for different entities like: Resources, Signals, Properties etc. In order for these custom config files to be read and parsed they can either be placed in /etc/resource-tuner or if the user wants even more flexibility in terms of location, they can specify the location of the Config file, using the Extensions Interface's RESTUNE_REGISTER_CONFIG macro.

For example:
```cpp
RESTUNE_REGISTER_CONFIG(RESOURCE_CONFIG, "/opt/custom/ResourcesConfig.yaml")
```

This will tell Resource Tuner that the custom Resources Config file is located at: "/opt/custom/ResourcesConfig.yaml". Similar strategy can be followed for Signals, Properties, Ext Features etc.

---
