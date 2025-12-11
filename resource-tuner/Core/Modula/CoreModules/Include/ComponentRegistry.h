// Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
// SPDX-License-Identifier: BSD-3-Clause-Clear

#ifndef COMPONENT_REGISTRY_H
#define COMPONENT_REGISTRY_H

#include <unordered_map>

#include "Utils.h"

enum ModuleIdentifier {
    MOD_CORE,
    MOD_SIGNAL,
    MOD_STATE_OPTIMIZER,
};

enum EventIdentifier {
    MOD_CORE_INIT,
    MOD_CORE_TEAR,
    MOD_CORE_ON_MSG_RECV,
    MOD_SIGNAL_INIT,
    MOD_SIGNAL_TEAR,
    MOD_SIGNAL_ON_MSG_RECV,
    PROP_ON_MSG_RECV,
    MOD_STATE_OPTIMIZER_INIT,
    MOD_STATE_OPTIMIZER_TEAR,
};

/**
 * @brief ComponentRegistry
 * @details Keeps track of the Resource Tuner modules, used to check if a module
 *          is enabled or not and stores the module's registered callbacks if it is enabled.
 */
class ComponentRegistry {
private:
    static std::unordered_map<EventIdentifier, EventCallback> mEventCallbacks;
    static std::unordered_map<ModuleIdentifier, int8_t> mModuleRegistry;

public:
    ComponentRegistry(EventIdentifier EventIdentifier,
                      EventCallback messageHandlerCallback);

    ComponentRegistry(ModuleIdentifier EventIdentifier,
                      EventCallback registrationCallback,
                      EventCallback terardownCallback,
                      EventCallback messageHandlerCallback);

    static int8_t isModuleEnabled(ModuleIdentifier moduleIdentifier);
    static EventCallback getEventCallback(EventIdentifier EventIdentifier);
};

#define CONCAT(a, b) a ## b

#define RESTUNE_REGISTER_MODULE(identifier, registration, teardown, handler) \
        static ComponentRegistry CONCAT(_module, identifier)(identifier, registration, teardown, handler);

#define RESTUNE_REGISTER_EVENT_CALLBACK(identifier, handler) \
        static ComponentRegistry CONCAT(_eventCB, identifier)(identifier, handler);

#endif
