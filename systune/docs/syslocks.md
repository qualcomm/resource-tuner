# Syslocks {#Syslocks}

[TOC]

## Overview
Syslocks is an abstraction layer between the user space and the sysfs nodes is necessary to mediate access and ensure that all user-space programs can reliably and fairly request configuration changes. This allows flexibility to all processes to fine-tune system parameters according to their power/performance needs. Without such a framework, concurrent access leads to race conditions, overwrites, or the need for inefficient polling mechanisms for each program. This not only increases complexity but also opens the door to denial-of-service scenarios, instability and unpredictability.

## Existing Solutions
- Qualcomm’s perfHAL is a software stack that deals with these issues, mainly for Android.
- In upstream implementations, RedHat offers a simplistic service, TuneD, to set system which are a combination of modified sysfs nodes
- Our goal is to build a unified, extensible solution that can serve as a foundation for various business units (BUs) to build upon.

## Proposed Solution
- Syslocks acts as a middleware layer between the user and the sysfs nodes.
- Programs can submit requests with the desired values and durations, and syslocks applies these in a fair, best-effort manner. The service ensures fairness and deals with concurrency appropriately.
- BUs can define custom resources and actions as per their needs.

### Software Components Overview

Below is the software components overview highlighting mainly the components used in the Syslocks software development.

<img src="design_syslocks.png" alt="Syslocks Design Overview" width="1000" height="500">

### Implementation Details

Below is the implementation details on how a resource request is being processed in the Syslocks.

<img src="implementation_syslocks.png" alt="Syslocks Design Overview" width="1000" height="500">

### Novely and Impact
- The future plan is to include support for VMs, so that clients running on these VMs can issue resource provisioning requests.
- Security modules like SELinux, Smack etc will be integrated into the software stack.
- Prevention of abuse of framework is prevented by **Client Rate Limiting capability**.
- Service is higly extensible by various BUs using extensions module.
- Open source this service to enable developers to leverage our infrastructure to build efficient applications and contribute to the community as a whole.

To know more about syslocks, please refer to the ppt:
- [Doc](https://qualcomm-my.sharepoint.com/:p:/g/personal/veershah_qti_qualcomm_com1/EeJqIUnUMUZLsLJ5wIBnK3wBjhKInTcyWRwcL2uNGIWClA)


## Components
Pulse monitor can be referred from here

- \subpage Pulse_Monitor

- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
# Document Revision History {#syslocks_hist}

Revision | Date          | Author         | Description              | Status
---------|---------------|----------------|--------------------------|-------
0.1      | 30 May 2025   | Vaibhav Jindal | First version            | Draft

[Back To Top](@ref Syslocks)