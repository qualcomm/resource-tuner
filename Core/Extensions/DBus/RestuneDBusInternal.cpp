// Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
// SPDX-License-Identifier: BSD-3-Clause-Clear

#include "RestuneDBusInternal.h"

#define SYSTEMD_DBUS_NAME "org.freedesktop.systemd1"
#define SYSTEMD_DBUS_PATH "/org/freedesktop/systemd1"
#define SYSTEMD_DBUS_IF "org.freedesktop.systemd1.Manager"

RestuneSDBus::RestuneSDBus() {
    this->mBus = nullptr;

    // Connect to the system bus
    int32_t rc;
    if((rc = sd_bus_default_system(&this->mBus)) < 0) {
        TYPELOGV(SYSTEM_BUS_CONN_FAILED, strerror(-rc));
    }
}

ErrCode startService(const std::string& unitName) {
    if(this->mBus == nullptr) return RC_MODULE_INIT_FAILURE;

    // Start irqbalance
    int32_t rc = sd_bus_call_method(
        this->mBus,
        SYSTEMD_DBUS_NAME,
        SYSTEMD_DBUS_PATH,
        SYSTEMD_DBUS_IF,
        "StartUnit",
        &error,
        &reply,
        "ss",
        unitName.c_str(),
        "replace"
    );

    if(rc < 0) {
        LOGE("RESTUNE_COCO_TABLE", "Failed to start irqbalanced: " + std::string(error.message));
        return RC_DBUS_COMM_FAIL;
    }

    sd_bus_message_unref(reply);
    sd_bus_error_free(&error);

    return RC_SUCCESS;
}

ErrCode stopService(const std::string& unitName) {
    if(this->mBus == nullptr) return RC_DBUS_COMM_FAIL;

    rc = sd_bus_call_method(
        this->mBus,
        SYSTEMD_DBUS_NAME,
        SYSTEMD_DBUS_PATH,
        SYSTEMD_DBUS_IF,
        "StopUnit",
        &error,
        &reply,
        "ss",
        unitName.c_str(),
        "replace"
    );

    if(rc < 0) {
        LOGE("RESTUNE_COCO_TABLE", "Failed to stop irqbalanced: " + std::string(error.message));
        return RC_DBUS_COMM_FAIL;
    }

    sd_bus_message_unref(reply);
    sd_bus_error_free(&error);

    return RC_SUCCESS;
}

ErrCode restartService(const std::string& unitName) {
    if(RC_IS_NOTOK(stopService(unitName))) {
        return RC_DBUS_COMM_FAIL;
    }

    if(RC_IS_NOTOK(stopService(unitName))) {
        return RC_DBUS_COMM_FAIL;
    }

    return RC_SUCCESS;
}

sd_bus* RestuneSDBus::getBus() {
    return this->mBus;
}

RestuneSDBus::~RestuneSDBus() {
    if(this->mBus != nullptr) {
        sd_bus_unref(this->mBus);
        this->mBus = nullptr;
    }
}
