// Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
// SPDX-License-Identifier: BSD-3-Clause-Clear

#include <thread>

#include "RestuneDBusInternal.h"
#include "Logger.h"
#include "ComponentRegistry.h"
#include "ResourceTunerSettings.h"
#include "ServerInternal.h"

#define DBUS_SIGNAL_SENDER "org.freedesktop.login1"
#define DBUS_SIGNAL_SENDER_PATH "/org/freedesktop/login1"
#define DBUS_SIGNAL_INTERFACE "org.freedesktop.login1.Manager"
#define DBUS_SIGNAL_NAME "PrepareForSleep"

static sd_bus_slot* slot = nullptr;
static sd_event* event = nullptr;

static std::thread eventTrackerThread;

static void cleanup() {
    // Cleanup
    if(event != nullptr) {
        sd_event_unref(event);
    }

    if(slot != nullptr) {
        sd_bus_slot_unref(slot);
    }

    event = nullptr;
    slot = nullptr;
}

static int32_t onSdBusMessageReceived(sd_bus_message* message,
                                      void *userdata,
                                      sd_bus_error* retError) {
    int8_t sleepStatus;
    LOGI("RESTUNE_MODE_DETECTION", "sd-bus signal received");

    if(sd_bus_message_read(message, "b", &sleepStatus) < 0) {
        LOGE("RESTUNE_MODE_DETECTION", "Failed to parse Signal Parameter");
        return 0;
    }

    std::shared_ptr<RequestManager> requestManager = RequestManager::getInstance();
    if(sleepStatus) {
        // System is suspending
        LOGI("RESTUNE_MODE_DETECTION", "System is suspending");
        if(ResourceTunerSettings::targetConfigs.currMode & MODE_RESUME) {
            // Toggle to Display Off
            ResourceTunerSettings::targetConfigs.currMode &= ~MODE_RESUME;
            ResourceTunerSettings::targetConfigs.currMode |= MODE_SUSPEND;

            // First drain out the CocoTable, and move Requests to the Pending List
            // (the ones which cannot be processed in Background)
            requestManager->moveToPendingList();
        }
    } else {
        // System has resumed
        LOGI("RESTUNE_MODE_DETECTION", "System is resuming");
        if(ResourceTunerSettings::targetConfigs.currMode & MODE_SUSPEND) {
            // Toggle to Display On
            ResourceTunerSettings::targetConfigs.currMode &= ~MODE_SUSPEND;
            ResourceTunerSettings::targetConfigs.currMode |= MODE_RESUME;

            // Add all the Requests from the Pending List into the Active List
            std::vector<Request*> pendingRequests = requestManager->getPendingList();
            for(Request* request: pendingRequests) {
                submitResProvisionRequest(request, false);
            }
            requestManager->clearPending();
        }
    }

    return 0;
}

static int32_t eventLoopTerminator(sd_event_source *s,
                                   const struct signalfd_siginfo *si,
                                   void *userdata) {
    sd_event_exit(event, 0);
    return 0;
}

static void initHelper() {
    // Create the Event Loop object
    if(sd_event_default(&event) < 0) {
        LOGE("RESTUNE_DISPLAY_AWARE_OPS", "Failed to create event-loop");
        cleanup();
        return;
    }

    // Subscribe to D-Bus signal (PrepareForSleep)
    if(sd_bus_match_signal(RestuneDBusInternal::getInstance()->getBus(),
                           &slot,
                           nullptr,
                           DBUS_SIGNAL_SENDER_PATH,
                           DBUS_SIGNAL_INTERFACE,
                           DBUS_SIGNAL_NAME,
                           onSdBusMessageReceived,
                           nullptr) < 0) {
        LOGE("RESTUNE_DISPLAY_AWARE_OPS", "Failed to subscribe to PrepareForSleep (D-Bus) signal");
        cleanup();
        return;
    }

    // Listen for D-Bus events
    if(sd_bus_attach_event(RestuneDBusInternal::getInstance()->getBus(), event, 0) < 0) {
        LOGE("RESTUNE_DISPLAY_AWARE_OPS", "Failed to start event-loop");
        cleanup();
        return;
    }

    // Start the Event Loop
    if(sd_event_loop(event) < 0) {
        LOGE("RESTUNE_DISPLAY_AWARE_OPS", "Failed to start event-loop");
        cleanup();
        return;
    }

    cleanup();
}

// Register Module's Callback functions
static ErrCode init(void* arg) {
    try {
        eventTrackerThread = std::thread(initHelper);
    } catch(const std::system_error& e) {
        TYPELOGV(SYSTEM_THREAD_CREATION_FAILURE, "State_Optimizer", e.what());
        return RC_MODULE_INIT_FAILURE;
    }

    return RC_SUCCESS;
}

static ErrCode tear(void* arg) {
    if(eventTrackerThread.joinable()) {
        eventTrackerThread.join();
    }

    return RC_SUCCESS;
}

RESTUNE_REGISTER_MODULE(MOD_STATE_OPTIMIZER, init, tear, nullptr);
