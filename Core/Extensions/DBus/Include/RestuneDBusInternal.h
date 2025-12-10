// Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
// SPDX-License-Identifier: BSD-3-Clause-Clear

#ifndef RESTUNE_DBUS_INTERNAL_H
#define RESTUNE_DBUS_INTERNAL_H

#include <systemd/sd-bus.h>
#include <systemd/sd-event.h>

class RestuneSDBus {
private:
    sd_bus* mBus;

    RestuneSDBus();

public:
    ~RestuneSDBus();

    ErrCode stopService(const std::string& unitName);
    ErrCode startService(const std::string& unitName);
    ErrCode restartService(const std::string& unitName);

    // For custom use-cases, like StateDetector while still keeping
    // a single open connection to system-bus.
    sd_bus* getBus();
};

#endif
