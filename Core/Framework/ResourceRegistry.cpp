// Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
// SPDX-License-Identifier: BSD-3-Clause-Clear

#include "ResourceRegistry.h"
#include <iostream>

std::shared_ptr<ResourceRegistry> ResourceRegistry::resourceRegistryInstance = nullptr;

ResourceRegistry::ResourceRegistry() {
    this->customerBit = false;
    this->mTotalResources = 0;
}

void ResourceRegistry::initRegistry(int8_t customerBit) {
    this->customerBit = customerBit;
}

int8_t ResourceRegistry::isResourceConfigMalformed(ResourceConfigInfo* rConf) {
    if(rConf == nullptr) return true;
    if(rConf->mResourceOptype < 0 || rConf->mResourceOpcode < 0) return true;
    if(rConf->mHighThreshold < 0 || rConf->mLowThreshold < 0) return true;
    return false;
}

void ResourceRegistry::registerResource(ResourceConfigInfo* resourceConfigInfo) {
    // Invalid Resource, skip.
    if(this->isResourceConfigMalformed(resourceConfigInfo)) {
        delete resourceConfigInfo;
        return;
    }

    // Persist the Default Values of the Resources in a File.
    // These values will be used to restore the Sysfs nodes in case the Server Process crashes.
    std::fstream sysfsPersistenceFile("../sysfsOriginalValues.txt", std::ios::out | std::ios::app);
    std::string resourceData = resourceConfigInfo->mResourceName;
    resourceData.push_back(',');
    resourceData.append(std::to_string(resourceConfigInfo->mDefaultValue));
    resourceData.push_back('\n');
    sysfsPersistenceFile << resourceData;

    // Create the OpID Bitmap, this will serve as the key for the entry in mSystemIndependentLayerMappings.
    uint32_t resourceBitmap = 0;
    if(this->customerBit) {
        resourceBitmap |= (1 << 31);
    }

    resourceBitmap |= ((uint32_t)resourceConfigInfo->mResourceOpcode);
    resourceBitmap |= ((uint32_t)resourceConfigInfo->mResourceOptype << 16);

    this->mSystemIndependentLayerMappings[resourceBitmap] = this->mTotalResources;
    this->mResourceConfig.push_back(resourceConfigInfo);

    this->mTotalResources++;
}

void ResourceRegistry::displayResources() {
    for(int32_t i = 0; i < mTotalResources; i++) {
        auto& res = mResourceConfig[i];

        LOGI("RTN_RESOURCE_PROCESSOR", "Resource Name: " + res->mResourceName);
        LOGI("RTN_RESOURCE_PROCESSOR", "Optype: " + std::to_string(res->mResourceOptype));
        LOGI("RTN_RESOURCE_PROCESSOR", "Opcode: " + std::to_string(res->mResourceOpcode));
        LOGI("RTN_RESOURCE_PROCESSOR", "High Threshold: " + std::to_string(res->mHighThreshold));
        LOGI("RTN_RESOURCE_PROCESSOR", "Low Threshold: " + std::to_string(res->mLowThreshold));

        if(res->resourceApplierCallback != nullptr) {
            LOGI("RTN_RESOURCE_PROCESSOR", "BU has defined its own custom Resource Applier Function");
            res->resourceApplierCallback(nullptr);
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
    } catch(const std::exception& e) {
        LOGE("RTN_RESOURCE_PROCESSOR",
             "Resource ID not found in the registry");
        return -1;
    }

    return this->mSystemIndependentLayerMappings[resourceId];
}

int32_t ResourceRegistry::getTotalResourcesCount() {
    return this->mTotalResources;
}

void ResourceRegistry::pluginModifications(const std::vector<std::pair<uint32_t, ResourceApplierCallback>>& modifiedResources) {
    for(std::pair<uint32_t, ResourceApplierCallback> resource: modifiedResources) {
        int32_t resourceTableIndex = this->getResourceTableIndex(resource.first);
        if(resourceTableIndex != -1) {
            this->mResourceConfig[resourceTableIndex]->resourceApplierCallback = resource.second;
        }
    }
}

void writeToNode(const std::string& fName, int32_t fValue) {
    std::ofstream myFile(fName, std::ios::out | std::ios::trunc);

    if(!myFile.is_open()) {
        LOGD("RTN_COCO_TABLE", "Failed to open file: "+ fName);
        return;
    }

    myFile << std::to_string(fValue);
    if(myFile.fail()) {
        LOGD("RTN_COCO_TABLE", "Failed to write to file: "+ fName);
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

ResourceConfigInfoBuilder* ResourceConfigInfoBuilder::setName(const std::string& name) {
    if(this->mResourceConfigInfo == nullptr) return this;

    this->mResourceConfigInfo->mResourceName = name;
    return this;
}

ResourceConfigInfoBuilder* ResourceConfigInfoBuilder::setOptype(const std::string& opTypeString) {
    if(this->mResourceConfigInfo == nullptr) return this;

    this->mResourceConfigInfo->mResourceOptype = -1;
    try {
        this->mResourceConfigInfo->mResourceOptype = (int8_t)stoi(opTypeString, nullptr, 0);
    } catch(const std::invalid_argument& ex) {
        LOGE("RTN_RESOURCE_REGISTRY",
             "Resource Parsing Failed with error: " + std::string(ex.what()));
    } catch(const std::out_of_range& ex) {
        LOGE("RTN_RESOURCE_REGISTRY",
             "Resource Parsing Failed with error: " + std::string(ex.what()));
    }
    return this;
}

ResourceConfigInfoBuilder* ResourceConfigInfoBuilder::setOpcode(const std::string& opCodeString) {
    if(this->mResourceConfigInfo == nullptr) return this;

    this->mResourceConfigInfo->mResourceOpcode = -1;
    try {
        this->mResourceConfigInfo->mResourceOpcode = (int16_t)stoi(opCodeString, nullptr, 0);
    } catch(const std::invalid_argument& ex) {
        LOGE("RTN_SIGNAL_REGISTRY",
             "Signal Parsing Failed with error: " + std::string(ex.what()));
    } catch(const std::out_of_range& ex) {
        LOGE("RTN_SIGNAL_REGISTRY",
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

ResourceConfigInfoBuilder* ResourceConfigInfoBuilder::setCoreLevelConflict(int8_t coreLevelConflict) {
    if(this->mResourceConfigInfo == nullptr) return this;

    this->mResourceConfigInfo->mCoreLevelConflict = coreLevelConflict;
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

ResourceConfigInfoBuilder* ResourceConfigInfoBuilder::setDefaultValue(int32_t defaultValue) {
    if(this->mResourceConfigInfo == nullptr) return this;

    this->mResourceConfigInfo->mDefaultValue = defaultValue;
    return this;
}

ResourceConfigInfo* ResourceConfigInfoBuilder::build() {
    return this->mResourceConfigInfo;
}
