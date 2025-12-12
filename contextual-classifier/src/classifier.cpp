#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#include <sys/socket.h>
#include <linux/netlink.h>
//#include <linux/connector.h>
//#include <linux/cn_proc.h>
#include "connector.h"
#include "cn_proc.h"
#include <signal.h>
#include <errno.h>
#include <stdbool.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <dlfcn.h>
#include <unordered_map>
#include <unordered_set>
//#include <ResourceTuner/ResourceTunerAPIs.h>
#include <syslog.h> // Include syslog for logging
#include <iostream>
#include <sstream>

#include <fstream>
#include <string>
#include <dirent.h>
#include <cstring>
#include <cstdlib>
#include <vector>
#include <algorithm>
#include <thread>
#include <queue>
#include <mutex>
#include <condition_variable>
#include "parser.h"
#include "ml_inference.h" // Include our new ML inference header
#include "proc_stats.h"
//#include "proc_metrics.h" // Include proc_metrics.h for fetching detailed process statistics

#define CLASSIFIER_CONF_DIR "/etc/classifier/"

// Thread pool and job queue
std::queue<int> classification_queue;
std::mutex queue_mutex;
std::condition_variable queue_cond;
std::vector<std::thread> thread_pool;
const int NUM_THREADS = 4;

// Define paths to ML artifacts
const std::string FT_MODEL_PATH = CLASSIFIER_CONF_DIR "fasttext_model.bin";
const std::string LGBM_MODEL_PATH = CLASSIFIER_CONF_DIR "lgbm_model.txt";
const std::string META_PATH = CLASSIFIER_CONF_DIR "meta.json";
const std::string IGNORE_PROC_PATH = CLASSIFIER_CONF_DIR "ignore_processes.txt";

std::unordered_set<std::string> ignored_processes;

void load_ignored_processes() {
    std::ifstream file(IGNORE_PROC_PATH);
    if (!file.is_open()) {
        syslog(LOG_WARNING, "Could not open ignore process file: %s", IGNORE_PROC_PATH.c_str());
        return;
    }
    std::string line;
    while (std::getline(file, line)) {
        // Trim whitespace
        line.erase(0, line.find_first_not_of(" \t\n\r"));
        line.erase(line.find_last_not_of(" \t\n\r") + 1);
        if (!line.empty()) {
            ignored_processes.insert(line);
        }
    }
    syslog(LOG_INFO, "Loaded %zu ignored processes.", ignored_processes.size());
}

// Singleton for MLInference
MLInference& get_ml_inference_instance() {
    static MLInference ml_inference_obj(FT_MODEL_PATH, LGBM_MODEL_PATH, META_PATH);
    return ml_inference_obj;
}

// Helper to check if a string contains only digits
bool is_digits(const std::string& str) {
    return std::all_of(str.begin(), str.end(), ::isdigit);
}

// Helper to check if process is still alive
bool is_process_alive(int pid) {
    std::string proc_path = "/proc/" + std::to_string(pid);
    if (access(proc_path.c_str(), F_OK) == -1) {
        syslog(LOG_DEBUG, "Process %d has exited.", pid);
        return false;
    }
    return true;
}

static void initialize(void) {
    //TODO: Do the setup required for resource-tuner.
}

/*
 * connect to netlink
 * returns netlink socket, or -1 on error
 */
static int nl_connect()
{
    int rc;
    int nl_sock;
    struct sockaddr_nl sa_nl;

    nl_sock = socket(PF_NETLINK, SOCK_DGRAM, NETLINK_CONNECTOR);
    if (nl_sock == -1) {
        syslog(LOG_ERR, "socket: %m");
        return -1;
    }

    sa_nl.nl_family = AF_NETLINK;
    sa_nl.nl_groups = CN_IDX_PROC;
    sa_nl.nl_pid = getpid();

    rc = bind(nl_sock, (struct sockaddr *)&sa_nl, sizeof(sa_nl));
    if (rc == -1) {
        syslog(LOG_ERR, "bind: %m");
        close(nl_sock);
        return -1;
    }

    return nl_sock;
}

/*
 * subscribe on proc events (process notifications)
 */
static int set_proc_ev_listen(int nl_sock, bool enable)
{
    int rc;
    struct __attribute__ ((aligned(NLMSG_ALIGNTO))) {
        struct nlmsghdr nl_hdr;
        struct __attribute__ ((__packed__)) {
            struct cn_msg cn_msg;
            enum proc_cn_mcast_op cn_mcast;
        };
    } nlcn_msg;

    memset(&nlcn_msg, 0, sizeof(nlcn_msg));
    nlcn_msg.nl_hdr.nlmsg_len = sizeof(nlcn_msg);
    nlcn_msg.nl_hdr.nlmsg_pid = getpid();
    nlcn_msg.nl_hdr.nlmsg_type = NLMSG_DONE;

    nlcn_msg.cn_msg.id.idx = CN_IDX_PROC;
    nlcn_msg.cn_msg.id.val = CN_VAL_PROC;
    nlcn_msg.cn_msg.len = sizeof(enum proc_cn_mcast_op);

    nlcn_msg.cn_mcast = enable ? PROC_CN_MCAST_LISTEN : PROC_CN_MCAST_IGNORE;

    rc = send(nl_sock, &nlcn_msg, sizeof(nlcn_msg), 0);
    if (rc == -1) {
        syslog(LOG_ERR, "netlink send: %m");
        return -1;
    }

    return 0;
}

/* Remove actions applied for perf stores */
static void remove_actions(int process_pid, int process_tgid,
                           std::unordered_map<int, int> & pid_perf_handle)
{
    /* Remove the process from CG ?*/
    /* TODO: Should we need to check periodically for tasks in cgroups */
    if (pid_perf_handle.find(process_pid) != pid_perf_handle.end()) {
        //untuneResource()
        pid_perf_handle.erase(process_pid);
    }
}

/* Process classfication based on selinux context of process
 * TODO: How to create or use cgroups based on process creation.
 * TODO: Apply utilization limit on process groups.
 */
static void classify_process(int process_pid, int process_tgid,
                             std::unordered_map <int, int> &pid_perf_handle,
                             MLInference& ml_inference_obj)
{
    // Check if the process still exists
    if (!is_process_alive(process_pid)) {
        return;
    }

    // Check if process should be ignored
    std::vector<std::string> comm_vec = parse_proc_comm(process_pid, "");
    if (!comm_vec.empty()) {
        std::string proc_name = comm_vec[0];
        // Trim whitespace just in case
        proc_name.erase(proc_name.find_last_not_of(" \n\r\t") + 1);
        if (ignored_processes.count(proc_name)) {
            syslog(LOG_DEBUG, "Skipping inference for ignored process: %s (PID: %d)", proc_name.c_str(), process_pid);
            return;
        }
    }

    syslog(LOG_DEBUG, "Starting classification for PID:%d", process_pid);

    std::map<std::string, std::string> raw_data;
    const auto& text_cols = ml_inference_obj.getTextCols();
    const auto& numeric_cols = ml_inference_obj.getNumericCols();

    // Collect Text Features
    for (const auto& col : text_cols) {
        std::vector<std::string> data_vec;
        if (col == "attr") {
            data_vec = parse_proc_attr_current(process_pid, " ");
        } else if (col == "cgroup") {
            data_vec = parse_proc_cgroup(process_pid, " ");
        } else if (col == "cmdline") {
            data_vec = parse_proc_cmdline(process_pid, " ");
        } else if (col == "comm") {
            data_vec = parse_proc_comm(process_pid, " ");
        } else if (col == "maps") {
            data_vec = parse_proc_map_files(process_pid, " ");
        } else if (col == "fds") {
            data_vec = parse_proc_fd(process_pid, " ");
        } else if (col == "environ") {
            data_vec = parse_proc_environ(process_pid, " ");
        } else if (col == "exe") {
            data_vec = parse_proc_exe(process_pid, " ");
        } else if (col == "logs") {
            data_vec = readJournalForPid(process_pid);
        } else {
            syslog(LOG_WARNING, "Unknown text column '%s' for PID:%d. Skipping.", col.c_str(), process_pid);
            continue;
        }

        std::stringstream ss;
        for(const auto& s : data_vec) {
            ss << s << " ";
        }
        raw_data[col] = ss.str();
        if (!raw_data[col].empty()) {
            raw_data[col].pop_back();
        }

        if (raw_data[col].empty()) {
            syslog(LOG_DEBUG, "PID:%d | Text Feature: %s | Value: <EMPTY> (Failed to collect or was empty)", process_pid, col.c_str());
        } else {
            syslog(LOG_DEBUG, "PID:%d | Text Feature: %s | Value: %s", process_pid, col.c_str(), raw_data[col].c_str());
        }
    }
    syslog(LOG_DEBUG, "Text features collected for PID:%d", process_pid);

    if (!is_process_alive(process_pid)) return;

    // Collect Numeric Features
    ProcStats proc_stats;
    FetchProcStats(process_pid, proc_stats);
    syslog(LOG_DEBUG, "PID:%d | ProcStats | Threads: %d, CPU Time: %f", process_pid, proc_stats.num_threads, proc_stats.cpu_time);
    MemStats mem_stats;
    FetchMemStats(process_pid, mem_stats);
    IOStats io_stats;
    FetchIOStats(process_pid, io_stats);
    NwStats nw_stats;
    FetchNwStats(process_pid, nw_stats);
    GpuStats gpu_stats;
    FetchGpuStats(gpu_stats);
    DispStats disp_stats;
    FetchDisplayStats(disp_stats);
    SchedStats sched_stats;
    read_schedstat(process_pid, sched_stats);

    for (const auto& col : numeric_cols) {
        if (col == "cpu_time") raw_data[col] = std::to_string(proc_stats.cpu_time);
        else if (col == "threads") raw_data[col] = std::to_string(proc_stats.num_threads);
        else if (col == "rss") raw_data[col] = std::to_string(proc_stats.memory_rss);
        else if (col == "vms") raw_data[col] = std::to_string(proc_stats.memory_vms);
        else if (col == "mem_vmpeak") raw_data[col] = std::to_string(mem_stats.vm_peak);
        else if (col == "mem_vmlck") raw_data[col] = std::to_string(mem_stats.vm_lck);
        else if (col == "mem_hwm") raw_data[col] = std::to_string(mem_stats.vm_hwm);
        else if (col == "mem_vm_rss") raw_data[col] = std::to_string(mem_stats.vm_rss);
        else if (col == "mem_vmsize") raw_data[col] = std::to_string(mem_stats.vm_size);
        else if (col == "mem_vmdata") raw_data[col] = std::to_string(mem_stats.vm_data);
        else if (col == "mem_vmstk") raw_data[col] = std::to_string(mem_stats.vm_stk);
        else if (col == "mem_vm_exe") raw_data[col] = std::to_string(mem_stats.vm_exe);
        else if (col == "mem_vmlib") raw_data[col] = std::to_string(mem_stats.vm_lib);
        else if (col == "mem_vmpte") raw_data[col] = std::to_string(mem_stats.vm_pte);
        else if (col == "mem_vmpmd") raw_data[col] = std::to_string(mem_stats.vm_pmd);
        else if (col == "mem_vmswap") raw_data[col] = std::to_string(mem_stats.vm_swap);
        else if (col == "mem_thread") raw_data[col] = std::to_string(mem_stats.threads);
        else if (col == "read_bytes") raw_data[col] = std::to_string(io_stats.read_bytes);
        else if (col == "write_bytes") raw_data[col] = std::to_string(io_stats.write_bytes);
        else if (col == "tcp_tx") raw_data[col] = std::to_string(nw_stats.tcp_tx);
        else if (col == "tcp_rx") raw_data[col] = std::to_string(nw_stats.tcp_rx);
        else if (col == "udp_tx") raw_data[col] = std::to_string(nw_stats.udp_tx);
        else if (col == "udp_rx") raw_data[col] = std::to_string(nw_stats.udp_rx);
        else if (col == "gpu_busy") raw_data[col] = std::to_string(gpu_stats.busy_percent);
        else if (col == "gpu_mem_allocated") raw_data[col] = std::to_string(gpu_stats.mem_allocated);
        else if (col == "display_on") raw_data[col] = std::to_string(disp_stats.display_on);
        else if (col == "active_displays") raw_data[col] = std::to_string(disp_stats.num_active_disp);
        else if (col == "runtime_ns") raw_data[col] = std::to_string(sched_stats.runtime_ns);
        else if (col == "rq_wait_ns") raw_data[col] = std::to_string(sched_stats.rq_wait_ns);
        else if (col == "timeslices") raw_data[col] = std::to_string(sched_stats.timeslices);
        else {
            syslog(LOG_WARNING, "Unknown numeric column '%s' for PID:%d. Defaulting to 0.", col.c_str(), process_pid);
            raw_data[col] = "0.0";
        }
    }
    syslog(LOG_DEBUG, "Numeric features collected for PID:%d", process_pid);

    if (!is_process_alive(process_pid)) return;

    bool has_sufficient_features = false;
    for (const auto& col : text_cols) {
        if (raw_data.count(col) && !raw_data.at(col).empty()) {
            has_sufficient_features = true;
            break;
        }
    }
    if (!has_sufficient_features) {
        for (const auto& col : numeric_cols) {
            if (raw_data.count(col) && raw_data.at(col) != "0.0" && raw_data.at(col) != "0") {
                has_sufficient_features = true;
                break;
            }
        }
    }

    if (has_sufficient_features) {
        if (!is_process_alive(process_pid)) return;

        syslog(LOG_DEBUG, "Invoking ML inference for PID:%d", process_pid);
        std::string predicted_label = ml_inference_obj.predict(raw_data);
        syslog(LOG_INFO, "PID:%d Classified as: %s", process_pid, predicted_label.c_str());
        // TODO: Apply resource tuning based on predicted_label
    } else {
        syslog(LOG_DEBUG, "Skipping ML inference for PID:%d due to insufficient features.", process_pid);
    }
}

/*
 * handle a single process event
 */
static volatile bool need_exit = false;

void worker_thread() {
    while (true) {
        int pid_to_classify;
        {
            std::unique_lock<std::mutex> lock(queue_mutex);
            queue_cond.wait(lock, []{ return !classification_queue.empty() || need_exit; });

            if (need_exit && classification_queue.empty()) {
                return;
            }

            pid_to_classify = classification_queue.front();
            classification_queue.pop();
        }

        std::unordered_map<int, int> pid_perf_handle;
        classify_process(pid_to_classify, 0, pid_perf_handle, get_ml_inference_instance());
    }
}

static int handle_proc_ev(int nl_sock, MLInference& ml_inference_obj)
{
    int rc;
    struct __attribute__ ((aligned(NLMSG_ALIGNTO))) {
        struct nlmsghdr nl_hdr;
        struct __attribute__ ((__packed__)) {
            struct cn_msg cn_msg;
            struct proc_event proc_ev;
        };
    } nlcn_msg;
    std::unordered_map <int, int> pid_perf_handle {};

    while (!need_exit) {
        rc = recv(nl_sock, &nlcn_msg, sizeof(nlcn_msg), 0);
        if (rc == 0) {
            /* shutdown? */
            return 0;
        } else if (rc == -1) {
            if (errno == EINTR) continue;
            syslog(LOG_ERR, "netlink recv: %m");
            return -1;
        }
        switch (nlcn_msg.proc_ev.what) {
            case proc_event::PROC_EVENT_NONE:
                // syslog(LOG_DEBUG, "set mcast listen ok");
                break;
            case proc_event::PROC_EVENT_FORK:
                syslog(LOG_DEBUG, "fork: parent tid=%d pid=%d -> child tid=%d pid=%d",
                       nlcn_msg.proc_ev.event_data.fork.parent_pid,
                       nlcn_msg.proc_ev.event_data.fork.parent_tgid,
                       nlcn_msg.proc_ev.event_data.fork.child_pid,
                       nlcn_msg.proc_ev.event_data.fork.child_tgid);
                break;
            case proc_event::PROC_EVENT_EXEC:
                syslog(LOG_DEBUG, "Received PROC_EVENT_EXEC for tid=%d pid=%d",
                       nlcn_msg.proc_ev.event_data.exec.process_pid,
                       nlcn_msg.proc_ev.event_data.exec.process_tgid);

                // Early filtering of ignored processes
                {
                    int pid = nlcn_msg.proc_ev.event_data.exec.process_pid;
                    std::vector<std::string> comm_vec = parse_proc_comm(pid, "");
                    if (comm_vec.empty()) {
                        syslog(LOG_DEBUG, "Process %d exited before initial check. Skipping.", pid);
                    } else {
                        std::string proc_name = comm_vec[0];
                        proc_name.erase(proc_name.find_last_not_of(" \n\r\t") + 1);
                        if (ignored_processes.count(proc_name)) {
                            syslog(LOG_DEBUG, "Ignoring process: %s (PID: %d)", proc_name.c_str(), pid);
                        } else {
                            std::lock_guard<std::mutex> lock(queue_mutex);
                            classification_queue.push(pid);
                            queue_cond.notify_one();
                        }
                    }
                }
                break;
            case proc_event::PROC_EVENT_UID:
                syslog(LOG_DEBUG, "uid change: tid=%d pid=%d from %d to %d",
                       nlcn_msg.proc_ev.event_data.id.process_pid,
                       nlcn_msg.proc_ev.event_data.id.process_tgid,
                       nlcn_msg.proc_ev.event_data.id.r.ruid,
                       nlcn_msg.proc_ev.event_data.id.e.euid);
                break;
            case proc_event::PROC_EVENT_GID:
                syslog(LOG_DEBUG, "gid change: tid=%d pid=%d from %d to %d",
                       nlcn_msg.proc_ev.event_data.id.process_pid,
                       nlcn_msg.proc_ev.event_data.id.process_tgid,
                       nlcn_msg.proc_ev.event_data.id.r.rgid,
                       nlcn_msg.proc_ev.event_data.id.e.egid);
                break;
            case proc_event::PROC_EVENT_EXIT:
                syslog(LOG_DEBUG, "exit: tid=%d pid=%d exit_code=%d",
                       nlcn_msg.proc_ev.event_data.exit.process_pid,
                       nlcn_msg.proc_ev.event_data.exit.process_tgid,
                       nlcn_msg.proc_ev.event_data.exit.exit_code);
                remove_actions(nlcn_msg.proc_ev.event_data.exec.process_pid,
                               nlcn_msg.proc_ev.event_data.exec.process_tgid,
                               pid_perf_handle);
                break;
            default:
                syslog(LOG_WARNING, "unhandled proc event");
                break;
        }
    }

    return 0;
}

static void on_sigint(int unused)
{
    need_exit = true;
}

int main(int argc, const char *argv[])
{
    int nl_sock;
    int rc = EXIT_SUCCESS;

    openlog("classifier", LOG_PID | LOG_CONS | LOG_NDELAY, LOG_DAEMON); // Initialize syslog
    syslog(LOG_INFO, "Classifier service started.");
    initialize();
    load_ignored_processes();
    
    for (int i = 0; i < NUM_THREADS; ++i) {
        thread_pool.emplace_back(worker_thread);
    }

    //signal(SIGINT, &on_sigint);
    //siginterrupt(SIGINT, true);

    // Get the MLInference singleton instance
    MLInference& ml_inference_obj = get_ml_inference_instance();
    syslog(LOG_INFO, "MLInference object initialized.");

    nl_sock = nl_connect();
    if (nl_sock == -1) {
        syslog(LOG_CRIT, "Failed to connect to netlink socket. Exiting.");
        exit(EXIT_FAILURE);
    }
    syslog(LOG_INFO, "Netlink socket connected successfully.");

    rc = set_proc_ev_listen(nl_sock, true);
    if (rc == -1) {
        syslog(LOG_CRIT, "Failed to set proc event listener. Exiting.");
        rc = EXIT_FAILURE;
        goto out;
    }
    syslog(LOG_INFO, "Now listening for process events.");

    rc = handle_proc_ev(nl_sock, ml_inference_obj);
    if (rc == -1) {
        rc = EXIT_FAILURE;
        goto out;
    }

    set_proc_ev_listen(nl_sock, false);

out:
    close(nl_sock);
    {
        std::unique_lock<std::mutex> lock(queue_mutex);
        need_exit = true;
    }
    queue_cond.notify_all();
    for (std::thread &t : thread_pool) {
        t.join();
    }
    closelog(); // Close syslog
    exit(rc);
}
