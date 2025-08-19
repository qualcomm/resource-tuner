// Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
// SPDX-License-Identifier: BSD-3-Clause-Clear

#include "ResourceTunerSettings.h"

std::shared_timed_mutex ResourceTunerSettings::mModeLock{};
int32_t ResourceTunerSettings::serverOnlineStatus = false;
int8_t ResourceTunerSettings::serverInTestMode = false;
MetaConfigs ResourceTunerSettings::metaConfigs{};
TargetConfigs ResourceTunerSettings::targetConfigs{};

const std::string ResourceTunerSettings::mCommonResourceFilePath =
                                    "/etc/ResourceTuner/Common/ResourcesConfig.yaml";
const std::string ResourceTunerSettings::mInitConfigFilePath =
                                    "/etc/ResourceTuner/Common/InitConfig.yaml";
const std::string ResourceTunerSettings::mCommonSignalFilePath =
                                    "/etc/ResourceTuner/Common/SignalsConfig.yaml";
const std::string ResourceTunerSettings::mTargetSpecificResourceFilePath =
                                    "/etc/ResourceTuner/ResourcesConfig.yaml";
const std::string ResourceTunerSettings::mTargetSpecificSignalFilePath =
                                    "/etc/ResourceTuner/SignalsConfig.yaml";
const std::string ResourceTunerSettings::mPropertiesFilePath =
                                    "/etc/ResourceTuner/Common/PropertiesConfig.yaml";

int32_t ResourceTunerSettings::isServerOnline() {
    return serverOnlineStatus;
}

void ResourceTunerSettings::setServerOnlineStatus(int32_t isOnline) {
    serverOnlineStatus = isOnline;
}

int64_t ResourceTunerSettings::generateUniqueHandle() {
    static int64_t handleGenerator = 0;
    OperationStatus opStatus;
    int64_t nextHandle = Add(handleGenerator, (int64_t)1, opStatus);
    if(opStatus == SUCCESS) {
        handleGenerator = nextHandle;
        return nextHandle;
    }
    return -1;
}

int64_t ResourceTunerSettings::getCurrentTimeInMilliseconds() {
    auto now = std::chrono::system_clock::now();
    return std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()).count();
}
