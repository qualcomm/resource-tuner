\page extensions Customizing URM: Extension Interface

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
