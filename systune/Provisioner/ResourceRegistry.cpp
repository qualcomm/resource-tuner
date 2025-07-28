// Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
// SPDX-License-Identifier: BSD-3-Clause-Clear

#include "ResourceRegistry.h"
#include <iostream>

std::shared_ptr<ResourceRegistry> ResourceRegistry::resourceRegistryInstance = nullptr;

ResourceRegistry::ResourceRegistry() {
    this->customerBit = false;
    this->mTotalResources = 0;
}

void ResourceRegistry::initRegistry(int32_t size, int8_t customerBit) {
    if(mResourceConfig.size() == 0 && size > 0) {
        this->mResourceConfig.resize(size);
        this->customerBit = customerBit;
    }
}

void ResourceRegistry::registerResource(ResourceConfigInfo* resourceConfigInfo) {
    // Persist the Default Values of the Resources in a File.
    // These values will be used to restore the Sysfs nodes in case the Server Process crashes.
    std::fstream sysfsPersistenceFile("../sysfsOriginalValues.txt", std::ios::out | std::ios::app);
    std::string resourceData = resourceConfigInfo->mResourceName;
    resourceData.push_back(',');
    resourceData.append(std::to_string(resourceConfigInfo->mDefaultValue));
    resourceData.push_back('\n');
    sysfsPersistenceFile << resourceData;

    // Invalid Resource, skip.
    if(resourceConfigInfo->mResourceOpcode == -1 ||
       resourceConfigInfo->mResourceOptype == -1) {
        return;
    }

    // Create the OpID Bitmap, this will serve as the key for the entry in mSystemIndependentLayerMappings.
    uint32_t resourceBitmap = 0;
    if(this->customerBit) {
        resourceBitmap |= (1 << 31);
    }

    resourceBitmap |= ((uint32_t)resourceConfigInfo->mResourceOpcode);
    resourceBitmap |= ((uint32_t)resourceConfigInfo->mResourceOptype << 16);

    this->mSystemIndependentLayerMappings[resourceBitmap] = this->mTotalResources;
    this->mResourceConfig[this->mTotalResources] = resourceConfigInfo;

    this->mTotalResources++;
}

void ResourceRegistry::displayResources() {
    for(int32_t i = 0; i < mTotalResources; i++) {
        auto& res = mResourceConfig[i];

        LOGI("URM_RESOURCE_PROCESSOR", "Resource Name: " + res->mResourceName);
        LOGI("URM_RESOURCE_PROCESSOR", "Optype: " + std::to_string(res->mResourceOptype));
        LOGI("URM_RESOURCE_PROCESSOR", "Opcode: " + std::to_string(res->mResourceOpcode));
        LOGI("URM_RESOURCE_PROCESSOR", "High Threshold: " + std::to_string(res->mHighThreshold));
        LOGI("URM_RESOURCE_PROCESSOR", "Low Threshold: " + std::to_string(res->mLowThreshold));

        if(res->resourceApplierCallback != nullptr) {
            LOGI("URM_RESOURCE_PROCESSOR", "BU has defined its own custom Resource Applier Function");
            res->resourceApplierCallback(nullptr);
        } else {
            LOGI("URM_RESOURCE_PROCESSOR", "No custom Resource Applier Specified, will use default one");
        }

        LOGI("URM_RESOURCE_PROCESSOR", "====================================");
    }
}

std::vector<ResourceConfigInfo*> ResourceRegistry::getRegisteredResources() {
    return mResourceConfig;
}

ResourceConfigInfo* ResourceRegistry::getResourceById(uint32_t resourceId) {
    if(this->mSystemIndependentLayerMappings.find(resourceId) == this->mSystemIndependentLayerMappings.end()) {
        return nullptr;
    }

    int32_t resourceTableIndex = this->mSystemIndependentLayerMappings[resourceId];
    return this->mResourceConfig[resourceTableIndex];
}

int32_t ResourceRegistry::getResourceTableIndex(uint32_t resourceId) {
    try {
        if(this->mSystemIndependentLayerMappings.find(resourceId) == this->mSystemIndependentLayerMappings.end()) {
            throw std::out_of_range("Index out of bounds");
        }
    } catch (const std::exception& e) {
        LOGE("URM_RESOURCE_PROCESSOR",
             "Resource ID not found in the registry");
        return -1;
    }

    return this->mSystemIndependentLayerMappings[resourceId];
}

int32_t ResourceRegistry::getTotalResourcesCount() {
    return this->mTotalResources;
}

void ResourceRegistry::pluginModifications(const std::vector<std::pair<int32_t, ResourceApplierCallback>>& modifiedResources) {
    for(std::pair<int32_t, ResourceApplierCallback> resource: modifiedResources) {
        this->mResourceConfig[resource.first]->resourceApplierCallback = resource.second;
    }
}

void writeToNode(const std::string& fName, int32_t fValue) {
    std::ofstream myFile(fName, std::ios::out | std::ios::trunc);

    if(!myFile.is_open()) {
        LOGD("URM_COCO_TABLE", "Failed to open file: "+ fName);
        return;
    }

    myFile << std::to_string(fValue);
    if(myFile.fail()) {
        LOGD("URM_COCO_TABLE", "Failed to write to file: "+ fName);
    }
    myFile.flush();
    myFile.close();
}

void ResourceRegistry::restoreResourcesToDefaultValues() {
    for(ResourceConfigInfo* resourceConfig: this->mResourceConfig) {
        int32_t defaultValue = resourceConfig->mDefaultValue;
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

    this->mResourceConfigInfo->resourceApplierCallback = nullptr;
    this->mResourceConfigInfo->mModes = 0;
}

ResourceConfigInfoBuilder* ResourceConfigInfoBuilder::setName(std::string name) {
    if(this->mResourceConfigInfo == nullptr) return this;

    this->mResourceConfigInfo->mResourceName = name;
    return this;
}

ResourceConfigInfoBuilder* ResourceConfigInfoBuilder::setOptype(std::string opTypeString) {
    if(this->mResourceConfigInfo == nullptr) return this;

    this->mResourceConfigInfo->mResourceOptype = -1;
    try {
        this->mResourceConfigInfo->mResourceOptype = (int8_t)stoi(opTypeString, nullptr, 0);
    } catch(const std::invalid_argument& ex) {
        LOGE("URM_RESOURCE_REGISTRY",
             "Resource Parsing Failed with error: " + std::string(ex.what()));
    } catch(const std::out_of_range& ex) {
        LOGE("URM_RESOURCE_REGISTRY",
             "Resource Parsing Failed with error: " + std::string(ex.what()));
    }
    return this;
}

ResourceConfigInfoBuilder* ResourceConfigInfoBuilder::setOpcode(std::string opCodeString) {
    if(this->mResourceConfigInfo == nullptr) return this;

    this->mResourceConfigInfo->mResourceOpcode = -1;
    try {
        this->mResourceConfigInfo->mResourceOpcode = (int16_t)stoi(opCodeString, nullptr, 0);
    } catch(const std::invalid_argument& ex) {
        LOGE("URM_SIGNAL_REGISTRY",
             "Signal Parsing Failed with error: " + std::string(ex.what()));
    } catch(const std::out_of_range& ex) {
        LOGE("URM_SIGNAL_REGISTRY",
             "Signal Parsing Failed with error: " + std::string(ex.what()));
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

ResourceConfigInfoBuilder* ResourceConfigInfoBuilder::setPermissions(std::string permissionString) {
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

ResourceConfigInfoBuilder* ResourceConfigInfoBuilder::setModes(std::string modeString) {
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

ResourceConfigInfoBuilder* ResourceConfigInfoBuilder::setPolicy(std::string policyString) {
    if(this->mResourceConfigInfo == nullptr) return this;

    enum Policy policy = INSTANT_APPLY;
    if(policyString == "higher_is_better") {
        policy = HIGHER_BETTER;
    } else if(policyString == "lower_is_better") {
        policy = LOWER_BETTER;
    } else if(policyString == "lazy_apply") {
        policy = LAZY_APPLY;
    } else {
        policy = INSTANT_APPLY;
    }
    this->mResourceConfigInfo->mPolicy = policy;
    return this;
}

ResourceConfigInfoBuilder* ResourceConfigInfoBuilder::setCoreLevelConflict(int8_t coreLevelConflict) {
    if(this->mResourceConfigInfo == nullptr) return this;

    this->mResourceConfigInfo->mCoreLevelConflict = coreLevelConflict;
    return this;
}

ResourceConfigInfoBuilder* ResourceConfigInfoBuilder::setDefaultValue(int32_t defaultValue) {
    if(this->mResourceConfigInfo == nullptr) return this;

    this->mResourceConfigInfo->mDefaultValue = defaultValue;
    return this;
}

ResourceConfigInfo* ResourceConfigInfoBuilder::build() {
    return this->mResourceConfigInfo;
}
