#ifndef KGSL_GPU_H
#define KGSL_GPU_H

#include <stdint.h>
#include <sys/ioctl.h>

// Define types if not already defined
typedef uint32_t u32;
typedef uint64_t u64;

// Dummy struct for kgsl_memstats
struct kgsl_memstats {
    u64 gpu_memory;
    u64 system_memory;
    u64 gpu_mem_total;
    u64 gpu_mem_allocated;
    u64 gpu_mem_free;
};

// Dummy struct for kgsl_gpu_busy
struct kgsl_gpu_busy {
    u64 busy_time;
    u64 total_time;
};

// Dummy ioctl command numbers (these are placeholders)
#define KGSL_IOC_TYPE 0x09

#define KGSL_IOC_QUERY_MEMSTAT _IOWR(KGSL_IOC_TYPE, 0x3A, struct kgsl_memstats)
#define KGSL_IOC_GPU_BUSY      _IOWR(KGSL_IOC_TYPE, 0x3B, struct kgsl_gpu_busy)

#endif // KGSL_GPU_H
