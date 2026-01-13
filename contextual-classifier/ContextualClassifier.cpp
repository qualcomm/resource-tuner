// Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
// SPDX-License-Identifier: BSD-3-Clause-Clear

#include <algorithm>
#include <chrono>
#include <cstdarg>
#include <cstdlib>
#include <cstring>
#include <dirent.h>
#include <fstream>
#include <pthread.h>
#include <sstream>
#include <string>

#include "Utils.h"
#include "Common.h"
#include "Request.h"
#include "Logger.h"
#include "AuxRoutines.h"
#include "ContextualClassifier.h"
#include "RestuneInternal.h"
#include "Extensions.h"
#include "Inference.h"

#define CLASSIFIER_TAG "ContextualClassifier"
#define CLASSIFIER_CONF_DIR "/etc/classifier/"
#define CLASSIFIER_CONFIGS_PATH "/etc/urm/classifier/"

const std::string FT_MODEL_PATH =
    CLASSIFIER_CONF_DIR "fasttext_model_supervised.bin";
const std::string IGNORE_PROC_PATH =
    CLASSIFIER_CONFIGS_PATH "classifier-blocklist.txt";
const std::string IGNORE_TOKENS_PATH = CLASSIFIER_CONFIGS_PATH "ignore-tokens.txt";

#ifdef USE_FASTTEXT
#include "MLInference.h"
#include "FeatureExtractor.h"
#include "FeaturePruner.h"

//MLInference
Inference *ContextualClassifier::GetInferenceObject() {
	return (new MLInference(FT_MODEL_PATH));
}
#else

//inference
Inference *ContextualClassifier::GetInferenceObject() {
	return (new Inference(FT_MODEL_PATH));
}
#endif

ContextualClassifier::ContextualClassifier() {
    this->mRestuneHandle = -1;
    mInference = GetInferenceObject();
}

static ContextualClassifier *g_classifier = nullptr;
static const int32_t pendingQueueControlSize = 30;

ContextualClassifier::~ContextualClassifier() {
    Terminate();
    if (mInference) {
		delete mInference;
        mInference = NULL;
	}
}

ErrCode ContextualClassifier::Init() {
    LOGI(CLASSIFIER_TAG, "Classifier module init.");

    // Record PID and TID
    this->mOurPid = getpid();
    this->mOurTid = gettid();

    this->LoadIgnoredProcesses();

    // Single worker thread for classification
    this->mClassifierMain = std::thread(&ContextualClassifier::ClassifierMain, this);

    if (this->mNetLinkComm.connect() == -1) {
        LOGE(CLASSIFIER_TAG, "Failed to connect to netlink socket.");
        return RC_SOCKET_OP_FAILURE;
    }

    if (this->mNetLinkComm.set_listen(true) == -1) {
        LOGE(CLASSIFIER_TAG, "Failed to set proc event listener.");
        mNetLinkComm.close_socket();
        return RC_SOCKET_OP_FAILURE;
    }
    LOGI(CLASSIFIER_TAG, "Now listening for process events.");

    this->mNetlinkThread = std::thread(&ContextualClassifier::HandleProcEv, this);
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

            if(ev.pid != -1) {
                if(FetchComm(ev.pid, comm) != 0) {
                    continue;
                }

                // Step 1: Figure out workload type
                int contextType =
                    ClassifyProcess(ev.pid, ev.tgid, comm, ctxDetails);
				if (contextType == CC_IGNORE) {
					//ignore and wait for next event
					continue;
				}
                // Identify if any signal configuration exists
                // Will return the sigID based on the workload
                // For example: game, browser, multimedia
                this->GetSignalDetailsForWorkload(contextType, sigId, sigSubtype);

                // Step 2:
                // - Move the process to focused-cgroup, Also involves removing the process
                //  already there from the cgroup.
                // - Move the "threads" from per-app config to appropriate cgroups
                this->MoveAppThreadsToCGroup(ev.pid, comm, FOCUSED_CGROUP_IDENTIFIER);

                // Step 3: If the post processing block exists, call it
                // It might provide us a more specific sigSubtype
                PostProcessingCallback postCb =
                    Extensions::getPostProcessingCallback(comm);
                if(postCb != nullptr) {
                    PostProcessCBData postProcessData = {
                        .mPid = ev.pid,
                        .mSigId = sigId,
                        .mSigSubtype = sigSubtype,
                    };
                    postCb((void*)&postProcessData);

                    sigId = postProcessData.mSigId;
                    sigSubtype = postProcessData.mSigSubtype;
                }

                // Step 4: Apply actions, call tuneSignal
                // Skip
                // ApplyActions(comm, sigId, sigSubtype);
            }
        } else if (ev.type == CC_APP_CLOSE) {
			//Step1: move process to original cgroup

			//Step2: remove actions, call untune signal
            // RemoveActions(ev.pid, ev.tgid);
        }
    }
}

int ContextualClassifier::HandleProcEv() {
    pthread_setname_np(pthread_self(), "ClassNetlink");
    int32_t rc = 0;

    while(!mNeedExit) {
        ProcEvent ev{};
        rc = mNetLinkComm.RecvEvent(ev);
        if(rc == CC_IGNORE) {
            continue;
        }
        if(rc == -1) {
            if(errno == EINTR) {
                continue;
            }

            if(mNeedExit) {
                return 0;
            }

            // Error would have already been logged by this point.
            return -1;
        }

        // Process still up?
        if(!AuxRoutines::fileExists(COMM(ev.pid))) {
            continue;
        }

        switch(rc) {
            case CC_APP_OPEN:
                // TYPELOGV(NOTIFY_CLASSIFIER_PROC_EVENT, "CC_APP_OPEN", ev.pid);
                if(!this->isIgnoredProcess(ev.type, ev.pid)) {
                    const std::lock_guard<std::mutex> lock(mQueueMutex);
                    this->mPendingEv.push(ev);
                    if(this->mPendingEv.size() > pendingQueueControlSize) {
                        this->mPendingEv.pop();
                    }
                    this->mQueueCond.notify_one();
                } else {
                    // TYPELOGV(NOTIFY_CLASSIFIER_PROC_IGNORE, ev.pid);
                }

                break;

            case CC_APP_CLOSE:
                // TYPELOGV(NOTIFY_CLASSIFIER_PROC_EVENT, "CC_APP_CLOSE", ev.pid);
                if(!this->isIgnoredProcess(ev.type, ev.pid)) {
                    const std::lock_guard<std::mutex> lock(mQueueMutex);
                    this->mPendingEv.push(ev);
                    if(this->mPendingEv.size() > pendingQueueControlSize) {
                        this->mPendingEv.pop();
                    }
                    this->mQueueCond.notify_one();
                } else {
                    // TYPELOGV(NOTIFY_CLASSIFIER_PROC_IGNORE, ev.pid);
                }

                break;

            default:
                // log error??
                break;
        }
    }

    return 0;
}

int32_t ContextualClassifier::ClassifyProcess(pid_t process_pid, pid_t process_tgid,
                                              const std::string &comm,
                                              uint32_t &ctxDetails) {
    (void)process_tgid;
    (void)ctxDetails;
    CC_TYPE context = CC_APP;

    if(mIgnoredProcesses.count(comm) != 0U) {
        // LOGD(CLASSIFIER_TAG,
        //      "Skipping inference for ignored process: "+ comm);
        return CC_IGNORE;
    }

    // Check if the process is still alive
    if(!AuxRoutines::fileExists(COMM(process_pid))) {
        // LOGD(CLASSIFIER_TAG,
        //      "Skipping inference, process is dead: "+ comm);
        return CC_IGNORE;
    }

    // LOGD(CLASSIFIER_TAG,
        //  "Starting classification for PID: "+ std::to_string(process_pid));
    context = mInference->Classify(process_pid);
    return context;
}

void ContextualClassifier::ApplyActions(std::string comm, int32_t sigId,
                                        int32_t sigType) {
    (void)comm;
	(void)sigId;
	(void)sigType;
    // tuneSignal and update the handles
    // mResTunerHandles
    return;
}

void ContextualClassifier::RemoveActions(pid_t process_pid, pid_t process_tgid) {
	(void)process_pid;
    (void)process_tgid;
    // untuneSignal and erase handles
    // mResTunerHandles
    return;
}

void ContextualClassifier::GetSignalDetailsForWorkload(int32_t contextType,
                                                       uint32_t &sigId,
                                                       uint32_t &sigSubtype) {
    (void)sigSubtype;
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

void ContextualClassifier::LoadIgnoredProcesses() {
    std::ifstream file(IGNORE_PROC_PATH);
    if (!file.is_open()) {
        // LOGW(CLASSIFIER_TAG,
            //  "Could not open ignore process file: "+IGNORE_PROC_PATH);
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
    LOGI(CLASSIFIER_TAG, "Loaded ignored processes.");
}

bool ContextualClassifier::isIgnoredProcess(int32_t evType, pid_t pid) {
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
    std::string comm_path = COMM(pid);
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
            // LOGD(CLASSIFIER_TAG, "Ignoring process: "+proc_name);
            mIgnoredPids.insert(pid);
            ignore = true;
        }
    } else {
        // LOGD(CLASSIFIER_TAG,
            //  "Process " + std::to_string(pid) + " exited before initial check. Skipping.");
    }

    return ignore;
}

int32_t ContextualClassifier::FetchComm(pid_t pid, std::string &comm) {
    std::string proc_path = "/proc/" + std::to_string(pid);
    if(!AuxRoutines::fileExists(proc_path)) {
        // LOGD(CLASSIFIER_TAG, "Process %d has exited." + std::to_string(pid));
        return -1;
    }

    std::string comm_path = COMM(pid);
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

// Function to get the first matching PID for a given process name
pid_t ContextualClassifier::FetchPid(const std::string& process_name) {
    DIR* proc_dir = opendir("/proc");
    if(proc_dir == nullptr) {
        TYPELOGV(ERRNO_LOG, "opendir", strerror(errno));
        return -1;
    }

    struct dirent* entry;
    while ((entry = readdir(proc_dir)) != nullptr) {
        if (entry->d_type == DT_DIR && IsNumericString(entry->d_name)) {
            std::string pid_str = entry->d_name;
            std::string comm_path = COMM_S(pid_str);
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

// Helper to check if a string contains only digits
bool ContextualClassifier::IsNumericString(const std::string& str) {
    return std::all_of(str.begin(), str.end(), ::isdigit);
}

ResIterable* ContextualClassifier::createMovePidResource(int32_t cGroupdId, pid_t pid) {
    ResIterable* resIterable = MPLACED(ResIterable);
    Resource* resource = MPLACED(Resource);
    resource->setResCode(RES_CGRP_MOVE_PID);
    resource->setNumValues(2);
    resource->setValueAt(0, cGroupdId);
    resource->setValueAt(1, pid);

    resIterable->mData = resource;
    return resIterable;
}

void ContextualClassifier::MoveAppThreadsToCGroup(pid_t incomingPID,
                                                  const std::string& comm,
                                                  int32_t cgroupIdentifier) {
    (void)comm;
    try {
        // Check for any outstanding request, if found untune it.
        LOGE(CLASSIFIER_TAG, "enter MoveAppThreadsToCGroup");
        if(this->mRestuneHandle != -1) {
            LOGE("Creating untune request for handle = ", std::to_string(this->mRestuneHandle));
            Request* untuneRequest = MPLACED(Request);

            untuneRequest->setRequestType(REQ_RESOURCE_UNTUNING);
            untuneRequest->setHandle(this->mRestuneHandle);
            untuneRequest->setDuration(-1);
            // Passing priority as HIGH_TRANSFER_PRIORITY (= -1)
            // - Ensures untune requests are processed before even SERVER_HIGH priority tune requests
            //   which helps in freeing memory
            // - Since this level of priority is only used internally, hence it has been customized to
            //   not free up the underlying Request object, allowing for reuse.
            // Priority Level: -2 is used to force server termination and cleanup so should not be used otherwise.
            untuneRequest->setPriority(SYSTEM_HIGH);
            untuneRequest->setClientPID(mOurPid);
            untuneRequest->setClientTID(mOurTid);

            // fast path to Request Queue
            // Mark verification status as true. Request still goes through RequestManager though.
            LOGE("Issuing untune request for handle = ", std::to_string(this->mRestuneHandle));
            submitResProvisionRequest(untuneRequest, true);
            this->mRestuneHandle = -1;
            LOGE(CLASSIFIER_TAG, "Untune request issued");
        }

        // Issue a tune request for the new pid (and any associated app-config pids)
        LOGE(CLASSIFIER_TAG, "Starting tune request creation");
        Request* request = MPLACED(Request);
        request->setRequestType(REQ_RESOURCE_TUNING);
        // Generate and store the handle for future use
        this->mRestuneHandle = AuxRoutines::generateUniqueHandle();
        request->setHandle(this->mRestuneHandle);
        request->setDuration(-1);
        request->setPriority(SYSTEM_LOW);
        request->setClientPID(this->mOurPid);
        request->setClientTID(this->mOurTid);

        // Move the incoming pid
        LOGE(CLASSIFIER_TAG, "proceeding with reources creation");
        ResIterable* resIter = this->createMovePidResource(cgroupIdentifier, incomingPID);
        request->addResource(resIter);

        AppConfig* appConfig = AppConfigs::getInstance()->getAppConfig(comm);
        if(appConfig != nullptr && appConfig->mThreadNameList != nullptr) {
            int32_t numThreads = appConfig->mNumThreads;
            // Go over the list of proc names (comm) and get their pids
            for(int32_t i = 0; i < numThreads; i++) {
                std::string targetComm = appConfig->mThreadNameList[i];
                pid_t targetPID = this->FetchPid(targetComm);
                if(targetPID != -1 && targetPID != incomingPID) {
                    // Get the CGroup
                    int32_t currCGroupID = appConfig->mCGroupIds[i];
                    request->addResource(createMovePidResource(currCGroupID, targetPID));
                }
            }
        }

        // Anything to issue
        if(request->getResourcesCount() > 0) {
            // fast path to Request Queue
            submitResProvisionRequest(request, true);
        } else {
            Request::cleanUpRequest(request);
            this->mRestuneHandle = -1;
        }

    } catch(const std::exception& e) {
        LOGE(CLASSIFIER_TAG,
             "Failed to move per-app threads to cgroup, Error: " + std::string(e.what()));
        this->mRestuneHandle = -1;
    }
}

// Public C interface exported from the contextual-classifier shared library.
// These are what the URM module entrypoints will call.
extern "C" ErrCode cc_init(void) {
    if (!g_classifier) {
        g_classifier = new ContextualClassifier();
    }
    return g_classifier->Init();
}

extern "C" ErrCode cc_terminate(void) {
    if (g_classifier) {
        ErrCode rc = g_classifier->Terminate();
        delete g_classifier;
        g_classifier = nullptr;
        return rc;
    }
    return RC_SUCCESS;
}
