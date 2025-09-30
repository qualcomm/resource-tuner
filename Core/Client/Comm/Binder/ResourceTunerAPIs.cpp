// Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
// SPDX-License-Identifier: BSD-3-Clause-Clear

#include <memory>
#include <mutex>

#include "Utils.h"
#include "ResourceTunerAPIs.h"

#define REQ_SEND_ERR(e) "Failed to send Request to Server, Error: " + std::string(e)
#define CONN_SEND_FAIL "Failed to send Request to Server"
#define CONN_INIT_FAIL "Failed to initialize Connection to resource-tuner Server"

using ::aidl::vendor::qti::hardware::restune::IRestune;

static const std::string resourceTunerAidl = "vendor.qti.hardware.restune.IRestune";

class BinderManager {
private:
    IRestune* mRestuneAidl;

public:
    BinderManager() {
        mRestuneAidl = IRestune::fromBinder(
            ndk::SpAIBinder(AServiceManager_getService(resourceTunerAidl.c_str()))
        );

        if(this->mRestuneAidl == nullptr) {
            LOGE("RESTUNE_CLIENT", REQ_SEND_ERR(e.what()));
        }
    }

    ~BinderManager() {
        if(this->mRestuneAidl != nullptr) {
            delete this->mRestuneAidl;
        }
    }
};

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
static BinderManager binderManager;

int64_t tuneResources(int64_t duration, int32_t properties, int32_t numRes, SysResource* resourceList) {
    // Only one client Thread can send a Request at any moment
    try {
        const std::lock_guard<std::mutex> lock(apiLock);
        IRestune* restuneAidl = binderManager.getRestuneAidl();

        if(restuneAidl == nullptr) {
            LOGE("RESTUNE_CLIENT", CONN_INIT_FAIL);
            return -1;
        }

        return restuneAidl->tuneResources(duration, properties, numRes, resourceList);

    } catch(const std::exception& e) {
        LOGE("RESTUNE_CLIENT", REQ_SEND_ERR(e.what()));
        return -1;
    }

    return -1;
}

int8_t retuneResources(int64_t handle, int64_t duration) {
    try {
        const std::lock_guard<std::mutex> lock(apiLock);
        IRestune* restuneAidl = binderManager.getRestuneAidl();

        if(restuneAidl == nullptr) {
            LOGE("RESTUNE_CLIENT", CONN_INIT_FAIL);
            return -1;
        }

        restuneAidl->retuneResources(handle, duration);
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
        IRestune* restuneAidl = binderManager.getRestuneAidl();

        if(restuneAidl == nullptr) {
            LOGE("RESTUNE_CLIENT", CONN_INIT_FAIL);
            return -1;
        }

        restuneAidl->untuneResources(handle);
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
        IRestune* restuneAidl = binderManager.getRestuneAidl();

        if(restuneAidl == nullptr) {
            LOGE("RESTUNE_CLIENT", CONN_INIT_FAIL);
            return -1;
        }

        restuneAidl->getProp(prop, buffer, bufferSize, defValue);

        buffer[bufferSize - 1] = '\0';
        if(strncmp(resultBuf, "na", 2) == 0) {
            // Copy default value
            strncpy(buffer, defValue, bufferSize - 1);
        } else {
            strncpy(buffer, resultBuf, bufferSize - 1);
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
        IRestune* restuneAidl = binderManager.getRestuneAidl();

        if(restuneAidl == nullptr) {
            LOGE("RESTUNE_CLIENT", CONN_INIT_FAIL);
            return -1;
        }

        return restuneAidl->tuneSignal(signalCode, duration, properties, appName, scenario, numArgs, list);

    } catch(const std::exception& e) {
        LOGE("RESTUNE_CLIENT", REQ_SEND_ERR(e.what()));
        return -1;
    }

    return -1;
}

int8_t untuneSignal(int64_t handle) {
    try {
        const std::lock_guard<std::mutex> lock(apiLock);
        IRestune* restuneAidl = binderManager.getRestuneAidl();

        if(restuneAidl == nullptr) {
            LOGE("RESTUNE_CLIENT", CONN_INIT_FAIL);
            return -1;
        }

        restuneAidl->untuneSignal(handle);
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
        IRestune* restuneAidl = binderManager.getRestuneAidl();

        if(restuneAidl == nullptr) {
            LOGE("RESTUNE_CLIENT", CONN_INIT_FAIL);
            return -1;
        }

        restuneAidl->relaySignal(signalCode, duration, properties, appName, scenario, numArgs, list);
        return 0;

    } catch(const std::exception& e) {
        LOGE("RESTUNE_CLIENT", REQ_SEND_ERR(e.what()));
        return -1;
    }

    return -1;
}
