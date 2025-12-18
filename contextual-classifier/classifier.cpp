// Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
// SPDX-License-Identifier: BSD-3-Clause-Clear

#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#include <sys/socket.h>
#include <linux/netlink.h>
#include <linux/connector.h>
#include <linux/cn_proc.h>
#include <sys/stat.h>
#include <sys/types.h>
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
#include <syslog.h>
#include <iostream>
#include <sstream>
#include <chrono>
#include <pthread.h>
#include <cstdarg>

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
#include "ml_inference.h"

#include "ComponentRegistry.h"
#include "Utils.h"
#include "Logger.h"

#define CLASSIFIER_TAG "Classifier"

static std::string format_string(const char* fmt, ...) {
    char buffer[1024];
    va_list args;
    va_start(args, fmt);
    vsnprintf(buffer, sizeof(buffer), fmt, args);
    va_end(args);
    return std::string(buffer);
}

// Define a local version of cn_msg without the flexible array member 'data[]'
// to allow embedding it in other structures.
struct cn_msg_hdr {
    struct cb_id id;
    __u32 seq;
    __u32 ack;
    __u16 len;
    __u16 flags;
};

#define CLASSIFIER_CONF_DIR "/etc/classifier/"

// Thread pool and job queue
std::queue<int> classification_queue;
std::mutex queue_mutex;
std::condition_variable queue_cond;
std::vector<std::thread> thread_pool;
const int NUM_THREADS = 4;

// Netlink handling
int global_nl_sock = -1;
std::thread netlink_thread;
static volatile bool need_exit = false;

// Define paths to ML artifacts
const std::string FT_MODEL_PATH = CLASSIFIER_CONF_DIR "fasttext_model_supervised.bin";
const std::string IGNORE_PROC_PATH = CLASSIFIER_CONF_DIR "classifier-blocklist.txt";
const std::string IGNORE_TOKENS_PATH = CLASSIFIER_CONF_DIR "ignore-tokens.txt";

std::unordered_set<std::string> ignored_processes;
std::unordered_map<std::string, std::unordered_set<std::string>> g_token_ignore_map;
bool g_debug_mode = false;

void load_ignored_processes() {
    std::ifstream file(IGNORE_PROC_PATH);
    if (!file.is_open()) {
        LOGW(CLASSIFIER_TAG, format_string("Could not open ignore process file: %s", IGNORE_PROC_PATH.c_str()));
        return;
    }
    std::string line;
    while (std::getline(file, line)) {
        std::stringstream ss(line);
        std::string segment;
        while(std::getline(ss, segment, ',')) {
            // Trim whitespace
            size_t first = segment.find_first_not_of(" \t\n\r");
            if (first == std::string::npos) continue;
            size_t last = segment.find_last_not_of(" \t\n\r");
            segment = segment.substr(first, (last - first + 1));
            if (!segment.empty()) {
                ignored_processes.insert(segment);
            }
        }
    }
    LOGI(CLASSIFIER_TAG, format_string("Loaded %zu ignored processes.", ignored_processes.size()));
}

// Singleton for MLInference
MLInference& get_ml_inference_instance() {
    static MLInference ml_inference_obj(FT_MODEL_PATH);
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
        LOGD(CLASSIFIER_TAG, format_string("Process %d has exited.", pid));
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
        LOGE(CLASSIFIER_TAG, format_string("socket: %s", strerror(errno)));
        return -1;
    }

    sa_nl.nl_family = AF_NETLINK;
    sa_nl.nl_groups = CN_IDX_PROC;
    sa_nl.nl_pid = getpid();

    rc = bind(nl_sock, (struct sockaddr *)&sa_nl, sizeof(sa_nl));
    if (rc == -1) {
        LOGE(CLASSIFIER_TAG, format_string("bind: %s", strerror(errno)));
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
            struct cn_msg_hdr cn_msg;
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
        LOGE(CLASSIFIER_TAG, format_string("netlink send: %s", strerror(errno)));
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
    (void)process_tgid; // unused
}

/* Process classfication based on selinux context of process
 * TODO: How to create or use cgroups based on process creation.
 * TODO: Apply utilization limit on process groups.
 */
static void classify_process(int process_pid, int process_tgid,
                             std::unordered_map <int, int> &pid_perf_handle,
                             MLInference& ml_inference_obj)
{
    (void)process_tgid; // unused
    (void)pid_perf_handle; // unused
    
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
            LOGD(CLASSIFIER_TAG, format_string("Skipping inference for ignored process: %s (PID: %d)", proc_name.c_str(), process_pid));
            return;
        }
    }

    LOGD(CLASSIFIER_TAG, format_string("Starting classification for PID:%d", process_pid));

    std::map<std::string, std::string> raw_data;

    // Collect data using collect_and_store_data.
    // This performs collection, filtering, and optional CSV dumping in one go.
    auto start_collect = std::chrono::high_resolution_clock::now();
    collect_and_store_data(process_pid, g_token_ignore_map, raw_data, g_debug_mode);
    auto end_collect = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double, std::milli> elapsed_collect = end_collect - start_collect;
    LOGD(CLASSIFIER_TAG, format_string("Data collection for PID:%d took %f ms", process_pid, elapsed_collect.count()));

    LOGD(CLASSIFIER_TAG, format_string("Text features collected for PID:%d", process_pid));

    if (!is_process_alive(process_pid)) return;

    bool has_sufficient_features = false;
    // Check if we have any data collected
    for (const auto& kv : raw_data) {
        if (!kv.second.empty()) {
            has_sufficient_features = true;
            break;
        }
    }

    if (has_sufficient_features) {
        if (!is_process_alive(process_pid)) return;

        LOGD(CLASSIFIER_TAG, format_string("Invoking ML inference for PID:%d", process_pid));

        auto start_inference = std::chrono::high_resolution_clock::now();
        std::string predicted_label = ml_inference_obj.predict(process_pid, raw_data);
        auto end_inference = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double, std::milli> elapsed_inference = end_inference - start_inference;
        LOGD(CLASSIFIER_TAG, format_string("Inference for PID:%d took %f ms", process_pid, elapsed_inference.count()));

        // LOGI(CLASSIFIER_TAG, format_string("PID:%d Classified as: %s", process_pid, predicted_label.c_str())); // Already logged in MLInference
        // TODO: Apply resource tuning based on predicted_label
    } else {
        LOGD(CLASSIFIER_TAG, format_string("Skipping ML inference for PID:%d due to insufficient features.", process_pid));
    }
}

void worker_thread() {
    pthread_setname_np(pthread_self(), "ClassWorker");
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
    (void)ml_inference_obj; // unused in this function
    int rc;
    struct __attribute__ ((aligned(NLMSG_ALIGNTO))) {
        struct nlmsghdr nl_hdr;
        struct __attribute__ ((__packed__)) {
            struct cn_msg_hdr cn_msg;
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
            // When socket is closed, we expect error
            if (need_exit) return 0;
            LOGE(CLASSIFIER_TAG, format_string("netlink recv: %s", strerror(errno)));
            return -1;
        }
        switch (nlcn_msg.proc_ev.what) {
            case PROC_EVENT_NONE:
                // LOGD(CLASSIFIER_TAG, "set mcast listen ok");
                break;
            case PROC_EVENT_FORK:
                LOGD(CLASSIFIER_TAG, format_string("fork: parent tid=%d pid=%d -> child tid=%d pid=%d",
                       nlcn_msg.proc_ev.event_data.fork.parent_pid,
                       nlcn_msg.proc_ev.event_data.fork.parent_tgid,
                       nlcn_msg.proc_ev.event_data.fork.child_pid,
                       nlcn_msg.proc_ev.event_data.fork.child_tgid));
                break;
            case PROC_EVENT_EXEC:
                LOGD(CLASSIFIER_TAG, format_string("Received PROC_EVENT_EXEC for tid=%d pid=%d",
                       nlcn_msg.proc_ev.event_data.exec.process_pid,
                       nlcn_msg.proc_ev.event_data.exec.process_tgid));

                // Early filtering of ignored processes
                {
                    int pid = nlcn_msg.proc_ev.event_data.exec.process_pid;
                    std::vector<std::string> comm_vec = parse_proc_comm(pid, "");
                    if (comm_vec.empty()) {
                        LOGD(CLASSIFIER_TAG, format_string("Process %d exited before initial check. Skipping.", pid));
                    } else {
                        std::string proc_name = comm_vec[0];
                        proc_name.erase(proc_name.find_last_not_of(" \n\r\t") + 1);
                        if (ignored_processes.count(proc_name)) {
                            LOGD(CLASSIFIER_TAG, format_string("Ignoring process: %s (PID: %d)", proc_name.c_str(), pid));
                        } else {
                            std::lock_guard<std::mutex> lock(queue_mutex);
                            classification_queue.push(pid);
                            queue_cond.notify_one();
                        }
                    }
                }
                break;
            case PROC_EVENT_UID:
                LOGD(CLASSIFIER_TAG, format_string("uid change: tid=%d pid=%d from %d to %d",
                       nlcn_msg.proc_ev.event_data.id.process_pid,
                       nlcn_msg.proc_ev.event_data.id.process_tgid,
                       nlcn_msg.proc_ev.event_data.id.r.ruid,
                       nlcn_msg.proc_ev.event_data.id.e.euid));
                break;
            case PROC_EVENT_GID:
                LOGD(CLASSIFIER_TAG, format_string("gid change: tid=%d pid=%d from %d to %d",
                       nlcn_msg.proc_ev.event_data.id.process_pid,
                       nlcn_msg.proc_ev.event_data.id.process_tgid,
                       nlcn_msg.proc_ev.event_data.id.r.rgid,
                       nlcn_msg.proc_ev.event_data.id.e.egid));
                break;
            case PROC_EVENT_EXIT:
                LOGD(CLASSIFIER_TAG, format_string("exit: tid=%d pid=%d exit_code=%d",
                       nlcn_msg.proc_ev.event_data.exit.process_pid,
                       nlcn_msg.proc_ev.event_data.exit.process_tgid,
                       nlcn_msg.proc_ev.event_data.exit.exit_code));
                remove_actions(nlcn_msg.proc_ev.event_data.exec.process_pid,
                               nlcn_msg.proc_ev.event_data.exec.process_tgid,
                               pid_perf_handle);
                break;
            default:
                LOGW(CLASSIFIER_TAG, "unhandled proc event");
                break;
        }
    }

    return 0;
}

static ErrCode init(void* arg=nullptr) {
    (void)arg; // unused
    ErrCode opStatus = RC_SUCCESS;
    
    LOGI(CLASSIFIER_TAG, "Classifier module init.");

    initialize();
    load_ignored_processes();
    
    // Load ignore tokens for filtering
    g_token_ignore_map = loadIgnoreMap(IGNORE_TOKENS_PATH);
    LOGI(CLASSIFIER_TAG, "Loaded ignore tokens configuration.");
    
    for (int i = 0; i < NUM_THREADS; ++i) {
        thread_pool.emplace_back(worker_thread);
    }
    
    // Get the MLInference singleton instance
    get_ml_inference_instance();
    LOGI(CLASSIFIER_TAG, "MLInference object initialized.");

    global_nl_sock = nl_connect();
    if (global_nl_sock == -1) {
        LOGE(CLASSIFIER_TAG, "Failed to connect to netlink socket.");
        return RC_SOCKET_OP_FAILURE;
    }
    LOGI(CLASSIFIER_TAG, "Netlink socket connected successfully.");

    if (set_proc_ev_listen(global_nl_sock, true) == -1) {
        LOGE(CLASSIFIER_TAG, "Failed to set proc event listener.");
        close(global_nl_sock);
        global_nl_sock = -1;
        return RC_SOCKET_OP_FAILURE;
    }
    LOGI(CLASSIFIER_TAG, "Now listening for process events.");
    
    // Start netlink handling thread
    netlink_thread = std::thread([](){
        pthread_setname_np(pthread_self(), "ClassNetlink");
        handle_proc_ev(global_nl_sock, get_ml_inference_instance());
    });

    return opStatus;
}


static ErrCode terminate(void* arg=nullptr) {
    (void)arg; // unused
    LOGI(CLASSIFIER_TAG, "Classifier module terminate.");
    
    // Attempt to disable events (might fail if socket closed)
    if (global_nl_sock != -1) {
        set_proc_ev_listen(global_nl_sock, false);
    }
    
    // Signal exit
    {
        std::unique_lock<std::mutex> lock(queue_mutex);
        need_exit = true;
    }
    queue_cond.notify_all();
    
    // Close socket to wake up recv
    if (global_nl_sock != -1) {
        close(global_nl_sock);
        global_nl_sock = -1;
    }
    
    if (netlink_thread.joinable()) {
        netlink_thread.join();
    }
    
    for (std::thread &t : thread_pool) {
        if (t.joinable()) t.join();
    }
    thread_pool.clear();
    
    // closelog();
    return RC_SUCCESS;
}

RESTUNE_REGISTER_MODULE(MOD_CLASSIFIER, init, terminate);
