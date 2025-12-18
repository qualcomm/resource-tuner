// Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
// SPDX-License-Identifier: BSD-3-Clause-Clear

#include "ContextualClassifier.h"
#include "FeatureExtractor.h"
#include "FeaturePruner.h"
#include "Logger.h"
#include "Utils.h"
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
#include <algorithm>

#define CLASSIFIER_TAG "ContextualClassifier"
#define CLASSIFIER_CONF_DIR "/etc/classifier/"

const std::string FT_MODEL_PATH = CLASSIFIER_CONF_DIR "fasttext_model_supervised.bin";
const std::string IGNORE_PROC_PATH = CLASSIFIER_CONF_DIR "classifier-blocklist.txt";
const std::string IGNORE_TOKENS_PATH = CLASSIFIER_CONF_DIR "ignore-tokens.txt";

static std::string format_string(const char* fmt, ...) {
    char buffer[1024];
    va_list args;
    va_start(args, fmt);
    vsnprintf(buffer, sizeof(buffer), fmt, args);
    va_end(args);
    return std::string(buffer);
}

ContextualClassifier::ContextualClassifier() : mMLInference(FT_MODEL_PATH) {
}

ContextualClassifier::~ContextualClassifier() {
    Terminate();
}

void ContextualClassifier::LoadIgnoredProcesses() {
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
            size_t first = segment.find_first_not_of(" \t\n\r");
            if (first == std::string::npos) continue;
            size_t last = segment.find_last_not_of(" \t\n\r");
            segment = segment.substr(first, (last - first + 1));
            if (!segment.empty()) {
                mIgnoredProcesses.insert(segment);
            }
        }
    }
    LOGI(CLASSIFIER_TAG, format_string("Loaded %zu ignored processes.", mIgnoredProcesses.size()));
}

ErrCode ContextualClassifier::Init() {
    LOGI(CLASSIFIER_TAG, "Classifier module init.");

    LoadIgnoredProcesses();
    
    std::vector<std::string> labels = {"attr", "cgroup", "cmdline", "comm", "environ", "exe", "logs", "fds", "map_files"};
    mTokenIgnoreMap = FeaturePruner::loadIgnoreMap(IGNORE_TOKENS_PATH, labels);
    LOGI(CLASSIFIER_TAG, "Loaded ignore tokens configuration.");
    
    // Single worker thread for classification
    mWorkerThread = std::thread(&ContextualClassifier::WorkerThread, this);
    
    LOGI(CLASSIFIER_TAG, "MLInference object initialized.");

    if (mNetLinkComm.connect() == -1) {
        LOGE(CLASSIFIER_TAG, "Failed to connect to netlink socket.");
        return RC_SOCKET_OP_FAILURE;
    }
    LOGI(CLASSIFIER_TAG, "Netlink socket connected successfully.");

    if (mNetLinkComm.set_listen(true) == -1) {
        LOGE(CLASSIFIER_TAG, "Failed to set proc event listener.");
        mNetLinkComm.close_socket();
        return RC_SOCKET_OP_FAILURE;
    }
    LOGI(CLASSIFIER_TAG, "Now listening for process events.");
    
    mNetlinkThread = std::thread(&ContextualClassifier::HandleProcEv, this);
    
    return RC_SUCCESS;
}

ErrCode ContextualClassifier::Terminate() {
    LOGI(CLASSIFIER_TAG, "Classifier module terminate.");
    
    if (mNetLinkComm.get_socket() != -1) {
        mNetLinkComm.set_listen(false);
    }
    
    {
        std::unique_lock<std::mutex> lock(mQueueMutex);
        mNeedExit = true;
    }
    mQueueCond.notify_all();
    
    mNetLinkComm.close_socket();
    
    if (mNetlinkThread.joinable()) {
        mNetlinkThread.join();
    }
    
    if (mWorkerThread.joinable()) {
        mWorkerThread.join();
    }
    
    return RC_SUCCESS;
}

void ContextualClassifier::WorkerThread() {
    pthread_setname_np(pthread_self(), "ClassWorker");
    while (true) {
        int pid_to_classify = -1;
        {
            std::unique_lock<std::mutex> lock(mQueueMutex);
            mQueueCond.wait(lock, [this]{
                return !mPendingPids.empty() || mNeedExit;
            });

            if (mNeedExit && mPendingPids.empty()) {
                return;
            }

            // Take one PID from the pending set
            auto it = mPendingPids.begin();
            if (it != mPendingPids.end()) {
                pid_to_classify = *it;
                mPendingPids.erase(it);
            }
        }

        if (pid_to_classify != -1) {
            std::unordered_map<int, int> pid_perf_handle;
            ClassifyProcess(pid_to_classify, 0, pid_perf_handle);
        }
    }
}

void ContextualClassifier::ClassifyProcess(int process_pid, int process_tgid, std::unordered_map<int, int>& pid_perf_handle) {
    (void)process_tgid;
    (void)pid_perf_handle;
    
    std::string proc_path = "/proc/" + std::to_string(process_pid);
    if (access(proc_path.c_str(), F_OK) == -1) {
        LOGD(CLASSIFIER_TAG, format_string("Process %d has exited.", process_pid));
        return;
    }

    std::string comm_path = "/proc/" + std::to_string(process_pid) + "/comm";
    std::ifstream comm_file(comm_path);
    if (comm_file.is_open()) {
        std::string proc_name;
        std::getline(comm_file, proc_name);
        // Trim
        size_t first = proc_name.find_first_not_of(" \t\n\r");
        if (first != std::string::npos) {
            size_t last = proc_name.find_last_not_of(" \t\n\r");
            proc_name = proc_name.substr(first, (last - first + 1));
        }
        
        if (mIgnoredProcesses.count(proc_name)) {
            LOGD(CLASSIFIER_TAG, format_string("Skipping inference for ignored process: %s (PID: %d)", proc_name.c_str(), process_pid));
            return;
        }
    } else {
        // Process might have exited if comm file not found
        LOGD(CLASSIFIER_TAG, format_string("Process %d exited before initial check. Skipping.", process_pid));
    }

    LOGD(CLASSIFIER_TAG, format_string("Starting classification for PID:%d", process_pid));

    std::map<std::string, std::string> raw_data;

    auto start_collect = std::chrono::high_resolution_clock::now();
    FeatureExtractor::collect_and_store_data(process_pid, mTokenIgnoreMap, raw_data, mDebugMode);
    auto end_collect = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double, std::milli> elapsed_collect = end_collect - start_collect;
    LOGD(CLASSIFIER_TAG, format_string("Data collection for PID:%d took %f ms", process_pid, elapsed_collect.count()));

    LOGD(CLASSIFIER_TAG, format_string("Text features collected for PID:%d", process_pid));

    if (access(proc_path.c_str(), F_OK) == -1) return;

    bool has_sufficient_features = false;
    for (const auto& kv : raw_data) {
        if (!kv.second.empty()) {
            has_sufficient_features = true;
            break;
        }
    }

    if (has_sufficient_features) {
        if (access(proc_path.c_str(), F_OK) == -1) return;

        LOGD(CLASSIFIER_TAG, format_string("Invoking ML inference for PID:%d", process_pid));

        auto start_inference = std::chrono::high_resolution_clock::now();
        std::string predicted_label = mMLInference.predict(process_pid, raw_data);
        auto end_inference = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double, std::milli> elapsed_inference = end_inference - start_inference;
        LOGD(CLASSIFIER_TAG, format_string("Inference for PID:%d took %f ms", process_pid, elapsed_inference.count()));
    } else {
        LOGD(CLASSIFIER_TAG, format_string("Skipping ML inference for PID:%d due to insufficient features.", process_pid));
    }
}

void ContextualClassifier::RemoveActions(int process_pid, int process_tgid, std::unordered_map<int, int>& pid_perf_handle) {
    (void)process_tgid;
    if (pid_perf_handle.find(process_pid) != pid_perf_handle.end()) {
        pid_perf_handle.erase(process_pid);
    }
}

int ContextualClassifier::HandleProcEv() {
    pthread_setname_np(pthread_self(), "ClassNetlink");
    int rc;
    struct __attribute__ ((aligned(NLMSG_ALIGNTO))) {
        struct nlmsghdr nl_hdr;
        struct __attribute__ ((__packed__)) {
            struct cn_msg_hdr cn_msg;
            struct proc_event proc_ev;
        };
    } nlcn_msg;
    std::unordered_map <int, int> pid_perf_handle {};

    int nl_sock = mNetLinkComm.get_socket();

    while (!mNeedExit) {
        rc = recv(nl_sock, &nlcn_msg, sizeof(nlcn_msg), 0);
        if (rc == 0) {
            return 0;
        } else if (rc == -1) {
            if (errno == EINTR) continue;
            if (mNeedExit) return 0;
            LOGE(CLASSIFIER_TAG, format_string("netlink recv: %s", strerror(errno)));
            return -1;
        }
        switch (nlcn_msg.proc_ev.what) {
            case PROC_EVENT_NONE:
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

                {
                    int pid = nlcn_msg.proc_ev.event_data.exec.process_pid;
                    // Simple check for ignore list (logic duplicated from ClassifyProcess but fast path)
                    std::string comm_path = "/proc/" + std::to_string(pid) + "/comm";
                    std::ifstream comm_file(comm_path);
                    if (comm_file.is_open()) {
                        std::string proc_name;
                        std::getline(comm_file, proc_name);
                        // Trim
                        size_t first = proc_name.find_first_not_of(" \t\n\r");
                        if (first != std::string::npos) {
                            size_t last = proc_name.find_last_not_of(" \t\n\r");
                            proc_name = proc_name.substr(first, (last - first + 1));
                        }
                        if (mIgnoredProcesses.count(proc_name)) {
                            LOGD(CLASSIFIER_TAG, format_string("Ignoring process: %s (PID: %d)", proc_name.c_str(), pid));
                        } else {
                            std::lock_guard<std::mutex> lock(mQueueMutex);
                            // Mark PID as pending and let worker pick it up
                            mPendingPids.insert(pid);
                            mQueueCond.notify_one();
                        }
                    } else {
                         // Process might have exited
                         LOGD(CLASSIFIER_TAG, format_string("Process %d exited before initial check. Skipping.", pid));
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

                {
                    int pid = nlcn_msg.proc_ev.event_data.exit.process_pid;
                    std::lock_guard<std::mutex> lock(mQueueMutex);
                    // If this PID was pending, drop it so the worker will skip it
                    mPendingPids.erase(pid);
                }

                RemoveActions(nlcn_msg.proc_ev.event_data.exec.process_pid,
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

// Global instance
static ContextualClassifier* g_classifier = nullptr;

static ErrCode init(void* arg=nullptr) {
    (void)arg;
    if (!g_classifier) {
        g_classifier = new ContextualClassifier();
    }
    return g_classifier->Init();
}

static ErrCode terminate(void* arg=nullptr) {
    (void)arg;
    if (g_classifier) {
        ErrCode rc = g_classifier->Terminate();
        delete g_classifier;
        g_classifier = nullptr;
        return rc;
    }
    return RC_SUCCESS;
}

RESTUNE_REGISTER_MODULE(MOD_CLASSIFIER, init, terminate);
