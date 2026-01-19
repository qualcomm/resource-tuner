\page sil Consistent Indexing through the System Independent Layer (SIL)

# System Independent Layer
This section defines logical layer to improve code and config portability across different targets and systems.
Please avoid changing these configs.

## Logical IDs
### 1. Logical Cluster Map
Logical IDs for clusters. Configs of cluster map can be found in InitConfigs->ClusterMap section
| LgcId  |     Name   |
|--------|------------|
|   0    |   "little" |
|   1    |   "big"    |
|   2    |   "prime"  |

### 2. Cgroups map
Logical IDs for cgroups. Configs of cgroups map in InitConfigs->CgroupsInfo section
| Lgc Cgrp No |   Cgrp Name      |
|-------------|------------------|
|      0      |  "root"          |
|      1      |  "init.scope"    |
|      2      |  "system.slice"  |
|      3      |  "user.slice"    |
|      4      |  "focused.slice" |

### 3. Mpam Groups Map
Logical IDs for MPAM groups. Configs of mpam grp map in InitConfigs->MpamGroupsInfo section
| LgcId  |    Mpam grp Name  |
|--------|-------------------|
|   0    |      "default"    |
|   1    |       "video"     |
|   2    |       "camera"    |
|   3    |       "games"     |

## Resources
Resource Types/Categories
|      Resource Type    | type  |
|-----------------------|-------|
|   Lpm                 |   0   |
|   Caches              |   1   |
|   Sched               |   3   |
|   Dcvs                |   4   |
|   Gpu                 |   5   |
|   Npu                 |   6   |
|   Memory              |   7   |
|   Mpam                |   8   |
|   Cgroup              |   9   |
|   Storage/Io          |   a   |

Resource Code is composed of 2 least significant bytes contains resource ID and next significant byte contains resource type
i.e. 0xrr tt iiii  (i: resource id, t: resoruce type, r: reserved)

|      Resource Name                          |    Id             |
|---------------------------------------------|-------------------|
|   RES_TUNER_CPU_DMA_LATENCY                 |   0x 00 01 0000   |
|   RES_TUNER_PM_QOS_LATENCY                  |   0x 00 01 0001   |
|   RES_TUNER_SCHED_UTIL_CLAMP_MIN            |   0x 00 03 0000   |
|   RES_TUNER_SCHED_UTIL_CLAMP_MAX            |   0x 00 03 0001   |
|   RES_TUNER_SCHED_ENERGY_AWARE              |   0x 00 03 0002   |
|   RES_TUNER_SCALING_MIN_FREQ                |   0x 00 04 0000   |
|   RES_TUNER_SCALING_MAX_FREQ                |   0x 00 04 0001   |
|   RES_TUNER_RATE_LIMIT_US                   |   0x 00 04 0002   |
|   RES_TUNER_CGROUP_MOVE_PID                 |   0x 00 09 0000   |
|   RES_TUNER_CGROUP_MOVE_TID                 |   0x 00 09 0001   |
|   RES_TUNER_CGROUP_RUN_ON_CORES             |   0x 00 09 0002   |
|   RES_TUNER_CGROUP_RUN_ON_CORES_EXCLUSIVELY |   0x 00 09 0003   |
|   RES_TUNER_CGROUP_FREEZE                   |   0x 00 09 0004   |
|   RES_TUNER_CGROUP_LIMIT_CPU_TIME           |   0x 00 09 0005   |
|   RES_TUNER_CGROUP_RUN_WHEN_CPU_IDLE        |   0x 00 09 0006   |
|   RES_TUNER_CGROUP_UCLAMP_MIN               |   0x 00 09 0007   |
|   RES_TUNER_CGROUP_UCLAMP_MAX               |   0x 00 09 0008   |
|   RES_TUNER_CGROUP_RELATIVE_CPU_WEIGHT      |   0x 00 09 0009   |
|   RES_TUNER_CGROUP_HIGH_MEMORY              |   0x 00 09 000a   |
|   RES_TUNER_CGROUP_MAX_MEMORY               |   0x 00 09 000b   |
|   RES_TUNER_CGROUP_LOW_MEMORY               |   0x 00 09 000c   |
|   RES_TUNER_CGROUP_MIN_MEMORY               |   0x 00 09 000d   |
|   RES_TUNER_CGROUP_SWAP_MAX_MEMORY          |   0x 00 09 000e   |
|   RES_TUNER_CGROUP_IO_WEIGHT                |   0x 00 09 000f   |
|   RES_TUNER_CGROUP_CPU_LATENCY              |   0x 00 09 0011   |

These are defined in resource config file, but should not be changed. Resources can be added.

## Signals
Signals

|      Signal           |     Id          |
|-----------------------|-----------------|
|   URM_APP_OPEN        |   0x 00000001   |
|   URM_APP_CLOSE       |   0x 00000002   |


<div style="page-break-after: always;"></div>
