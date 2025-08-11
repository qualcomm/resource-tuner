// Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
// SPDX-License-Identifier: BSD-3-Clause-Clear

#include "ComponentRegistry.h"

std::unordered_map<ModuleIdentifier, ModuleCallbacks> ComponentRegistry::mModuleCallbacks{};

ComponentRegistry::ComponentRegistry(ModuleIdentifier moduleIdentifier,
                               ModuleCallback registrationCallback,
                               ModuleCallback terardownCallback,
                               ModuleMessageHandlerCallback messageHandlerCallback) {
    mModuleCallbacks[moduleIdentifier] = {
        .initCallback = registrationCallback,
        .teardownCallback = terardownCallback,
        .messageHandlerCallback = messageHandlerCallback,
    };
}

int8_t ComponentRegistry::isModuleEnabled(ModuleIdentifier moduleIdentifier) {
    if(mModuleCallbacks.find(moduleIdentifier) == mModuleCallbacks.end()) {
        return false;
    }

    return (mModuleCallbacks[moduleIdentifier].initCallback != nullptr) &&
           (mModuleCallbacks[moduleIdentifier].teardownCallback != nullptr) &&
           (mModuleCallbacks[moduleIdentifier].messageHandlerCallback != nullptr);
}

ModuleCallback ComponentRegistry::getModuleRegistrationCallback(ModuleIdentifier moduleIdentifier) {
    return mModuleCallbacks.at(moduleIdentifier).initCallback;
}

ModuleCallback ComponentRegistry::getModuleTeardownCallback(ModuleIdentifier moduleIdentifier) {
    return mModuleCallbacks.at(moduleIdentifier).teardownCallback;
}

ModuleMessageHandlerCallback ComponentRegistry::getModuleMessageHandlerCallback(ModuleIdentifier moduleIdentifier) {
    return mModuleCallbacks.at(moduleIdentifier).messageHandlerCallback;
}
