// Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
// SPDX-License-Identifier: BSD-3-Clause-Clear

#include "Utils.h"
#include "Common.h"
#include "Request.h"
#include "Logger.h"
#include "AuxRoutines.h"
#include "ContextualClassifier.h"
#include "FeatureExtractor.h"
#include "PostProcess.h"
#include "FeaturePruner.h"
#include "RestuneInternal.h"
#include "AppConfigs.h"

#include <algorithm>
#include <chrono>
#include <cstdarg>
#include <cstdlib>
#include <cstring>
#include <dirent.h>
#include <fstream>
#include <iostream>
#include <pthread.h>
#include <sstream>
#include <string>
#include <syslog.h>

#define CLASSIFIER_TAG "ContextualClassifier"
#define CLASSIFIER_CONF_DIR "/etc/classifier/"

static pid_t ourPID = 0;
static pid_t ourTID = 0;

const std::string FT_MODEL_PATH =
    CLASSIFIER_CONF_DIR "fasttext_model_supervised.bin";
const std::string IGNORE_PROC_PATH =
    CLASSIFIER_CONF_DIR "classifier-blocklist.txt";
const std::string IGNORE_TOKENS_PATH = CLASSIFIER_CONF_DIR "ignore-tokens.txt";

static std::string format_string(const char *fmt, ...) {
    char buffer[1024];
    va_list args;
    va_start(args, fmt);
    vsnprintf(buffer, sizeof(buffer), fmt, args);
    va_end(args);
    return std::string(buffer);
}

// Helper to check if a string contains only digits
static bool is_digits(const std::string& str) {
    return std::all_of(str.begin(), str.end(), ::isdigit);
}

// Function to get the first matching PID for a given process name
static pid_t getProcessPID_COMM(const std::string& process_name) {
    DIR* proc_dir = opendir("/proc");
    if (!proc_dir) {
        std::cerr << "Failed to open /proc directory." << std::endl;
        return -1;
    }

    struct dirent* entry;
    while ((entry = readdir(proc_dir)) != nullptr) {
        if (entry->d_type == DT_DIR && is_digits(entry->d_name)) {
            std::string pid_str = entry->d_name;
            std::string comm_path = "/proc/" + pid_str + "/comm";
            std::ifstream comm_file(comm_path);
            std::string comm;
            if (comm_file) {
                std::getline(comm_file, comm);
                if (comm.find(process_name) != std::string::npos) {
                    closedir(proc_dir);
                    return static_cast<pid_t>(std::stoi(pid_str));
                }
            }
        }
    }

    closedir(proc_dir);
    return -1; // Not found
}

static void moveAppThreadsToCGroup(AppConfig* appConfig) {
    try {
        Request* request = MPLACED(Request);
        request->setRequestType(REQ_RESOURCE_TUNING);
        request->setHandle(AuxRoutines::generateUniqueHandle());
        request->setDuration(-1);
        request->setProperties(RequestPriority::REQ_PRIORITY_HIGH);
        request->setClientPID(ourPID);
        request->setClientTID(ourTID);

        if(appConfig->mThreadNameList != nullptr) {
            int32_t numThreads = appConfig->mNumThreads;
            // Go over the list of proc names (comm) and get their pids
            for(int32_t i = 0; i < numThreads; i++) {
                std::string targetComm = appConfig->mThreadNameList[i];
                pid_t targetPID = getProcessPID_COMM(targetComm);
                if(targetPID != -1) {
                    // Get the CGroup
                    int32_t currCGroupID = appConfig->mCGroupIds[i];
                    // Make the move via Resource Tuner APIs

                    Resource* resource = MPLACEV(Resource);
                    resource->setResCode(RES_CGRP_MOVE_PID);
                    resource->setNumValues(2);
                    resource->setValueAt(0, currCGroupID);
                    resource->setValueAt(1, targetPID);

                    ResIterable* resIterable = MPLACED(ResIterable);
                    resIterable->mData = resource;
                    request->addResource(resIterable);
                }
            }
        }

        // Anything to issue
        if(request->getResourcesCount() > 0) {
            // fast path to Request Queue
            submitResProvisionRequest(request, true);
        } else {
            Request::cleanUpRequest(request);
        }

    } catch(const std::exception& e) {
        LOGE("CLASSIFIER",
             "Failed to move per-app threads to cgroup, Error: " + std::string(e.what()));
    }
}

ContextualClassifier::ContextualClassifier() : mMLInference(FT_MODEL_PATH) {}

ContextualClassifier::~ContextualClassifier() { Terminate(); }

void ContextualClassifier::LoadIgnoredProcesses() {
    std::ifstream file(IGNORE_PROC_PATH);
    if (!file.is_open()) {
        LOGW(CLASSIFIER_TAG,
             format_string("Could not open ignore process file: %s",
                           IGNORE_PROC_PATH.c_str()));
        return;
    }
    std::string line;
    while (std::getline(file, line)) {
        std::stringstream ss(line);
        std::string segment;
        while (std::getline(ss, segment, ',')) {
            size_t first = segment.find_first_not_of(" \t\n\r");
            if (first == std::string::npos)
                continue;
            size_t last = segment.find_last_not_of(" \t\n\r");
            segment = segment.substr(first, (last - first + 1));
            if (!segment.empty()) {
                mIgnoredProcesses.insert(segment);
            }
        }
    }
    LOGI(CLASSIFIER_TAG, format_string("Loaded %zu ignored processes.",
                                       mIgnoredProcesses.size()));
}

int FetchComm(int pid, std::string &comm) {
    std::string proc_path = "/proc/" + std::to_string(pid);
    if (access(proc_path.c_str(), F_OK) == -1) {
        LOGD(CLASSIFIER_TAG, format_string("Process %d has exited.", pid));
        return -1;
    }

    std::string comm_path = "/proc/" + std::to_string(pid) + "/comm";
    std::ifstream comm_file(comm_path);
    if (comm_file.is_open()) {
        std::getline(comm_file, comm);
        // Trim
        size_t first = comm.find_first_not_of(" \t\n\r");
        if (first != std::string::npos) {
            size_t last = comm.find_last_not_of(" \t\n\r");
            comm = comm.substr(first, (last - first + 1));
        }
    }
    return 0;
}

bool ContextualClassifier::isIgnoredProcess(int evType, int pid) {
    bool ignore = false;

    // For context close, see if pid is in ignored list and remove it.
    if (evType == CC_APP_CLOSE) {
        auto it = mIgnoredPids.find(pid);
        if (it != mIgnoredPids.end()) {
            mIgnoredPids.erase(it);
            ignore = true;
        }
        return ignore;
    }

    // For context open, check if comm is in ignored list and track pid.
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
        if (mIgnoredProcesses.count(proc_name) != 0U) {
            LOGD(CLASSIFIER_TAG, format_string("Ignoring process: %s (PID: %d)",
                                               proc_name.c_str(), pid));
            mIgnoredPids.insert(pid);
            ignore = true;
        }
    } else {
        LOGD(CLASSIFIER_TAG,
             format_string("Process %d exited before initial check. Skipping.",
                           pid));
    }

    return ignore;
}

void ContextualClassifier::GetSignalDetailsForWorkload(int32_t contextType,
                                                       uint32_t &sigId,
                                                       uint32_t &sigSubtype) {
    switch (contextType) {
    case CC_MULTIMEDIA:
        sigId = CC_MULTIMEDIA_APP_OPEN;
        break;
    case CC_GAME:
        sigId = CC_GAME_APP_OPEN;
        break;
    case CC_BROWSER:
        sigId = CC_BROWSER_APP_OPEN;
        break;
    }
    return;
}

ErrCode ContextualClassifier::Init() {
    LOGI(CLASSIFIER_TAG, "Classifier module init.");

    LoadIgnoredProcesses();

    std::vector<std::string> labels = {"attr", "cgroup",  "cmdline",
                                       "comm", "environ", "exe",
                                       "logs", "fds",     "map_files"};
    mTokenIgnoreMap = FeaturePruner::loadIgnoreMap(IGNORE_TOKENS_PATH, labels);
    LOGI(CLASSIFIER_TAG, "Loaded ignore tokens configuration.");

    // Single worker thread for classification
    mClassifierMain = std::thread(&ContextualClassifier::ClassifierMain, this);

    if (mNetLinkComm.connect() == -1) {
        LOGE(CLASSIFIER_TAG, "Failed to connect to netlink socket.");
        return RC_SOCKET_OP_FAILURE;
    }

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
        // Clear any pending PIDs so the worker doesn't see stale entries
        while (!mPendingEv.empty()) {
            mPendingEv.pop();
        }
    }
    mQueueCond.notify_all();

    mNetLinkComm.close_socket();

    if (mNetlinkThread.joinable()) {
        mNetlinkThread.join();
    }

    if (mClassifierMain.joinable()) {
        mClassifierMain.join();
    }

    return RC_SUCCESS;
}

void ContextualClassifier::ClassifierMain() {
    pthread_setname_np(pthread_self(), "uRMClassifierMain");
    while (true) {
        ProcEvent ev{};
        {
            std::unique_lock<std::mutex> lock(mQueueMutex);
            mQueueCond.wait(
                lock, [this] { return !mPendingEv.empty() || mNeedExit; });

            if (mNeedExit) {
                return;
            }

            ev = mPendingEv.front();
            mPendingEv.pop();
        }

        if (ev.type == CC_APP_OPEN) {
            std::string comm;
            uint32_t sigId = CC_APP_OPEN;
            uint32_t sigSubtype = DEFAULT_CONFIG;
            uint32_t ctxDetails = 0U;

            if (ev.pid != -1) {
                if (FetchComm(ev.pid, comm) != 0) {
                    continue;
                }

                int contextType =
                    ClassifyProcess(ev.pid, ev.tgid, comm, ctxDetails);

                // Step 1: Move the process to focused-cgroup
                // Also involves removing the process already there from focused.

                // Code ........


                // Step 2: Move the "threads" from per-app config to appropriate cgroups

                // Code for determining per-app config threads
                AppConfig* appConfig =
                    AppConfigs::getInstance()->getAppConfig(comm);

                if(appConfig != nullptr) {
                    int32_t threadsCount = appConfig->mNumThreads;
                    if(threadsCount > 0) {
                        moveAppThreadsToCGroup(appConfig);
                    }
                }

                // Step 3: Identify if any signal configuration exists
                // Will return the sigID based on the workload
                // For example: game, browser, multimedia
                GetSignalDetailsForWorkload(contextType, sigId, sigSubtype);

                // By Default we can acquire the generic signal (subtype) associate with that ID

                // If the post processing block exists, call it
                // It might provide us a more specific sigSubtype

                PostProcessingCallback postCb =
                    Extensions::getPostProcessingCallback(comm);
                if(postCb) {
                    PostProcessCBData postProcessData = {
                        .mCmdline = comm,
                        .mSigId = sigId,
                        .mSigSubtype = sigSubtype,
                    };
                    postCb((void*)&postProcessData);

                    sigId = postProcessData.mSigId;
                    sigSubtype = postProcessData.mSigSubtype;
                }

                // Details are available, call tuneSignal
                ApplyActions(comm, sigId, sigSubtype);
            }
        } else if (ev.type == CC_APP_CLOSE) {
            RemoveActions(ev.pid, ev.tgid);
        }
    }
}

int ContextualClassifier::ClassifyProcess(int process_pid, int process_tgid,
                                          const std::string &comm,
                                          uint32_t &ctxDetails) {
    (void)process_tgid;
    (void)ctxDetails;

    if (mIgnoredProcesses.count(comm) != 0U) {
        LOGD(CLASSIFIER_TAG,
             format_string(
                 "Skipping inference for ignored process: %s (PID: %d)",
                 comm.c_str(), process_pid));
        return 0;
    }

    LOGD(CLASSIFIER_TAG,
         format_string("Starting classification for PID:%d", process_pid));

    std::map<std::string, std::string> raw_data;

    const std::string proc_path = "/proc/" + std::to_string(process_pid);

    auto start_collect = std::chrono::high_resolution_clock::now();
    int collect_rc = FeatureExtractor::CollectAndStoreData(
        process_pid, mTokenIgnoreMap, raw_data, mDebugMode);
    auto end_collect = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double, std::milli> elapsed_collect =
        end_collect - start_collect;
    LOGD(CLASSIFIER_TAG,
         format_string("Data collection for PID:%d took %f ms (rc=%d)",
                       process_pid, elapsed_collect.count(), collect_rc));

    if (collect_rc != 0) {
        // Process exited or collection failed; skip further work.
        return 0;
    }

    LOGD(CLASSIFIER_TAG,
         format_string("Text features collected for PID:%d", process_pid));

    if (access(proc_path.c_str(), F_OK) == -1) {
        return 0;
    }

    bool has_sufficient_features = false;
    for (const auto &kv : raw_data) {
        if (!kv.second.empty()) {
            has_sufficient_features = true;
            break;
        }
    }

    // Default context type UNKNOWN.
    CC_TYPE contextType = CC_UNKNOWN;

    if (has_sufficient_features) {
        if (access(proc_path.c_str(), F_OK) == -1) {
            return static_cast<int>(contextType);
        }

        LOGD(CLASSIFIER_TAG,
             format_string("Invoking ML inference for PID:%d", process_pid));

        auto start_inference = std::chrono::high_resolution_clock::now();
        std::string predicted_label =
            mMLInference.predict(process_pid, raw_data);
        auto end_inference = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double, std::milli> elapsed_inference =
            end_inference - start_inference;
        LOGD(CLASSIFIER_TAG,
             format_string("Inference for PID:%d took %f ms", process_pid,
                           elapsed_inference.count()));

        // Map stripped label -> CC_APP enum.
        // MLInference::predict() returns after stripping "__label__".
        if (predicted_label == "app") {
            contextType = CC_APP;
        } else if (predicted_label == "browser") {
            contextType = CC_BROWSER;
        } else if (predicted_label == "game") {
            contextType = CC_GAME;
        } else if (predicted_label == "media") {
            contextType = CC_MULTIMEDIA;
        } else {
            contextType = CC_UNKNOWN;
        }

        LOGD(CLASSIFIER_TAG,
             format_string("Predicted label '%s' mapped to contextType=%d",
                           predicted_label.c_str(),
                           static_cast<int>(contextType)));
    } else {
        LOGD(CLASSIFIER_TAG,
             format_string("Skipping ML inference for PID:%d due to "
                           "insufficient features.",
                           process_pid));
    }

    return static_cast<int>(contextType);
}

void ContextualClassifier::ApplyActions(std::string comm, int32_t sigId,
                                        int32_t sigType) {
    // tuneSignal and update the handles
    // mResTunerHandles
    return;
}

void ContextualClassifier::RemoveActions(int process_pid, int process_tgid) {
    (void)process_tgid;
    // untuneSignal and erase handles
    // mResTunerHandles
    return;
}

int ContextualClassifier::HandleProcEv() {
    pthread_setname_np(pthread_self(), "ClassNetlink");
    int rc = 0;

    while (!mNeedExit) {
        ProcEvent ev{};
        rc = mNetLinkComm.RecvEvent(ev);
        if (rc == CC_IGNORE) {
            continue;
        } else if (rc == -1) {
            if (errno == EINTR) {
                continue;
            }
            if (mNeedExit) {
                return 0;
            }
            LOGE(CLASSIFIER_TAG,
                 format_string("netlink recv: %s", strerror(errno)));
            return -1;
        }

        switch (rc) {
        case CC_APP_OPEN:
            LOGD(CLASSIFIER_TAG,
                 format_string("Received CC_APP_OPEN for tid=%d pid=%d", ev.pid,
                               ev.tgid));
            if (isIgnoredProcess(ev.type, ev.pid)) {
                break;
            }
            {
                std::lock_guard<std::mutex> lock(mQueueMutex);
                mPendingEv.push(ev);
                mQueueCond.notify_one();
            }
            break;
        case CC_APP_CLOSE:
            LOGD(CLASSIFIER_TAG,
                 format_string("Received CC_APP_CLOSE pid=%d tgid=%d", ev.pid,
                               ev.tgid));
            if (isIgnoredProcess(ev.type, ev.pid)) {
                break;
            }
            {
                std::lock_guard<std::mutex> lock(mQueueMutex);
                mPendingEv.push(ev);
                mQueueCond.notify_one();
            }
            break;
        default:
            LOGW(CLASSIFIER_TAG, "unhandled proc event");
            break;
        }
    }
    return 0;
}

// Global instance
static ContextualClassifier *g_classifier = nullptr;

static ErrCode init(void *arg = nullptr) {
    (void)arg;

    // Record PID and TID
    ourPID = getpid();
    ourTID = gettid();

    if (!g_classifier) {
        g_classifier = new ContextualClassifier();
    }
    return g_classifier->Init();
}

static ErrCode terminate(void *arg = nullptr) {
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
