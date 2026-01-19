\page urm_keyfeatures URM Key Features

# Userspace Resource Manager Key Points
Userspace resource manager (uRM) contains
- Userspace resource manager (uRM) exposes a variery of APIs for resource tuning and use-case/scenario tuning
- These APIs can be used by apps, features and other modules
- Using these APIs, client can tune any system resource parameters like cpu, dcvs, min / max frequencies etc.
- Userspace resource manager (uRM) provides

## Resource Tuner
- Queued requests processed by resource Tuner
- Set of Yaml config files provides extensive configuration capability
- Tuning resources provides control over system resources like CPU, Caches, GPU, etc for. Example changing the operating point of CPU DCVS min-freq to 1GHz to improve performance or limiting its max frequency to 1.5GHz to save power
- Tuning Signals dynamically provisions the system resources for a use case or scenario such as apps launches, installations, etc. in response to received signal. Resources can be configured in yaml for signals.
- Signals pick resources related to signal from SignalsConfig.yaml
- Extension interface provides a way to customize resource-tuner behaviour, by specifying custom resources, custom signals and features.
- Resource-tuner uses YAML based config files, for fetching information relating to resources/signals and properties.

## Contextual Classifier
The Contextual Classifier is an optional module designed to identify the static context of workloads (e.g., whether an application is a game, multimedia app, or browser) based on an offline-trained model.

**Key functionalities include:**
- **Process Event Monitoring:** Monitors process creation and termination events via Netlink.
- **Process Classification:** Classifies workloads (e.g., game, multimedia) using fastText (if enabled at build time). If fastText is not built, a default inference mechanism that always classifies the workload as an application.
- **Signal Generation:** Retrieves specific signal details based on classified workload types.
- **Cgroup Management:** Dynamically manages cgroups by moving application threads to designated cgroups.
- **Action Application:** Calls `ApplyActions` to send tuning requests and `RemoveActions` to untune for process events.
- **Configurability:** Influenced by configuration files such as `fasttext_model_supervised.bin`, `classifier-blocklist.txt`, and `ignore-tokens.txt`.

<div style="page-break-after: always;"></div>

Here is a more detailed explanation of the key features discussed above:

## 1. Client-Level Permissions
Certain resources can be tuned only by system clients and some which have no such restrictions and can be tuned even by third party clients. The client permissions are dynamically determined, the first time it makes a request. If a client with third party permissions tries to tune a resource, which allows only clients with system permissions to tune it, then the request shall be dropped.

## 2. Resource-Level Policies
To ensure efficient and predictable handling of concurrent requests, each system resource is governed by one of four predefined policies. Selecting the appropriate policy helps maintain system stability, optimize performance/power, and align resource behavior with application requirements.

- Instant Apply: This policy is for resources where the latest request needs to be honored. This is kept as the default policy.
- Higher is better: This policy honors the request writing the highest value to the node. One of the cases where this makes sense is for resources that describe the upper bound value. By applying the higher-valued request, the lower-valued request is implicitly honored.
- Lower is better: Works exactly opposite of the higher is better policy.
- Lazy Apply: Sometimes, you want the resources to apply requests in a first-in-first-out manner.

## 3. Request-Level Priorities
As part of the tuneResources API call, client is allowed to specify a desired priority level for the request. Resource-tuner supports 2 priority levels:
- High
- Low

However when multiplexed with client-level permissions, effetive request level priorities would be
- System High [SH]
- System Low [SL]
- Third Party High (or Regular High) [TPH]
- Third Party Low (or Regular Low) [TPL]

Requests with a higher priority will always be prioritized, over another request with a lower priority. Note, the request priorities are related to the client permissions. A client with system permission is allowed to acquire any priority level it wants, however a client with third party permissions can only acquire either third party high (TPH) or third party low (TPL) level of priorities. If a client with third party permissions tries to acquire a System High or System Low level of priority, then the request will not be honoured.

## 4. Pulse Monitor: Detection of Dead Clients and Subsequent Cleanup
To improve efficiency and conserve memory, it is essential to regularly check for dead clients and free up any system resources associated with them. This includes, untuning all (if any) ongoing tune request issued by this client and freeing up the memory used to store client specific data (Example: client's list of requests (handles), health, permissions, threads associated with the client etc). resource-tuner ensures that such clients are detected and cleaned up within 90 seconds of the client terminated.

Resource-tuner performs these actions by making use of two components:
- Pulse check: scans the list of the active clients, and checks if any of the client (PID) is dead. If it finds a dead client, it schedules the cleanup by adding this PID to a queue.
- Garbage collection: When the thread runs it iterates over the GC queue and performs the cleanup.

Pulse Monitor runs on a seperate thread peroidically.

## 5. Rate Limiter: Preventing System Abuse
Resource-tuner has rate limiter module that prevents abuse of the system by limiting the number of requests a client can make within a given time frame. This helps to prevent clients from overwhelming the system with requests and ensures that the system remains responsive and efficient. Rate limiter works on a reward/punishment methodology. Whenever a client requests the system for the first time, it is assigned a "Health" of 100. A punishment is handed over if a client makes subsequent new requests in a very short time interval (called delta, say 2 ms).
A Reward results in increasing the health of a client (not above 100), while a punishment involves decreasing the health of the client. If at any point this value of Health reaches zero then any further requests from this client wil be dropped. Value of delta, punishment and rewards are target-configurable.

## 6. Duplicate Checking
Resource-tuner's RequestManager component is responsible for detecting any duplicate requests issued by a client, and dropping them. This is done by checking against a list of all the requests issued by a clientto identify a duplicate. If it is, then the request is dropped. If it is not, then the request is added and processed. Duplicate checking helps to improve system efficiency, by saving wasteful CPU time on processing duplicates.

## 7. Dynamic Mapper: Logical to Physical Mapping
Logical to physical core/cluster mapping helps to achieve application code portability across different chipsets on client side. Client can specify logical values for core and cluster. Resource-tuner will translate these values to their physical counterparts and apply the request accordingly. Logical to physical mapping helps to create system independent layer and helps to make the same client code interchangable across different targets.

Logical mapping entries can be found in InitConfig.yaml and can be modified if required.

Logical layer values always arranged from lower to higher cluster capacities.
If no names assigned to entries in the dynamic mapping table then cluster'number' will be the name of the cluster
for ex. LgcId 4 named as "cluster4"

below table present in InitConfigs->ClusterMap section
| LgcId  |     Name   |
|--------|------------|
|   0    |   "little" |
|   1    |   "big"    |
|   2    |    "prime" |

resource-tuner reads machine topology and prepares logical to physical table dynamically in the init phase, similar to below one
| LgcId  |  PhyId  |
|--------|---------|
|   0    |     0   |
|   1    |     1   |
|   2    |     3   |
|   3    |     2   |

## 8. Display-Aware Operational Modes
The system's operational modes are influenced by the state of the device's display. To conserve power, certain system resources are optimized only when the display is active. However, for critical components that require consistent performance—such as during background processing or time-sensitive tasks, resource tuning can still be applied even when the display is off, including during low-power states like doze mode. This ensures that essential operations maintain responsiveness without compromising overall energy efficiency.

## 9. Crash Recovery
In case of server crash, resource-tuner ensures that all the resource sysfs nodes are restored to a sane state, i.e. they are reset to their original values. This is done by maintaining a backup of all the resource's original values, before any modification was made on behalf of the clients by resource tuner. In the event of server crash, reset to their original values in the backup.

## 10. Flexible Packaging
The Users are free to pick and choose the resource-tuner modules they want for their use-case and which fit their constraints. The Framework Module is the core/central module, however if the users choose they can add on top of it other Modules: signals and profiles.

## 11. Pre-Allocate Capacity for efficiency
Resource Tuner provides a MemoryPool component, which allows for pre-allocation of memory for certain commonly used type at the time of server initialization. This is done to improve the efficiency of the system, by reducing the number of memory allocations and deallocations that are required during the processing of requests. The allocated memory is managed as a series of blocks which can be recycled without any system call overhead. This reduces the overhead of memory allocation and deallocation, and improves the performance of the system.

Further, a ThreadPool component is provided to pre-allocate processing capacity. This is done to improve the efficiency of the system, by reducing the number of thread creation and destruction required during the processing of Requests, further ThreadPool allows for the threads to be repeatedly reused for processing different tasks.

---

<div style="page-break-after: always;"></div>
