// Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
// SPDX-License-Identifier: BSD-3-Clause-Clear

#ifndef BINDER_SERVER_H
#define BINDER_SERVER_H

#include <aidl/vendor/qti/hardware/restune/BnRestune.h>

namespace aidl {
namespace vendor {
namespace qti {
namespace hardware {
namespace restune {

class Restune : public BnRestune {
public:
    ndk::ScopedAStatus tuneResources(int64_t duration, int32_t prop, int32_t numRes, SysResource* resourceList) override;
    ndk::ScopedAStatus retuneResources(int64_t handle, int64_t duration) override;
    ndk::ScopedAStatus untuneResources(int64_t handle) override;

    ndk::ScopedAStatus getProp(const char* prop, char* buffer, size_t bufferSize, const char* defValue) override;

    ndk::ScopedAStatus tuneSignal(uint32_t signalCode, int64_t duration, int32_t properties,
                                  const char* appName, const char* scenario, int32_t numArgs, uint32_t* list) override;
    ndk::ScopedAStatus relaySignal(uint32_t signalCode, int64_t duration, int32_t properties,
                                   const char* appName, const char* scenario, int32_t numArgs, uint32_t* list) override;
    ndk::ScopedAStatus untuneSignal(int64_t handle) override;
};

}
}
}
}
}

#endif
