#include <string>
#include <vector>
#include <sstream>
#include <fcntl.h>      // for open, O_RDWR
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <cstdint>
#include <dirent.h>
#include <cstring>
#include <algorithm>

typedef uint32_t u32;
typedef uint64_t u64;

// Include KGSL headers from kernel source

extern "C" {
#include "msm_kgsl.h"
#include "kgsl_gpu.h"
}

struct ProcStats {
    int pid;
    std::string name;
    int tty_nr_exists;
    int tpgid_exists;
    long minflt;
    long majflt;
    double utime;
    double stime;
    double cpu_time;       // user + system time
    int priority;
    int nice;
    int num_threads;
    uint64_t memory_rss;       // resident memory
    uint64_t memory_vms;       // virtual memory
    int rt_priority;
    int policy;
    double delayacct_blkio_ticks;
    int fg;
};

struct MemStats {
    int is_app;
    long vm_peak;
    long vm_lck;
    long vm_hwm;
    long vm_rss;
    long vms_size;
    long vm_data;
    long vm_stk;
    long vm_exe;
    long vm_lib;
    long vm_pte;
    long vm_pmd;
    long vm_swap;
    long vm_size;
    long threads;
};

struct IOStats {
    int pid;
    unsigned long read_bytes;
    unsigned long write_bytes;
    int open_file_count;
    int sock_count;
    int pipe_count;
    int chardev_count;
    int anonmaps_count;
};

struct SocketStats {
    std::string local_addr;
    std::string remote_addr;
    std::string state;
    unsigned long tx_queue;
    unsigned long rx_queue;
    int inode;
};

struct NwStats {
    unsigned long tcp_tx;
    unsigned long tcp_rx;
    unsigned long udp_tx;
    unsigned long udp_rx;
};

struct GpuStats {
    long busy_percent;
    unsigned long mem_total;
    unsigned long mem_allocated;
    unsigned long mem_free;
};

struct DispStats {
    int num_active_disp;
    int display_on;
};

struct SchedStats {
    uint64_t runtime_ns;    // total runtime
    uint64_t rq_wait_ns;    // time spent waiting on runqueue
    uint64_t timeslices;    // number of timeslices
};

std::string readFile(const std::string &path);
void FetchProcStats(int pid, ProcStats &stats);
void FetchMemStats(int pid, MemStats &memstats);
void countFDTypes(int pid, int &fileCount, int &socketCount, int &pipeCount, int &charDevCount);
int countAnonMemoryMaps(int pid);
void FetchIOStats(int pid, IOStats &iostats);
int parseNetFile(const std::string &path, std::vector<SocketStats> &sockets);
int getProcessSocketInodes(int pid, std::vector<int> &inodes);
void FetchNwStats(int pid, NwStats &netstats);
std::string FetchGpuStats(GpuStats &gpustats);
int getActiveDisplays(std::vector<std::string> &activeDisplays);
void FetchDisplayStats(DispStats &dispstats);
void read_schedstat(pid_t pid, SchedStats &ss);
void list_threads(pid_t pid, std::vector<pid_t>& tids);
void FetchDisplayStats(pid_t pid, SchedStats schedstats);
