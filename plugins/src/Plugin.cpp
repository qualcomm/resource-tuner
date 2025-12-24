// Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
// SPDX-License-Identifier: BSD-3-Clause-Clear

#include "Utils.h"
#include "Extensions.h"

// What information to expose and how?
static void gstCamPostProcess(void* context) {
    if(context == nullptr) {
        return;
    }

    PostProcessCBData* postProcessInfo = static_cast<PostProcessCBData*>(context);
    std::string cmdLine = postProcessInfo->mCmdline;

    // Start Parsing

    // Update: mSigId or mSigSubtype in the same struct, if needed
}

// Register post processing block for gst-camera
CLASSIFIER_REGISTER_POST_PROCESS_CB("gst-camera-per", gstCamPostProcess);

// Any custom applier to tear callback can follow .....

__attribute__((constructor))
void registerWithUrm() {
    // Useful if the user wants to maintain app config in a separate location
    // For now we expect it will be placed in /etc/urm/custom/ directly
    // RESTUNE_REGISTER_CONFIG(APP_CONFIG, "/etc/urm/tests/configs/ResourcesConfig.yaml")
}
