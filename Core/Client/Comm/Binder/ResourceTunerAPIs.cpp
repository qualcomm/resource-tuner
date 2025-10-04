// Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
// SPDX-License-Identifier: BSD-3-Clause-Clear

#include <memory>
#include <mutex>
#include <android/binder_manager.h>
#include <android/binder_process.h>
#include <android/binder_interface_utils.h>
#include <aidl/vendor/qti/hardware/restune/BnRestune.h>

#include "Utils.h"
#include "Logger.h"
#include "ResourceTunerAPIs.h"

using IRestuneAidl = ::aidl::vendor::qti::hardware::restune::IRestune;
using ::ndk::SpAIBinder;

#define REQ_SEND_ERR(e) "Failed to send Request to Server, Error: " + std::string(e)
#define CONN_SEND_FAIL "Failed to send Request to Server"
#define CONN_INIT_FAIL "Failed to initialize Connection to resource-tuner Server"

static const std::string resourceTunerAidl = "vendor.qti.hardware.restune.IRestune";
static std::shared_ptr<IRestuneAidl> gRestuneAidl = nullptr;

class ClientLogger {
public:
    ClientLogger() {
        openlog("restune-client", LOG_PID | LOG_CONS, LOG_USER);
    }

    ~ClientLogger() {
        closelog();
    }
};

static ClientLogger clientLogger;
static std::mutex apiLock;

static void populateBinder() {
    if(gRestuneAidl == nullptr) {
        gRestuneAidl = IRestuneAidl::fromBinder(
            ndk::SpAIBinder(AServiceManager_getService(resourceTunerAidl.c_str()))
        );

        if(gRestuneAidl == nullptr) {
            LOGE("RESTUNE_CLIENT", "Failed to get Binder Proxy");
        }
    }
}

int64_t tuneResources(int64_t duration, int32_t properties, int32_t numRes, SysResource* resourceList) {
    // Only one client Thread can send a Request at any moment
    try {
        const std::lock_guard<std::mutex> lock(apiLock);
        populateBinder();

        if(gRestuneAidl == nullptr) {
            LOGE("RESTUNE_CLIENT", CONN_INIT_FAIL);
            return -1;
        }

        std::vector<aidl::vendor::qti::hardware::restune::SysResource> resourceVector(numRes);
        // for(int32_t i = 0; i < numRes; i++) {
        //     resourceVector[i] = resourceList[i];
        // }

        int64_t handle;
        gRestuneAidl->tuneResources(duration, properties, resourceVector, &handle);

        return handle;

    } catch(const std::exception& e) {
        LOGE("RESTUNE_CLIENT", REQ_SEND_ERR(e.what()));
        return -1;
    }

    return -1;
}

int8_t retuneResources(int64_t handle, int64_t duration) {
    try {
        const std::lock_guard<std::mutex> lock(apiLock);
        populateBinder();

        if(gRestuneAidl == nullptr) {
            LOGE("RESTUNE_CLIENT", CONN_INIT_FAIL);
            return -1;
        }

        int8_t status;
        gRestuneAidl->retuneResources(handle, duration, &status);

        return 0;

    } catch(const std::exception& e) {
        LOGE("RESTUNE_CLIENT", REQ_SEND_ERR(e.what()));
        return -1;
    }

    return -1;
}

int8_t untuneResources(int64_t handle) {
    try {
        const std::lock_guard<std::mutex> lock(apiLock);
        populateBinder();

        if(gRestuneAidl == nullptr) {
            LOGE("RESTUNE_CLIENT", CONN_INIT_FAIL);
            return -1;
        }

        int8_t status;
        gRestuneAidl->untuneResources(handle, &status);

        return 0;

    } catch(const std::exception& e) {
        LOGE("RESTUNE_CLIENT", REQ_SEND_ERR(e.what()));
        return -1;
    }

    return -1;
}

int8_t getProp(const char* prop, char* buffer, size_t bufferSize, const char* defValue) {
    try {
        const std::lock_guard<std::mutex> lock(apiLock);
        populateBinder();

        if(gRestuneAidl == nullptr) {
            LOGE("RESTUNE_CLIENT", CONN_INIT_FAIL);
            return -1;
        }

        std::string propValue = "";
        gRestuneAidl->getProp(prop, defValue, &propValue);

        buffer[bufferSize - 1] = '\0';
        if(propValue == "na") {
            strncpy(buffer, defValue, bufferSize - 1);
        } else {
            strncpy(buffer, propValue.c_str(), bufferSize - 1);
        }

        return 0;

    } catch(const std::exception& e) {
        LOGE("RESTUNE_CLIENT", REQ_SEND_ERR(e.what()));
        return -1;
    }

    return -1;
}

int64_t tuneSignal(uint32_t signalCode, int64_t duration, int32_t properties,
                   const char* appName, const char* scenario, int32_t numArgs,
                   uint32_t* list) {
    try {
        const std::lock_guard<std::mutex> lock(apiLock);
        populateBinder();

        if(gRestuneAidl == nullptr) {
            LOGE("RESTUNE_CLIENT", CONN_INIT_FAIL);
            return -1;
        }

        std::vector<int64_t> argsVector(numArgs);
        for(int32_t i = 0; i < numArgs; i++) {
            argsVector[i] = static_cast<int64_t>(list[i]);
        }

        int64_t handle;
        gRestuneAidl->tuneSignal(signalCode, duration, properties, appName, scenario, argsVector, &handle);

        return handle;

    } catch(const std::exception& e) {
        LOGE("RESTUNE_CLIENT", REQ_SEND_ERR(e.what()));
        return -1;
    }

    return -1;
}

int8_t untuneSignal(int64_t handle) {
    try {
        const std::lock_guard<std::mutex> lock(apiLock);
        populateBinder();

        if(gRestuneAidl == nullptr) {
            LOGE("RESTUNE_CLIENT", CONN_INIT_FAIL);
            return -1;
        }

        int8_t status;
        gRestuneAidl->untuneSignal(handle, &status);

        return 0;

    } catch(const std::exception& e) {
        LOGE("RESTUNE_CLIENT", REQ_SEND_ERR(e.what()));
        return -1;
    }

    return -1;
}

int8_t relaySignal(uint32_t signalCode, int64_t duration, int32_t properties,
                   const char* appName, const char* scenario, int32_t numArgs, uint32_t* list) {
    try {
        const std::lock_guard<std::mutex> lock(apiLock);
        populateBinder();

        if(gRestuneAidl == nullptr) {
            LOGE("RESTUNE_CLIENT", CONN_INIT_FAIL);
            return -1;
        }

        std::vector<int64_t> argsVector(numArgs);
        for(int32_t i = 0; i < numArgs; i++) {
            argsVector[i] = static_cast<int64_t>(list[i]);
        }

        int8_t status;
        gRestuneAidl->relaySignal(signalCode, duration, properties, appName, scenario, argsVector, &status);

        return 0;

    } catch(const std::exception& e) {
        LOGE("RESTUNE_CLIENT", REQ_SEND_ERR(e.what()));
        return -1;
    }

    return -1;
}
