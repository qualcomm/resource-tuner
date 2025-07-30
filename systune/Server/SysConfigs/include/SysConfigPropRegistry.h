// Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
// SPDX-License-Identifier: BSD-3-Clause-Clear

#ifndef SYSCONFIG_PROP_REGISTRY_H
#define SYSCONFIG_PROP_REGISTRY_H

#include <iostream>
#include <unordered_map>
#include <memory>
#include <shared_mutex>

class SysConfigPropRegistry {
private:
    static std::shared_ptr<SysConfigPropRegistry> sysConfigPropRegistryInstance;
    std::unordered_map<std::string, std::string> mProperties;
    std::shared_timed_mutex mPropRegistryMutex;

    SysConfigPropRegistry();

public:
    ~SysConfigPropRegistry();

    int8_t createProperty(const std::string& propertyName, const std::string& propertyValue);
    int8_t queryProperty(const std::string& propertyName, std::string& result);
    int8_t modifyProperty(const std::string& propertyName, const std::string& propertyValue);
    int8_t deleteProperty(const std::string& propertyName);

    int32_t getPropertiesCount();

    static std::shared_ptr<SysConfigPropRegistry> getInstance() {
        if(sysConfigPropRegistryInstance == nullptr) {
            std::shared_ptr<SysConfigPropRegistry> localSysConfigPropRegistryInstance(new SysConfigPropRegistry());
            localSysConfigPropRegistryInstance.swap(sysConfigPropRegistryInstance);
        }
        return sysConfigPropRegistryInstance;
    }
};

#endif
