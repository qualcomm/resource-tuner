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
        delete resourceConfigInfo;
        return;
    }

    // Persist the Default Values of the Resources in a File.
    // These values will be used to restore the Sysfs nodes in case the Server Process crashes.
    if(resourceConfigInfo->mDefaultValue.length() != 0) {
        std::fstream sysfsPersistenceFile("../sysfsOriginalValues.txt", std::ios::out | std::ios::app);
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

        LOGI("RTN_RESOURCE_PROCESSOR", "Resource Name: " + res->mResourceName);
        LOGI("RTN_RESOURCE_PROCESSOR", "Optype: " + std::to_string(res->mResourceOptype));
        LOGI("RTN_RESOURCE_PROCESSOR", "Opcode: " + std::to_string(res->mResourceOpcode));
        LOGI("RTN_RESOURCE_PROCESSOR", "High Threshold: " + std::to_string(res->mHighThreshold));
        LOGI("RTN_RESOURCE_PROCESSOR", "Low Threshold: " + std::to_string(res->mLowThreshold));

        if(res->mResourceApplierCallback != nullptr) {
            LOGI("RTN_RESOURCE_PROCESSOR", "BU has defined its own custom Resource Applier Function");
            res->mResourceApplierCallback(nullptr);
        } else {
            LOGI("RTN_RESOURCE_PROCESSOR", "No custom Resource Applier Specified, will use default one");
        }

        LOGI("RTN_RESOURCE_PROCESSOR", "====================================");
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

static void writeToNode(const std::string& filePath, const std::string& value) {
    std::ofstream resourceConfigFile(filePath, std::ios::out | std::ios::trunc);

    if(!resourceConfigFile.is_open()) {
        LOGD("RTN_COCO_TABLE", "Failed to open file: " + filePath);
        return;
    }

    resourceConfigFile<<value;
    if(resourceConfigFile.fail()) {
        LOGD("RTN_COCO_TABLE", "Failed to write to file: " + filePath);
    }
    resourceConfigFile.flush();
    resourceConfigFile.close();
}

void ResourceRegistry::restoreResourcesToDefaultValues() {
    for(ResourceConfigInfo* resourceConfig: this->mResourceConfig) {
        std::string defaultValue = resourceConfig->mDefaultValue;
        writeToNode(resourceConfig->mResourceName, defaultValue);
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
    this->mResourceConfigInfo->mModes = 0;
}

ResourceConfigInfoBuilder* ResourceConfigInfoBuilder::setName(const std::string& name) {
    if(this->mResourceConfigInfo == nullptr) return this;

    this->mResourceConfigInfo->mResourceName = name;
    return this;
}

ResourceConfigInfoBuilder* ResourceConfigInfoBuilder::setPath(const std::string& path) {
    if(this->mResourceConfigInfo == nullptr) return this;

    this->mResourceConfigInfo->mResourcePath = path;
    return this;
}

ResourceConfigInfoBuilder* ResourceConfigInfoBuilder::setOptype(const std::string& opTypeString) {
    if(this->mResourceConfigInfo == nullptr) return this;

    this->mResourceConfigInfo->mResourceOptype = -1;
    try {
        this->mResourceConfigInfo->mResourceOptype = (int8_t)stoi(opTypeString, nullptr, 0);
    } catch(const std::invalid_argument& e) {
        TYPELOGV(RESOURCE_REGISTRY_PARSING_FAILURE, e.what());
    } catch(const std::out_of_range& e) {
        TYPELOGV(RESOURCE_REGISTRY_PARSING_FAILURE, e.what());
    }
    return this;
}

ResourceConfigInfoBuilder* ResourceConfigInfoBuilder::setOpcode(const std::string& opCodeString) {
    if(this->mResourceConfigInfo == nullptr) return this;

    this->mResourceConfigInfo->mResourceOpcode = -1;
    try {
        this->mResourceConfigInfo->mResourceOpcode = (int16_t)stoi(opCodeString, nullptr, 0);
    } catch(const std::invalid_argument& e) {
        TYPELOGV(RESOURCE_REGISTRY_PARSING_FAILURE, e.what());
    } catch(const std::out_of_range& e) {
        TYPELOGV(RESOURCE_REGISTRY_PARSING_FAILURE, e.what());
    }
    return this;
}

ResourceConfigInfoBuilder* ResourceConfigInfoBuilder::setHighThreshold(int32_t highThreshold) {
    if(this->mResourceConfigInfo == nullptr) return this;

    this->mResourceConfigInfo->mHighThreshold = highThreshold;
    return this;
}

ResourceConfigInfoBuilder* ResourceConfigInfoBuilder::setLowThreshold(int32_t lowThreshold) {
    if(this->mResourceConfigInfo == nullptr) return this;

    this->mResourceConfigInfo->mLowThreshold = lowThreshold;
    return this;
}

ResourceConfigInfoBuilder* ResourceConfigInfoBuilder::setPermissions(const std::string& permissionString) {
    if(this->mResourceConfigInfo == nullptr) return this;

    enum Permissions permissions = PERMISSION_THIRD_PARTY;
    if(permissionString == "system") {
        permissions = PERMISSION_SYSTEM;
    } else if(permissionString == "third_party") {
        permissions = PERMISSION_THIRD_PARTY;
    }

    this->mResourceConfigInfo->mPermissions = permissions;
    return this;
}

ResourceConfigInfoBuilder* ResourceConfigInfoBuilder::setModes(const std::string& modeString) {
    if(this->mResourceConfigInfo == nullptr) return this;

    if(modeString == "display_on") {
        this->mResourceConfigInfo->mModes = this->mResourceConfigInfo->mModes | MODE_DISPLAY_ON;
    } else if(modeString == "display_off") {
        this->mResourceConfigInfo->mModes = this->mResourceConfigInfo->mModes | MODE_DISPLAY_OFF;
    } else if(modeString == "doze") {
        this->mResourceConfigInfo->mModes = this->mResourceConfigInfo->mModes | MODE_DOZE;
    }
    return this;
}

ResourceConfigInfoBuilder* ResourceConfigInfoBuilder::setSupported(int8_t supported) {
    if(this->mResourceConfigInfo == nullptr) return this;

    this->mResourceConfigInfo->mSupported = supported;
    return this;
}

ResourceConfigInfoBuilder* ResourceConfigInfoBuilder::setPolicy(const std::string& policyString) {
    if(this->mResourceConfigInfo == nullptr) return this;

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
    return this;
}

ResourceConfigInfoBuilder* ResourceConfigInfoBuilder::setApplyType(const std::string& applyTypeString) {
    if(this->mResourceConfigInfo == nullptr) return this;

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
    return this;
}

ResourceConfigInfoBuilder* ResourceConfigInfoBuilder::setDefaultValue(const std::string& defaultValue) {
    if(this->mResourceConfigInfo == nullptr) return this;

    this->mResourceConfigInfo->mDefaultValue = defaultValue;
    return this;
}

ResourceConfigInfo* ResourceConfigInfoBuilder::build() {
    return this->mResourceConfigInfo;
}
