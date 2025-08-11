// Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
// SPDX-License-Identifier: BSD-3-Clause-Clear

#ifndef MODULE_REGISTRY_H
#define MODULE_REGISTRY_H

#include <cstdint>
#include <unordered_map>

#include "Utils.h"

enum ModuleIdentifier {
    MOD_PROVISIONER,
    MOD_SYSSIGNAL,
};

typedef struct {
    ModuleCallback initCallback;
    ModuleCallback teardownCallback;
    ModuleMessageHandlerCallback messageHandlerCallback;
} ModuleCallbacks;

class ComponentRegistry {
private:
    static std::unordered_map<ModuleIdentifier, ModuleCallbacks> mModuleCallbacks;

public:
    ComponentRegistry(ModuleIdentifier moduleIdentifier,
                   ModuleCallback registrationCallback,
                   ModuleCallback terardownCallback,
                   ModuleMessageHandlerCallback messageHandlerCallback);

    static int8_t isModuleEnabled(ModuleIdentifier moduleIdentifier);
    static ModuleCallback getModuleRegistrationCallback(ModuleIdentifier moduleIdentifier);
    static ModuleCallback getModuleTeardownCallback(ModuleIdentifier moduleIdentifier);
    static ModuleMessageHandlerCallback getModuleMessageHandlerCallback(ModuleIdentifier moduleIdentifier);
};

#define CONCAT(a, b) a ## b

#define RTN_REGISTER_MODULE(identifier, registration, teardown, handler) \
        static ComponentRegistry CONCAT(_module, identifier)(identifier, registration, teardown, handler);

#endif
