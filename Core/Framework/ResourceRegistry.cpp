// Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
// SPDX-License-Identifier: BSD-3-Clause-Clear

#include "ResourceRegistry.h"
#include <iostream>

std::shared_ptr<ResourceRegistry> ResourceRegistry::resourceRegistryInstance = nullptr;

ResourceRegistry::ResourceRegistry() {
    this->mTotalResources = 0;
}

int8_t ResourceRegistry::isResourceConfigMalformed(ResourceConfigInfo* rConf) {
    if(rConf == nullptr) return true;
    if(rConf->mResourceOptype < 0 || rConf->mResourceOpcode < 0) return true;
    return false;
}

void ResourceRegistry::registerResource(ResourceConfigInfo* resourceConfigInfo,
                                        int8_t isBuSpecified) {
    // Invalid Resource, skip.
    if(this->isResourceConfigMalformed(resourceConfigInfo)) {
        if(resourceConfigInfo != nullptr) {
            delete resourceConfigInfo;
        }
        return;
    }

    // Persist the Default Values of the Resources in a File.
    // These values will be used to restore the Sysfs nodes in case the Server Process crashes.
    if(resourceConfigInfo->mDefaultValue.length() != 0) {
        std::fstream sysfsPersistenceFile("sysfsOriginalValues.txt", std::ios::out | std::ios::app);
        std::string resourceData = resourceConfigInfo->mResourcePath;
        resourceData.push_back(',');
        resourceData.append(resourceConfigInfo->mDefaultValue);
        resourceData.push_back('\n');
        sysfsPersistenceFile << resourceData;
    }

    // Create the OpID Bitmap, this will serve as the key for the entry in mSystemIndependentLayerMappings.
    uint32_t resourceBitmap = 0;
    resourceBitmap |= ((uint32_t)resourceConfigInfo->mResourceOpcode);
    resourceBitmap |= ((uint32_t)resourceConfigInfo->mResourceOptype << 16);

    // Check for any conflict
    if(this->mSystemIndependentLayerMappings.find(resourceBitmap) !=
        this->mSystemIndependentLayerMappings.end()) {
        // Resource with the specified ResType and ResCode already exists
        // Overwrite it.

        int32_t resourceTableIndex = getResourceTableIndex(resourceBitmap);
        this->mResourceConfig[resourceTableIndex] = resourceConfigInfo;

        if(isBuSpecified) {
            this->mSystemIndependentLayerMappings.erase(resourceBitmap);
            resourceBitmap |= (1 << 31);

            this->mSystemIndependentLayerMappings[resourceBitmap] = resourceTableIndex;
        }
    } else {
        if(isBuSpecified) {
            resourceBitmap |= (1 << 31);
        }

        this->mSystemIndependentLayerMappings[resourceBitmap] = this->mTotalResources;
        this->mResourceConfig.push_back(resourceConfigInfo);

        this->mTotalResources++;
    }
}

void ResourceRegistry::displayResources() {
    for(int32_t i = 0; i < this->mTotalResources; i++) {
        auto& res = mResourceConfig[i];

        LOGI("RESTUNE_RESOURCE_PROCESSOR", "Resource Name: " + res->mResourceName);
        LOGI("RESTUNE_RESOURCE_PROCESSOR", "Optype: " + std::to_string(res->mResourceOptype));
        LOGI("RESTUNE_RESOURCE_PROCESSOR", "Opcode: " + std::to_string(res->mResourceOpcode));
        LOGI("RESTUNE_RESOURCE_PROCESSOR", "High Threshold: " + std::to_string(res->mHighThreshold));
        LOGI("RESTUNE_RESOURCE_PROCESSOR", "Low Threshold: " + std::to_string(res->mLowThreshold));

        if(res->mResourceApplierCallback != nullptr) {
            LOGI("RESTUNE_RESOURCE_PROCESSOR", "BU has defined its own custom Resource Applier Function");
            res->mResourceApplierCallback(nullptr);
        } else {
            LOGI("RESTUNE_RESOURCE_PROCESSOR", "No custom Resource Applier Specified, will use default one");
        }

        LOGI("RESTUNE_RESOURCE_PROCESSOR", "====================================");
    }
}

std::vector<ResourceConfigInfo*> ResourceRegistry::getRegisteredResources() {
    return mResourceConfig;
}

ResourceConfigInfo* ResourceRegistry::getResourceById(uint32_t resourceId) {
    if(this->mSystemIndependentLayerMappings.find(resourceId) == this->mSystemIndependentLayerMappings.end()) {
        TYPELOGV(RESOURCE_REGISTRY_RESOURCE_NOT_FOUND, resourceId);
        return nullptr;
    }

    int32_t resourceTableIndex = this->mSystemIndependentLayerMappings[resourceId];
    if(resourceTableIndex == -1) {
        TYPELOGV(RESOURCE_REGISTRY_RESOURCE_NOT_FOUND, resourceId);
        return nullptr;
    }
    return this->mResourceConfig[resourceTableIndex];
}

int32_t ResourceRegistry::getResourceTableIndex(uint32_t resourceId) {
    if(this->mSystemIndependentLayerMappings.find(resourceId) == this->mSystemIndependentLayerMappings.end()) {
        return -1;
    }

    return this->mSystemIndependentLayerMappings[resourceId];
}

int32_t ResourceRegistry::getTotalResourcesCount() {
    return this->mTotalResources;
}

void ResourceRegistry::pluginModifications() {
    std::vector<std::pair<uint32_t, ResourceLifecycleCallback>> applierCallbacks = Extensions::getResourceApplierCallbacks();
    std::vector<std::pair<uint32_t, ResourceLifecycleCallback>> tearCallbacks = Extensions::getResourceTearCallbacks();

    for(std::pair<uint32_t, ResourceLifecycleCallback> resource: applierCallbacks) {
        int32_t resourceTableIndex = this->getResourceTableIndex(resource.first);
        if(resourceTableIndex != -1) {
            this->mResourceConfig[resourceTableIndex]->mResourceApplierCallback = resource.second;
        }
    }

    for(std::pair<uint32_t, ResourceLifecycleCallback> resource: tearCallbacks) {
        int32_t resourceTableIndex = this->getResourceTableIndex(resource.first);
        if(resourceTableIndex != -1) {
            this->mResourceConfig[resourceTableIndex]->mResourceTearCallback = resource.second;
        }
    }
}

void ResourceRegistry::restoreResourcesToDefaultValues() {
    for(ResourceConfigInfo* resourceConfig: this->mResourceConfig) {
        std::string defaultValue = resourceConfig->mDefaultValue;
        AuxRoutines::writeToFile(resourceConfig->mResourceName, defaultValue);
    }
}

ResourceRegistry::~ResourceRegistry() {
    for(int32_t i = 0; i < this->mResourceConfig.size(); i++) {
        if(this->mResourceConfig[i] != nullptr) {
            delete this->mResourceConfig[i];
            this->mResourceConfig[i] = nullptr;
        }
    }
}

ResourceConfigInfoBuilder::ResourceConfigInfoBuilder() {
    this->mResourceConfigInfo = new (std::nothrow) ResourceConfigInfo;
    if(this->mResourceConfigInfo == nullptr) {
        return;
    }

    this->mResourceConfigInfo->mResourceApplierCallback = nullptr;
    this->mResourceConfigInfo->mResourceTearCallback = nullptr;
    this->mResourceConfigInfo->mModes = MODE_DISPLAY_ON;
}

ErrCode ResourceConfigInfoBuilder::setName(const std::string& name) {
    if(this->mResourceConfigInfo == nullptr) {
        return RC_INVALID_VALUE;
    }

    this->mResourceConfigInfo->mResourceName = name;
    return RC_SUCCESS;
}

ErrCode ResourceConfigInfoBuilder::setPath(const std::string& path) {
    if(this->mResourceConfigInfo == nullptr) {
        return RC_INVALID_VALUE;
    }

    this->mResourceConfigInfo->mResourcePath = path;
    return RC_SUCCESS;
}

ErrCode ResourceConfigInfoBuilder::setResType(const std::string& resTypeString) {
    if(this->mResourceConfigInfo == nullptr) {
        return RC_INVALID_VALUE;
    }

    this->mResourceConfigInfo->mResourceOptype = -1;
    try {
        this->mResourceConfigInfo->mResourceOptype = (int8_t)stoi(resTypeString, nullptr, 0);
    } catch(const std::invalid_argument& e) {
        TYPELOGV(RESOURCE_REGISTRY_PARSING_FAILURE, e.what());
        return RC_INVALID_VALUE;

    } catch(const std::out_of_range& e) {
        TYPELOGV(RESOURCE_REGISTRY_PARSING_FAILURE, e.what());
        return RC_INVALID_VALUE;
    }

    return RC_SUCCESS;
}

ErrCode ResourceConfigInfoBuilder::setResID(const std::string& resIDString) {
    if(this->mResourceConfigInfo == nullptr) {
        return RC_INVALID_VALUE;
    }

    this->mResourceConfigInfo->mResourceOpcode = -1;
    try {
        this->mResourceConfigInfo->mResourceOpcode = (int16_t)stoi(resIDString, nullptr, 0);
    } catch(const std::invalid_argument& e) {
        TYPELOGV(RESOURCE_REGISTRY_PARSING_FAILURE, e.what());
        return RC_INVALID_VALUE;

    } catch(const std::out_of_range& e) {
        TYPELOGV(RESOURCE_REGISTRY_PARSING_FAILURE, e.what());
        return RC_INVALID_VALUE;
    }

    return RC_SUCCESS;
}

ErrCode ResourceConfigInfoBuilder::setHighThreshold(int32_t highThreshold) {
    if(this->mResourceConfigInfo == nullptr) {
        return RC_INVALID_VALUE;
    }

    this->mResourceConfigInfo->mHighThreshold = highThreshold;
    return RC_SUCCESS;
}

ErrCode ResourceConfigInfoBuilder::setLowThreshold(int32_t lowThreshold) {
    if(this->mResourceConfigInfo == nullptr) {
        return RC_INVALID_VALUE;
    }

    this->mResourceConfigInfo->mLowThreshold = lowThreshold;
    return RC_SUCCESS;
}

ErrCode ResourceConfigInfoBuilder::setPermissions(const std::string& permissionString) {
    if(this->mResourceConfigInfo == nullptr) {
        return RC_INVALID_VALUE;
    }

    enum Permissions permissions = PERMISSION_THIRD_PARTY;
    if(permissionString == "system") {
        permissions = PERMISSION_SYSTEM;
    } else if(permissionString == "third_party") {
        permissions = PERMISSION_THIRD_PARTY;
    }

    this->mResourceConfigInfo->mPermissions = permissions;
    return RC_SUCCESS;
}

ErrCode ResourceConfigInfoBuilder::setModes(const std::string& modeString) {
    if(this->mResourceConfigInfo == nullptr) {
        return RC_INVALID_VALUE;
    }

    if(modeString == "display_on") {
        this->mResourceConfigInfo->mModes = this->mResourceConfigInfo->mModes | MODE_DISPLAY_ON;
    } else if(modeString == "display_off") {
        this->mResourceConfigInfo->mModes = this->mResourceConfigInfo->mModes | MODE_DISPLAY_OFF;
    } else if(modeString == "doze") {
        this->mResourceConfigInfo->mModes = this->mResourceConfigInfo->mModes | MODE_DOZE;
    }
    return RC_SUCCESS;
}

ErrCode ResourceConfigInfoBuilder::setSupported(int8_t supported) {
    if(this->mResourceConfigInfo == nullptr) {
        return RC_INVALID_VALUE;
    }

    this->mResourceConfigInfo->mSupported = supported;
    return RC_SUCCESS;
}

ErrCode ResourceConfigInfoBuilder::setPolicy(const std::string& policyString) {
    if(this->mResourceConfigInfo == nullptr) {
        return RC_INVALID_VALUE;
    }

    enum Policy policy = LAZY_APPLY;
    if(policyString == "higher_is_better") {
        policy = HIGHER_BETTER;
    } else if(policyString == "lower_is_better") {
        policy = LOWER_BETTER;
    } else if(policyString == "lazy_apply") {
        policy = LAZY_APPLY;
    } else if(policyString == "instant_apply") {
        policy = INSTANT_APPLY;
    }
    this->mResourceConfigInfo->mPolicy = policy;
    return RC_SUCCESS;
}

ErrCode ResourceConfigInfoBuilder::setApplyType(const std::string& applyTypeString) {
    if(this->mResourceConfigInfo == nullptr) {
        return RC_INVALID_VALUE;
    }

    enum ResourceApplyType applyType = APPLY_GLOBAL;
    if(applyTypeString == "global") {
        applyType = APPLY_GLOBAL;
    } else if(applyTypeString == "core") {
        applyType = APPLY_CORE;
    } else if(applyTypeString == "cluster") {
        applyType = APPLY_CLUSTER;
    } else if(applyTypeString == "cgroup") {
        applyType = APPLY_CGROUP;
    }

    this->mResourceConfigInfo->mApplyType = applyType;
    return RC_SUCCESS;
}

ErrCode ResourceConfigInfoBuilder::setDefaultValue(const std::string& defaultValue) {
    if(this->mResourceConfigInfo == nullptr) {
        return RC_INVALID_VALUE;
    }

    this->mResourceConfigInfo->mDefaultValue = defaultValue;
    return RC_SUCCESS;
}

ResourceConfigInfo* ResourceConfigInfoBuilder::build() {
    return this->mResourceConfigInfo;
}
