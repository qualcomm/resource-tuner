// Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
// SPDX-License-Identifier: BSD-3-Clause-Clear

#ifndef RESOURCE_REGISTRY_H
#define RESOURCE_REGISTRY_H

#include <vector>
#include <memory>
#include <unordered_map>

#include "Utils.h"
#include "Resource.h"
#include "Extensions.h"
#include "Logger.h"

enum ResourceApplyType {
    APPLY_CORE,
    APPLY_CLUSTER,
    APPLY_GLOBAL,
    APPLY_CGROUP
};

/**
 * @struct ResourceConfigInfo
 * @brief Representation of a single Resource Configuration
 * @details This information is read from the Config files.\n
 *          Note this (ResourceConfigInfo) struct is separate from the Resource struct.
 */
typedef struct {
    /**
     * @brief Name of the Resource (Placeholder).
     */
    std::string mResourceName;
    /**
     * @brief Path to the Sysfs node, CGroup controller file or as applicable.
     */
    std::string mResourcePath;
    /**
     * @brief Type of the Resource, for example: LPM, CPU_DCVS, GPU etc.
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
     * @brief Application Type Enum, indicating whether the specified value for the Resource
     *        by a Request, needs to be applied at a per-core, per-cluster or global value.
     */
    enum ResourceApplyType mApplyType;
    /**
     * @brief Policy by which the resource is governed, for example Higher is Better.
     */
    enum Policy mPolicy;
    /**
     * @brief Original value of the Resource node, i.e. the value before any Tuning.
     */
    std::string mDefaultValue;
    /**
     * @brief Optional Custom Resource Applier Callback, it needs to be supplied by
     *        the BU via the Extension Interface.
     */
    ResourceLifecycleCallback mResourceApplierCallback;

    /**
     * @brief Optional Custom Resource Tear Callback, it needs to be supplied by
     *        the BU via the Extension Interface.
     */
    ResourceLifecycleCallback mResourceTearCallback;
} ResourceConfigInfo;

/**
* @brief ResourceRegistry
* @details Stores information Relating to all the Resources available for Tuning.
*          Note: This information is extracted from Config YAML files.
*/
class ResourceRegistry {
private:
    static std::shared_ptr<ResourceRegistry> resourceRegistryInstance;
    int32_t mTotalResources;

    std::vector<ResourceConfigInfo*> mResourceConfig;

    std::unordered_map<uint32_t, int32_t> mSystemIndependentLayerMappings;

    ResourceRegistry();

    /**
     * @brief Checks if a ResourceConfig is malformed
     * @details Validates all the mandatory fields form the ResourceConfig and
     *          checks if they have sane values.
     */
    int8_t isResourceConfigMalformed(ResourceConfigInfo* resourceConfigInfo);

public:
    ~ResourceRegistry();

    void initRegistry();

    /**
     * @brief Used to register a Config specified (through YAML) Resource with Resource Tuner
     * @details The Resource Info is parsed from YAML files. If the ResourceConfig provided is
     *          Malformed, then it will be freed as part of this routine, else it will
     *          be added to the "mResourceConfig" vector.
     */
    void registerResource(ResourceConfigInfo* resourceConfigInfo, int8_t isBuSpecified=false);

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
    void pluginModifications();

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
    ResourceConfigInfoBuilder* setPath(const std::string& resourcePath);
    ResourceConfigInfoBuilder* setOptype(const std::string& opTypeString);
    ResourceConfigInfoBuilder* setOpcode(const std::string& opCodeString);
    ResourceConfigInfoBuilder* setHighThreshold(int32_t highThreshold);
    ResourceConfigInfoBuilder* setLowThreshold(int32_t lowThreshold);
    ResourceConfigInfoBuilder* setPermissions(const std::string& permissionString);
    ResourceConfigInfoBuilder* setModes(const std::string& modeString);
    ResourceConfigInfoBuilder* setSupported(int8_t supported);
    ResourceConfigInfoBuilder* setPolicy(const std::string& policyString);
    ResourceConfigInfoBuilder* setApplyType(const std::string& applyTypeString);
    ResourceConfigInfoBuilder* setDefaultValue(const std::string& defaultValue);

    ResourceConfigInfo* build();
};

#endif
