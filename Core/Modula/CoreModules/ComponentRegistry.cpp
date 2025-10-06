// Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
// SPDX-License-Identifier: BSD-3-Clause-Clear

#include "ComponentRegistry.h"

std::unordered_map<EventIdentifier, EventCallback> ComponentRegistry::mEventCallbacks{};
std::unordered_map<ModuleIdentifier, int8_t> ComponentRegistry::mModuleRegistry{};

ComponentRegistry::ComponentRegistry(EventIdentifier eventIdentifier, EventCallback onMsgRecv) {
    if(onMsgRecv == nullptr) {
        return;
    }

    mEventCallbacks[eventIdentifier] = onMsgRecv;
}

ComponentRegistry::ComponentRegistry(
                               ModuleIdentifier moduleIdentifier,
                               EventCallback init,
                               EventCallback teardown,
                               EventCallback onMsgRecv) {
    if(init == nullptr || teardown == nullptr || onMsgRecv == nullptr) {
        return;
    }

    switch(moduleIdentifier) {
        case ModuleIdentifier::MOD_CORE: {
            mEventCallbacks[MOD_CORE_INIT] = init;
            mEventCallbacks[MOD_CORE_TEAR] = teardown;
            mEventCallbacks[MOD_CORE_ON_MSG_RECV] = onMsgRecv;
            break;
        }
        case ModuleIdentifier::MOD_SIGNAL: {
            mEventCallbacks[MOD_SIGNAL_INIT] = init;
            mEventCallbacks[MOD_SIGNAL_TEAR] = teardown;
            mEventCallbacks[MOD_SIGNAL_ON_MSG_RECV] = onMsgRecv;
            break;
        }
        case ModuleIdentifier::MOD_STATE_OPTIMIZER: {
            mEventCallbacks[MOD_STATE_OPTIMIZER_INIT] = init;
            mEventCallbacks[MOD_STATE_OPTIMIZER_TEAR] = teardown;
            break;
        }
        default:
            return;
    }

    mModuleRegistry[moduleIdentifier] = true;
}

int8_t ComponentRegistry::isModuleEnabled(ModuleIdentifier moduleIdentifier) {
    if(mModuleRegistry.find(moduleIdentifier) == mModuleRegistry.end()) {
        return false;
    }

    return mModuleRegistry[moduleIdentifier];
}

EventCallback ComponentRegistry::getEventCallback(EventIdentifier eventIdentifier) {
    if(mEventCallbacks.find(eventIdentifier) == mEventCallbacks.end()) {
        return nullptr;
    }

    return mEventCallbacks[eventIdentifier];
}
