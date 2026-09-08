// Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
// SPDX-License-Identifier: BSD-3-Clause

#include <getopt.h>

#include "URMTests.h"
#include "Extensions.h"

#define TEST_CLASS "COMPONENT"

URM_REGISTER_CONFIG(RESOURCE_CONFIG, "/usr/share/urm/tests/configs/ResourcesConfig.yaml")
URM_REGISTER_CONFIG(PROPERTIES_CONFIG, "/usr/share/urm/tests/configs/PropertiesConfig.yaml")
URM_REGISTER_CONFIG(SIGNALS_CONFIG, "/usr/share/urm/tests/configs/SignalsConfig.yaml")
URM_REGISTER_CONFIG(TARGET_CONFIG, "/usr/share/urm/tests/configs/TargetConfig.yaml")
URM_REGISTER_CONFIG(INIT_CONFIG, "/usr/share/urm/tests/configs/InitConfig.yaml")
URM_REGISTER_CONFIG(APP_CONFIG, "/usr/share/urm/tests/configs/PerApp.yaml")


int32_t main(int32_t argc, char* argv[]) {
    const char* shortPrompts = "hp:";
    const struct option longPrompts[] = {
        {"help", no_argument, nullptr, 'h'},
        {"npath", required_argument, nullptr, 'p'},
        {nullptr, no_argument, nullptr, 0}
    };

    std::string nodesPath = "";

    int32_t c;
    while((c = getopt_long(argc, argv, shortPrompts, longPrompts, nullptr)) != -1) {
        switch(c) {
            case 'h':
                std::cout<<"This suite tests individual URM components."<<std::endl;
                std::cout<<"Usage: <path_to_binary> [--npath <path_to_custom_test_nodes>]"<<std::endl;
                std::cout<<"Example: /usr/bin/UrmComponentTests"<<std::endl;
                std::cout<<"Or: /usr/bin/UrmComponentTests --npath \"/run/urm/tests/nodes\""<<std::endl;
                return 0;
            case 'p':
                nodesPath = optarg;
                break;
            default:
                break;
        }
    }

    if(nodesPath.length() > 0) {
        TestAggregator::setBaseTestNodePath(nodesPath);
    }

    return TestAggregator::runAll(TEST_CLASS);
}
