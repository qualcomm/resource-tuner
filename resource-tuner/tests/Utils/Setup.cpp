// Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
// SPDX-License-Identifier: BSD-3-Clause-Clear

#include "Extensions.h"

__attribute__((constructor))
void registerWithResourceTuner() {
    RESTUNE_REGISTER_CONFIG(RESOURCE_CONFIG, "/etc/resource-tuner/tests/configs/ResourcesConfig.yaml")
    RESTUNE_REGISTER_CONFIG(PROPERTIES_CONFIG, "/etc/resource-tuner/tests/configs/PropertiesConfig.yaml")
    RESTUNE_REGISTER_CONFIG(SIGNALS_CONFIG, "/etc/resource-tuner/tests/configs/SignalsConfig.yaml")
    RESTUNE_REGISTER_CONFIG(TARGET_CONFIG, "/etc/resource-tuner/tests/configs/TargetConfig.yaml")
    RESTUNE_REGISTER_CONFIG(INIT_CONFIG, "/etc/resource-tuner/tests/configs/InitConfig.yaml")
}
