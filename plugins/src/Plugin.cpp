// Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
// SPDX-License-Identifier: BSD-3-Clause-Clear

#include "Utils.h"
#include "Extensions.h"

Usecase fetchUsecaseDetails(pid_t pid, char *buf, size_t sz, uint32_t &sigId, uint32_t &sigType;) {
    int32_t encode = 0, decode = 0;
    int32_t height = 0;
    std::string target = "gst-camera-per";
    const char *e_str = "v4l2h264enc";
    const char *d_str = "v4l2h264dec";
    const char *qmm_str = "qtiqmmfsrc";
    const char *n_str = "name=";
    const char *h_str = "height=";
    char *e = buf;

    if((e = strstr(e, e_str)) != NULL) {
        encode += 1;
        sigId = URM_CAMERA_ENCODE;
        char *name = buf;
        if ((name = strstr(name, n_str)) != NULL) {
            name += strlen(n_str);
        }
        if (name == NULL) {
            name = "camsrc"
        }
        numSrc = count_threads_with_name(pid, name);
    }

    bool multi = checkProcessCommSubstring(process_pid, target);

    if ((numSrc > 1) || (multi)) {
        sigId = URM_CAMERA_ENCODE_MULTI_STREAMS;
        sigType = numSrc;
    }

    char *h = buf;
    size_t h_str_sz = strlen(h_str);
    h = strstr(h, h_str);
    if (h != NULL) {
        height = strtol(h + h_str_sz, NULL, 10);
    }

    char *d = buf;
    if ((d = strstr(d, d_str)) != NULL) {
        decode += 1;
        sigId = URM_VIDEO_DECODE;
        numSrc = 0;
        // count_threads_with_name(pid, d_str);
        sigType = numSrc;
    }

    /*Preview case*/
    if (encode == 0 && decode == 0) {
        char *d = buf;
        size_t d_str_sz = strlen(qmm_str);
        if ((d = strstr(d, qmm_str)) != NULL) {
            preview += 1;
            sigId = URM_CAMERA_PREVIEW;
        }
    }

    if (encode > 0 && decode > 0) {
        sigId = URM_ENCODE_DECODE;
    }

    return uc;
}

static void gstCamPostProcess(void* context) {
    if(context == nullptr) {
        return;
    }

    PostProcessCBData* postProcessInfo = static_cast<PostProcessCBData*>(context);
    std::string cmdLine = postProcessInfo->mCmdline;

    uint32_t sigID = postProcessInfo->mSigId;
    uint32_t sigType = postProcessInfo->mSigSubtype;

    char cmdline[1024];
    snprintf(cmdline, 1024, "/proc/%d/cmdline", postProcessInfo->mPid);
    FILE *fp = fopen(cmdline, "r");
    if (fp) {
        char *buf = NULL;
        size_t sz = 0;
        int len = 0;
        while ((len = getline(&buf, &sz, fp)) > 0) {
            sanitize_nulls(buf, len);
            enum USECASE type = fetchUsecaseDetails(postProcessInfo->mPid, buf, sz, sigID, sigType);
            if(type != UNDETERMINED) {
                // Update:
                postProcessInfo->mSigId = sigID;
                postProcessInfo->mSigSubtype = sigType;
                break;
            }
        }
    } else {
        printf("Failed to open file:%d\n", process_pid);
    }
}

// Register post processing block for gst-camera
CLASSIFIER_REGISTER_POST_PROCESS_CB("gst-camera-per", gstCamPostProcess);

__attribute__((constructor))
void registerWithUrm() {
    // Useful if the user wants to maintain app config in a separate location
    // For now we expect it will be placed in /etc/urm/custom/ directly
    // RESTUNE_REGISTER_CONFIG(APP_CONFIG, "/etc/urm/tests/configs/ResourcesConfig.yaml")
}
