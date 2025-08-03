// Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
// SPDX-License-Identifier: BSD-3-Clause-Clear

#ifndef RESOURCE_REGISTRY_H
#define RESOURCE_REGISTRY_H

#include <vector>
#include <memory>
#include <unordered_map>

#include "Utils.h"
#include "Logger.h"

/**
 * @struct ResourceConfigInfo
 * @brief Representation of a single Resource Configuration
 * @details This information is read from the Config files.\n
 *          Note this (ResourceConfigInfo) struct is separate from the Resource struct.
 */
typedef struct {
    /**
     * @brief Name of the sysfs node.
     */
    std::string mResourceName;
    /**
     * @brief Type of the Resource, for example: POWER, CPU_DCVS, GPU etc.
     */
    int8_t mResourceOptype;
    /**
     * @brief Unique opcode associated with the resource.
     */
    int16_t mResourceOpcode;
    /**
     * @brief Max Possible Value which can be configured for this Resource.
     */
    int32_t mHighThreshold;
    /**
     * @brief Min Possible Value which can be configured for this Resource.
     */
    int32_t mLowThreshold;
    /**
     * @brief The Permission Level needed by a client in order to tune this Resource
     */
    enum Permissions mPermissions;
    /**
     * @brief Specify the mode (ex: Display on, display off, doze) under which the Resource can be provisioned
     */
    uint8_t mModes;
    /**
     * @brief boolean flag which is set if node is available for Tuning.
     */
    int8_t mSupported;
    /**
     * @brief boolean flag which is set if the Resource can have different values on different cores.
     */
    int8_t mCoreLevelConflict;
    /**
     * @brief Policy by which the resource is governed, for example Higher is Better.
     */
    enum Policy mPolicy;
    /**
     * @brief Original value of the Resource node, i.e. the value before any Tuning.
     */
    int32_t mDefaultValue;
    /**
     * @brief Optional Custom Resource Applier Callback, it needs to be supplied by
     *        the BU via the Extension Interface.
     */
    void (*resourceApplierCallback)(void*);
} ResourceConfigInfo;

/**
* @brief ResourceRegistry
* @details Stores information Relating to all the Resources available for Tuning.
*          Note: This information is extracted from Config JSON files.
*/
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

    ResourceConfigInfoBuilder* setName(const std::string& resourceName);
    ResourceConfigInfoBuilder* setOptype(const std::string& opTypeString);
    ResourceConfigInfoBuilder* setOpcode(const std::string& opCodeString);
    ResourceConfigInfoBuilder* setHighThreshold(int32_t highThreshold);
    ResourceConfigInfoBuilder* setLowThreshold(int32_t lowThreshold);
    ResourceConfigInfoBuilder* setPermissions(const std::string& permissionString);
    ResourceConfigInfoBuilder* setModes(const std::string& modeString);
    ResourceConfigInfoBuilder* setSupported(int8_t supported);
    ResourceConfigInfoBuilder* setPolicy(const std::string& policyString);
    ResourceConfigInfoBuilder* setCoreLevelConflict(int8_t coreLevelConflict);
    ResourceConfigInfoBuilder* setDefaultValue(int32_t defaultValue);

    ResourceConfigInfo* build();
};

#endif
