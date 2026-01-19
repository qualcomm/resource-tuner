\page intro Introduction to URM

# Introduction

Userspace Resource Manager is a lightweight framework which provides below
- Contextual Classifier : Identifies static contexts of workloads
- Resource Tuner : Helps to dynamically provision system resources like CPU, memory, Gpu, I/O, etc. It leverages kernel interfaces like procfs, sysfs and cgroups to infleunce the resource usage to ensure power and performance of applications or usecases in embedded and resource-constrained environments.

Gaining control over system resources such as the CPU, caches, and GPU is a powerful capability in any developer’s toolkit. By fine-tuning these components, developers can optimize the system’s operating point to make more efficient use of hardware resources and significantly enhance the user experience while saving power.

For example, increasing the CPU's dynamic clock and voltage scaling (DCVS) minimum frequency to 1 GHz can boost performance during demanding tasks. Conversely, capping the maximum frequency at 1.5 GHz can help conserve power during less intensive operations.

Resource Tuner supports `Signals` which is dynamic provisioning of system resources in response to specific signals —such as app launches or app installations —based on configurations defined in YAML. It allows other software modules or applications to register extensions and add custom functionality tailored to their specific needs.

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
- Contextual-Classifier -> Contains support for idenfitification of static usecases
- Tests -> Unit tests and module level tests
- CLI -> Command Line Interface to interact with service for debug and development purpose.

Userspace resource manager offers flexibility to select modules through the build system at compile time to make it suitable for devices which have stringent memory requirements. While Tests and Cli are debug and develoment modules which can be removed in the final product config. However resource tuner core module is mandatory.

Alter options in corresponding build file like below (ex. cmake options)
```cmake
option(BUILD_SIGNALS "Signals" OFF)
option(BUILD_CONTEXTUAL_CLASSIFIER, "ContextualClassifier" OFF)
option(BUILD_TESTS "Testing" OFF)
option(BUILD_CLI "CLI" OFF)
```

---

<div style="page-break-after: always;"></div>

## Project Structure

```text
/
├── Client
├── Server                 # Defines the Server Communication Endpoint and other Common Server-Side Utils
├── Resource Tuner
│   ├── Core
│   │   ├── Framework      # Core Resource Provisioning Request Flow Logic
│   │   ├── Modula         # Common Utilities and Components used across Resource Tuner Modules
│   ├── Signals            # Optional Module, exposes Signal Tuning / Relay APIs
├── Contextual Classifier
├── CLI                    # Exposes the Client Facing APIs, and Defines the Client Communication Endpoint
├── Configs                # Resources Config, Properties Config, Init Config, Signal Configs, Ext Feature Configs
├── Tests                  # Unit and System Wide Tests
└── Docs                   # Documentation
```

---

<div style="page-break-after: always;"></div>
