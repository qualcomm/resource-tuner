// Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
// SPDX-License-Identifier: BSD-3-Clause-Clear

#include "UrmSettings.h"

int32_t UrmSettings::serverOnlineStatus = false;
MetaConfigs UrmSettings::metaConfigs{};
TargetConfigs UrmSettings::targetConfigs{};

const std::string UrmSettings::mCommonResourceFilePath =
                                    "/etc/resource-tuner/common/ResourcesConfig.yaml";
const std::string UrmSettings::mCustomResourceFilePath =
                                    "/etc/resource-tuner/custom/ResourcesConfig.yaml";

const std::string UrmSettings::mCommonSignalFilePath =
                                    "/etc/resource-tuner/common/SignalsConfig.yaml";
const std::string UrmSettings::mCustomSignalFilePath =
                                    "/etc/resource-tuner/custom/SignalsConfig.yaml";

const std::string UrmSettings::mCommonInitConfigFilePath =
                                    "/etc/resource-tuner/common/InitConfig.yaml";
const std::string UrmSettings::mCustomInitConfigFilePath =
                                    "/etc/resource-tuner/custom/InitConfig.yaml";

const std::string UrmSettings::mCommonPropertiesFilePath =
                                    "/etc/resource-tuner/common/PropertiesConfig.yaml";
const std::string UrmSettings::mCustomPropertiesFilePath =
                                    "/etc/resource-tuner/custom/PropertiesConfig.yaml";

const std::string UrmSettings::mCustomTargetFilePath =
                                    "/etc/resource-tuner/custom/TargetConfig.yaml";

const std::string UrmSettings::mCustomExtFeaturesFilePath =
                                    "/etc/resource-tuner/custom/ExtFeaturesConfig.yaml";

const std::string UrmSettings::mExtensionsPluginLibPath = "libRestunePlugin.so";

const std::string UrmSettings::mDeviceNamePath =
                                    "/sys/devices/soc0/machine";

const std::string UrmSettings::mBaseCGroupPath =
                                    "/sys/fs/cgroup/";

const std::string UrmSettings::mPersistenceFile =
                                    "/etc/resource-tuner/data/resource_original_values.txt";

int32_t UrmSettings::isServerOnline() {
    return serverOnlineStatus;
}

void UrmSettings::setServerOnlineStatus(int32_t isOnline) {
    serverOnlineStatus = isOnline;
}
