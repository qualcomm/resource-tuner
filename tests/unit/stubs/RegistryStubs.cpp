// Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
// SPDX-License-Identifier: BSD-3-Clause-Clear


// -------------- TargetRegistry stubs --------------
std::shared_ptr<TargetRegistry> TargetRegistry::targetRegistryInstance = nullptr;
TargetRegistry::TargetRegistry() {}
TargetRegistry::~TargetRegistry() {}

void TargetRegistry::getClusterIDs(std::vector<int32_t>& out) { 
    out.clear();        // test: no clusters 
}
void TargetRegistry::getCGroupNames(std::vector<std::string>& out) {
    out.clear();        // test: no cgroups
}

// -------------- Default lifecycle callbacks --------------
void defaultClusterLevelApplierCb(void*) {}
void defaultClusterLevelTearCb(void*) {}
void defaultCoreLevelApplierCb(void*) {}
void defaultCoreLevelTearCb(void*) {}
void defaultCGroupLevelApplierCb(void*) {}
void defaultCGroupLevelTearCb(void*) {}
void defaultGlobalLevelApplierCb(void*) {}
void defaultGlobalLevelTearCb(void*) {}

