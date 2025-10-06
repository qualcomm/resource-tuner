// Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
// SPDX-License-Identifier: BSD-3-Clause-Clear

#include <iostream>
#include <thread>
#include <systemd/sd-bus.h>
#include <systemd/sd-event.h>

#include "Logger.h"
#include "ComponentRegistry.h"

#define DBUS_SIGNAL_SENDER "org.freedesktop.login1"
#define DBUS_SIGNAL_SENDER_PATH "/org/freedesktop/login1"
#define DBUS_SIGNAL_INTERFACE "org.freedesktop.login1.Manager"
#define DBUS_SIGNAL_NAME "PrepareForSleep"

static sd_bus* bus = nullptr;
static sd_bus_slot* slot = nullptr;
static sd_event* event = nullptr;

static std::thread eventTrackerThread;

int32_t onSdBusMessageReceived(sd_bus_message* message,
                               void *userdata,
                               sd_bus_error* retError) {
    std::cout<<"sd-bus signal received"<<std::endl;
    return 0;
}

static void initHelper() {
    // Connect to the system bus
    if(sd_bus_default_system(&bus) < 0) {
        LOGE("RESTUNE_DISPLAY_AWARE_OPS", "Failed to establish connection with system bus");
        return;
    }

    // Create the Event Loop object
    if(sd_event_default(&event) < 0) {
        LOGE("RESTUNE_DISPLAY_AWARE_OPS", "Failed to create event-loop");
        return;
    }

    // Register for UNIX signals (to control event loop termination)
    if(sd_event_add_signal(event, nullptr, SIGINT, nullptr, nullptr) < 0) {
        LOGE("RESTUNE_DISPLAY_AWARE_OPS", "Failed to Attach SIGINT handler");
        return;
    }

    if(sd_event_add_signal(event, nullptr, SIGTERM, nullptr, nullptr) < 0) {
        LOGE("RESTUNE_DISPLAY_AWARE_OPS", "Failed to Attach SIGTERM handler");
        return;
    }

    // Subscribe to D-Bus signal (PrepareForSleep)
    if(sd_bus_match_signal(bus,
                           &slot,
                           DBUS_SIGNAL_SENDER,
                           DBUS_SIGNAL_SENDER_PATH,
                           DBUS_SIGNAL_INTERFACE,
                           DBUS_SIGNAL_NAME,
                           onSdBusMessageReceived,
                           nullptr) < 0) {
        LOGE("RESTUNE_DISPLAY_AWARE_OPS", "Failed to subscribe to PrepareForSleep (D-Bus) signal");
        return;
    }

    // Listen for D-Bus events
    if(sd_bus_attach_event(bus, event, 0) < 0) {
        LOGE("RESTUNE_DISPLAY_AWARE_OPS", "Failed to start event-loop");
        return;
    }

    // Start the Event Loop
    if(sd_event_loop(event) < 0) {
        LOGE("RESTUNE_DISPLAY_AWARE_OPS", "Failed to start event-loop");
        return;
    }
}

static ErrCode init(void* arg) {
    eventTrackerThread = std::thread(initHelper);
    return RC_SUCCESS;
}

static ErrCode tear(void* arg) {
    if(eventTrackerThread.joinable()) {
        eventTrackerThread.join();
    }

    sd_event_unref(event);
    sd_bus_slot_unref(slot);
    sd_bus_unref(bus);

    event = nullptr;
    slot = nullptr;
    bus = nullptr;
    return RC_SUCCESS;
}

RESTUNE_REGISTER_MODULE(MOD_DISPLAY_DETECTOR, init, tear, nullptr);
