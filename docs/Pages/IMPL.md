\page impl_details Implementation and Feature Details

<img src="design_resource_tuner.png" alt="Resource Tuner Design" width="70%"/>

Resource-tuner architecture is captured above.

## Initialization
- During the server initialization phase, the YAML config files are read to build up the resource registry, property store etc.
- If the target chipset has registered any custom resources, signals or custom YAML files via the extension interface, then these changes are detected during this phase itself to build up a consolidated system view, before it can start serving requests.
- During the initialization phase, memory is pre-allocated for commonly used types (via MemoryPool), and worker (thread) capacity is reserved in advance via the ThreadPool, to avoid any delays during the request processing phase.
- Resource-tuner will also fetch the target details, like target name, total number of cores, logical to physical cluster / core mapping in this phase.
- If the Signals module is plugged in, it will be initialized as well and the signal configs will be parsed similarly to resource configs.
- Once all the initialization is completed, the server is ready to serve requests, a new listener thread is created for handling requests.

<div style="page-break-after: always;"></div>

## Request Processing
- The client can use the resource-tuner client library to send their requests.
- Resource-tuner supports sockets and binders for client-server communication.
- As soon as the request is received on the server end, a handle is generated and returned to the client. This handle uniquely identifies the request and can be used for subsequent retune (retuneResources) or untune (untuneResources) API calls.
- The request is submitted to the ThreadPool for async processing.
- When the request is picked up by a worker thread (from the ThreadPool), it will decode the request message and then validate the request.
- The request verifier, will run a series of checks on the request like permission checks, and on the resources part of the request, like config value bounds check.
- Once request is verified, a duplicate check is performed, to verify if the client has already submitted the same request before. This is done so as to the improve system efficiency and performace.
- Next the request is added to an queue, which is essentially PriorityQueue, which orders requests based on their priorities (for more details on Priority Levels, refer the next Section). This is done so that the request with the highest priority is always served first.
- To handle concurrent requests for the same resource, we maintain resource level linked lists of pending requests, which are ordered according to the request priority and resource policy. This ensures that the request with the higher priority will always be applied first. For two requests with the same priority, the application order will depend on resource policy. For example, in case of resource with "higher is better" policy, the request with a higher configuration value for the resource shall take effect first.
- Once a request reaches the head of the resource level linked list, it is applied, i.e. the config value specified by this request for the resource takes effect on the corresponding sysfs node.
- A timer is created and used to keep track of a request, i.e. check if it has expired. Once it is detected that the request has expired an untune request for the same handle as this request, is automatically generated and submitted, it will take care of resetting the effected resource nodes to their original values.
- Client modules can provide their own custom resource actions for any resource. The default action provided by resource-tuner is writing to the resource sysfs node.


## Flow of Events

```
+---------------------------+
| Process Event Listener    |
|       (NetLinkComm)       |
+-------------+-------------+
              |
              | Catches process events (e.g., fork, exec, exit)
              V
+---------------------------+
|      HandleProcEv()       |
| (Filters and queues events)|
+-------------+-------------+
              |
              | Notifies worker thread
              V
+---------------------------+
|     ClassifierMain()      |
|    (Worker Thread)        |
+-------------+-------------+
              |
              +----(Event Type)----+
              |                     |
              V                     V
        +------------+        +------------+
        | CC_APP_OPEN|        | CC_APP_CLOSE|
        +-----+------+        +-----+------+
              |                     |
              V                     V
   +-----------------------+    +---------------------------+
   | Is Ignored Process?   |--->| Move Process to Original  |
   | (classifier-blocklist)|    |   Cgroup                  |
   +----------+------------+    +---+-----------------------+
              |  No                 |
              V                     V
   +---------------------+   +---------------------------+
   |  ClassifyProcess()  |   |   RemoveActions           |
   | (MLInference model) |   |    (untuneSignal)         |
   +----------+----------+   +---------------------------+
              |
              V
   +---------------------+
   | GetSignalDetailsFor |
   |      Workload()     |
   +----------+----------+
              |
              V
   +----------------------------+
   | MoveAppThreadsToCGroup()   |
   | (Assign to Focused Cgroup) |
   +----------+-----------------+
              |
              V
   +---------------------+
   |    ApplyActions     |
   |    (tuneSignal)     |
   +---------------------+
```

---
