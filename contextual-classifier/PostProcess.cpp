#include "PostProcess.h"

static void gstCamPostProcess(void* context) {
    if(context == nullptr) {
        return;
    }

    // PostProcessCBData* postProcessInfo = static_cast<PostProcessCBData*>(context);
    // std::string cmdLine = postProcessInfo->mCmdline;

    // Start Parsing

    // Update: mSigId or mSigSubtype in the same struct, if needed
}

// CLASSIFIER_REGISTER_POST_PROCESS_CB("gst-camera-per", gstCamPostProcess);
