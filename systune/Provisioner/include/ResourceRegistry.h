// Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
// SPDX-License-Identifier: BSD-3-Clause-Clear

#ifndef RESOURCE_REGISTRY_H
#define RESOURCE_REGISTRY_H

#include <vector>
#include <memory>
#include <unordered_map>

#include "Utils.h"
#include "Types.h"
#include "Logger.h"

/**
 * @brief Representation of a single Resource Configuration
 */
typedef struct {
    std::string mResourceName; //<! Name of the sysfs node.
    int8_t mResourceOptype; //<! Type of the Resource, for example: POWER, CPU_DCVS, GPU etc.
    int16_t mResourceOpcode; //<! Unique opcode associated with the resource.
    int32_t mHighThreshold; //<! Upper limit of the value that is allowed/
    int32_t mLowThreshold; //<! Lower limit of the value that is allowed.
    enum Permissions mPermissions; //<! Either third party or system.
    uint8_t mModes; //<! Supported modes: Display on, display off, doze.
    int8_t mSupported; //<! boolean flag which is set if node supported in that target.
    int8_t mCoreLevelConflict; //<! boolean flag which is set if different values for different cores.
    enum Policy mPolicy; //<! Policy by which the resource is governed.
    int32_t mDefaultValue; //<! Original value of the node which should be restored if all requests are untuned.
    void (*resourceApplierCallback)(void*); //<! Optional Custom Resource Applier Callback, needs to be supplied by the BU.
} ResourceConfigInfo;

class ResourceRegistry {
private:
    static std::shared_ptr<ResourceRegistry> resourceRegistryInstance;
    int32_t mTotalResources;
    int8_t customerBit;

    std::vector<ResourceConfigInfo*> mResourceConfig;

    std::unordered_map<uint32_t, int32_t> mSystemIndependentLayerMappings;

    ResourceRegistry();

public:
    ~ResourceRegistry();

    void initRegistry(int32_t size, int8_t customerBit);

    void registerResource(ResourceConfigInfo* resourceConfigInfo);

    std::vector<ResourceConfigInfo*> getRegisteredResources();

    /**
    * @brief Get the ResourceConfigInfo object corresponding to the given Resource ID.
    * @param resourceId An unsigned 32 bit integer, representing the Resource ID.
    * @return ResourceConfigInfo*:
    *          - A pointer to the ResourceConfigInfo object
    *          - nullptr, if no ResourceConfigInfo object with the given Resource ID exists.
    */
    ResourceConfigInfo* getResourceById(uint32_t resourceId);

    int32_t getResourceTableIndex(uint32_t resourceId);

    int32_t getTotalResourcesCount();

    // Merge the Changes provided by the BU with the existing ResourceTable.
    void pluginModifications(const std::vector<std::pair<int32_t, ResourceApplierCallback>>& modifiedResources);

    void restoreResourcesToDefaultValues();

    void displayResources();

    static std::shared_ptr<ResourceRegistry> getInstance() {
        if(resourceRegistryInstance == nullptr) {
            resourceRegistryInstance = std::shared_ptr<ResourceRegistry>(new ResourceRegistry());
        }
        return resourceRegistryInstance;
    }
};

class ResourceConfigInfoBuilder {
private:
    ResourceConfigInfo* mResourceConfigInfo;

public:
    ResourceConfigInfoBuilder();

    ResourceConfigInfoBuilder* setName(std::string resourceName);
    ResourceConfigInfoBuilder* setOptype(std::string opTypeString);
    ResourceConfigInfoBuilder* setOpcode(std::string opCodeString);
    ResourceConfigInfoBuilder* setHighThreshold(int32_t highThreshold);
    ResourceConfigInfoBuilder* setLowThreshold(int32_t lowThreshold);
    ResourceConfigInfoBuilder* setPermissions(std::string permissionString);
    ResourceConfigInfoBuilder* setModes(std::string modeString);
    ResourceConfigInfoBuilder* setSupported(int8_t supported);
    ResourceConfigInfoBuilder* setPolicy(std::string policyString);
    ResourceConfigInfoBuilder* setCoreLevelConflict(int8_t coreLevelConflict);
    ResourceConfigInfoBuilder* setDefaultValue(int32_t defaultValue);

    ResourceConfigInfo* build();
};

#endif
