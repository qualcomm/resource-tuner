// Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
// SPDX-License-Identifier: BSD-3-Clause-Clear

#include "ResourceTunerSettings.h"

std::shared_timed_mutex ResourceTunerSettings::mModeLock{};
int32_t ResourceTunerSettings::serverOnlineStatus = false;
MetaConfigs ResourceTunerSettings::metaConfigs{};
TargetConfigs ResourceTunerSettings::targetConfigs{};

const std::string ResourceTunerSettings::mCommonResourceFilePath =
                                    "/etc/resource-tuner/common/ResourcesConfig.yaml";
const std::string ResourceTunerSettings::mCustomResourceFilePath =
                                    "/etc/resource-tuner/custom/ResourcesConfig.yaml";

const std::string ResourceTunerSettings::mCommonSignalFilePath =
                                    "/etc/resource-tuner/common/SignalsConfig.yaml";
const std::string ResourceTunerSettings::mCustomSignalFilePath =
                                    "/etc/resource-tuner/custom/SignalsConfig.yaml";

const std::string ResourceTunerSettings::mCommonInitConfigFilePath =
                                    "/etc/resource-tuner/common/InitConfig.yaml";
const std::string ResourceTunerSettings::mCustomInitConfigFilePath =
                                    "/etc/resource-tuner/custom/InitConfig.yaml";

const std::string ResourceTunerSettings::mCommonPropertiesFilePath =
                                    "/etc/resource-tuner/common/PropertiesConfig.yaml";
const std::string ResourceTunerSettings::mCustomPropertiesFilePath =
                                    "/etc/resource-tuner/custom/PropertiesConfig.yaml";

const std::string ResourceTunerSettings::mCustomExtFeaturesFilePath =
                                    "/etc/resource-tuner/custom/ExtFeaturesConfig.yaml";

const std::string ResourceTunerSettings::mCustomTargetFilePath =
                                    "/etc/resource-tuner/custom/TargetConfig.yaml";

const std::string ResourceTunerSettings::mTestResourceFilePath =
                                    "/etc/resource-tuner/tests/Configs/ResourcesConfigA.yaml";
const std::string ResourceTunerSettings::mTestSignalFilePath =
                                    "/etc/resource-tuner/tests/Configs/SignalsConfigA.yaml";
const std::string ResourceTunerSettings::mTestPropertiesFilePath =
                                    "/etc/resource-tuner/tests/Configs/PropertiesConfig.yaml";
const std::string ResourceTunerSettings::mTestTargetConfigFilePath =
                                    "/etc/resource-tuner/tests/Configs/TargetConfig.yaml";
const std::string ResourceTunerSettings::mTestInitConfigFilePath =
                                    "/etc/resource-tuner/tests/Configs/InitConfig.yaml";

const std::string ResourceTunerSettings::mExtensionsPluginLibPath =
                                    "/etc/resource-tuner/custom/libPlugin.so";

const std::string ResourceTunerSettings::mBaseCGroupPath =
                                    "/sys/fs/cgroup/";

int32_t ResourceTunerSettings::isServerOnline() {
    return serverOnlineStatus;
}

void ResourceTunerSettings::setServerOnlineStatus(int32_t isOnline) {
    serverOnlineStatus = isOnline;
}
