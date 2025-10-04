// Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
// SPDX-License-Identifier: BSD-3-Clause-Clear

#ifndef RESTUNE_H
#define RESTUNE_H

#include <aidl/vendor/qti/hardware/restune/BnRestune.h>

#include "Logger.h"
#include "AuxRoutines.h"
#include "ComponentRegistry.h"

namespace aidl {
namespace vendor {
namespace qti {
namespace hardware {
namespace restune {

class Restune : public BnRestune {
public:
    ndk::ScopedAStatus tuneResources(int64_t duration, int32_t prop, const std::vector<SysResource>& resourceList, int64_t* _aidl_return) override;
    ndk::ScopedAStatus retuneResources(int64_t handle, int64_t duration, int8_t* _aidl_return) override;
    ndk::ScopedAStatus untuneResources(int64_t handle, int8_t* _aidl_return) override;

    ndk::ScopedAStatus getProp(const std::string& propName, const std::string& defaultVal, std::string *_aidl_return) override;

    ndk::ScopedAStatus tuneSignal(int64_t signalCode, int64_t duration, int32_t properties,
                                  const std::string& appName, const std::string& scenario, const std::vector<int64_t>& list, int64_t* _aidl_return) override;
    ndk::ScopedAStatus relaySignal(int64_t signalCode, int64_t duration, int32_t properties,
                                   const std::string& appName, const std::string& scenario, const std::vector<int64_t>& list, int8_t* _aidl_return) override;
    ndk::ScopedAStatus untuneSignal(int64_t handle, int8_t* _aidl_return) override;
};

}
}
}
}
}

#endif
