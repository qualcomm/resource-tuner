// Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
// SPDX-License-Identifier: BSD-3-Clause-Clear

#include "ComponentRegistry.h"

std::unordered_map<ModuleID, ModuleInfo> ComponentRegistry::mModuleRegistry{};

ComponentRegistry::ComponentRegistry(
                               ModuleID moduleId,
                               EventCallback init,
                               EventCallback tear,
                               EventCallback onEvent) {
    mModuleRegistry[moduleId] = {
        .mInit = init,
        .mTear = tear,
        .mOnEvent = onEvent,
    };
}

int8_t ComponentRegistry::isModuleEnabled(ModuleID moduleId) {
    if(mModuleRegistry.find(moduleId) == mModuleRegistry.end()) {
        return false;
    }

    return true;
}

ModuleInfo ComponentRegistry::getModuleInfo(ModuleID moduleId) {
    if(mModuleRegistry.find(moduleId) == mModuleRegistry.end()) {
        return {};
    }

    return mModuleRegistry[moduleId];
}
