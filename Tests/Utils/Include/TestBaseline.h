// Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
// SPDX-License-Identifier: BSD-3-Clause-Clear

#ifndef TEST_BASELINE_H
#define TEST_BASELINE_H

#include <unordered_map>

#include "YamlParser.h"
#include "ResourceTunerSettings.h"

#define TEST_ROOT "TestConfigs"
#define TARGET_NAME_LIST "TargetName"
#define CLUSTER_EXPECTATIONS "ClusterExpectations"
#define TARGET_CLUSTER_INFO_LOGICAL_ID "LgcId"
#define TARGET_CLUSTER_INFO_PHYSICAL_ID "PhyId"
#define NUM_CLUSERS "NumClusters"
#define NUM_CORES "NumCores"

const static std::string baselineYamlFilePath = "/etc/resource-tuner/custom/Baseline.yaml";

typedef struct {
    int32_t mLogicalID;
    int32_t mPhysicalID;
} ClusterExpection;

class ClusterExpectationBuilder {
private:
    ClusterExpection* mClusterExpectation;

public:
    ClusterExpectationBuilder() {
        this->mClusterExpectation = new(std::nothrow) ClusterExpection;
    }

    ErrCode setLogicalID(const std::string& logicalIDString) {
        this->mClusterExpectation->mLogicalID = -1;
        try {
            this->mClusterExpectation->mLogicalID = std::stoi(logicalIDString);
            return RC_SUCCESS;

        } catch(const std::exception& e) {
            return RC_INVALID_VALUE;
        }

        return RC_INVALID_VALUE;
    }

    ErrCode setPhysicalID(const std::string& physicalIDString) {
        this->mClusterExpectation->mPhysicalID = -1;
        try {
            this->mClusterExpectation->mPhysicalID = std::stoi(physicalIDString);
            return RC_SUCCESS;

        } catch(const std::exception& e) {
            return RC_INVALID_VALUE;
        }

        return RC_INVALID_VALUE;
    }

    ClusterExpection* build() {
        return this->mClusterExpectation;
    }
};

class TestBaseline {
private:
    std::unordered_map<int32_t, int32_t> mLogicalToPhysicalClusterMapping;
    int32_t mTotalClusterCount;
    int32_t mTotalCoreCount;

    ErrCode parseTestConfigYamlNode(const std::string& filePath) {
        int8_t isConfigForCurrentTarget = false;
        // Check if there exists a Target Config for this particular target in the Common Configs.
        // Skip this check if the BU has provided their own Target Configs
        std::string currTargetName = AuxRoutines::readFromFile("/sys/devices/soc0/machine");

        FILE* targetConfigFile = fopen(filePath.c_str(), "r");
        if(targetConfigFile == nullptr) {
            return RC_FILE_NOT_FOUND;
        }

        yaml_parser_t parser;
        yaml_event_t event;

        if(!yaml_parser_initialize(&parser)) {
            fclose(targetConfigFile);
            return RC_YAML_PARSING_ERROR;
        }

        yaml_parser_set_input_file(&parser, targetConfigFile);

        int8_t parsingDone = false;
        int8_t insideBaselineConfigs = false;
        int8_t inTargetNamesList = false;
        int8_t inClusterExpList = false;

        std::string value;
        std::string currentKey;

        ClusterExpectationBuilder* clusterExpectationBuilder = nullptr;

        while(!parsingDone) {
            if(!yaml_parser_parse(&parser, &event)) {
                return RC_YAML_PARSING_ERROR;
            }

            switch(event.type) {
                case YAML_STREAM_END_EVENT:
                    parsingDone = true;
                    break;

                case YAML_SEQUENCE_START_EVENT:
                    std::cout<<"YAML_SEQUENCE_START_EVENT, with currentKey = "<<currentKey<<std::endl;
                    if(currentKey == TARGET_NAME_LIST) {
                        inTargetNamesList = true;
                    } else if(currentKey == CLUSTER_EXPECTATIONS) {
                        inClusterExpList = true;
                    }
                    break;

                case YAML_SEQUENCE_END_EVENT:
                    std::cout<<"YAML_SEQUENCE_END_EVENT called with currentKey = "<<currentKey<<std::endl;
                    if(inTargetNamesList) {
                        inTargetNamesList = false;
                    } else if(inClusterExpList) {
                        inClusterExpList = false;
                    }

                    currentKey.clear();
                    break;

                case YAML_MAPPING_START_EVENT:
                    if(!insideBaselineConfigs) {
                        insideBaselineConfigs = true;
                    } else if(inClusterExpList) {
                        clusterExpectationBuilder = new(std::nothrow) ClusterExpectationBuilder;
                    }

                    currentKey.clear();
                    break;

                case YAML_MAPPING_END_EVENT:
                    if(inClusterExpList) {
                        if(isConfigForCurrentTarget) {
                            try {
                                std::cout<<"Adding cluster expectation: "<<std::endl;
                                ClusterExpection* info = clusterExpectationBuilder->build();
                                this->mLogicalToPhysicalClusterMapping[info->mLogicalID] = info->mPhysicalID;
                                delete clusterExpectationBuilder;

                            } catch(const std::exception& e) {}
                        }
                    }

                    currentKey.clear();
                    break;

                case YAML_SCALAR_EVENT:
                    if(event.data.scalar.value != nullptr) {
                        value = reinterpret_cast<char*>(event.data.scalar.value);
                    }

                    if(value == TEST_ROOT) {
                        break;
                    }

                    if(currentKey.length() == 0) {
                        currentKey = value;
                        std::cout<<"currentKey updated to: "<<currentKey<<std::endl;
                    } else {
                        if(inTargetNamesList) {
                            if(value == "*" || value == currTargetName) {
                                isConfigForCurrentTarget = true;
                            }
                            break;

                        } else if(inClusterExpList) {
                            if(currentKey == TARGET_CLUSTER_INFO_LOGICAL_ID) {
                                clusterExpectationBuilder->setLogicalID(value);
                            } else if(currentKey == TARGET_CLUSTER_INFO_PHYSICAL_ID) {
                                clusterExpectationBuilder->setPhysicalID(value);
                            }

                        } else {
                            if(currentKey == NUM_CLUSERS) {
                                if(isConfigForCurrentTarget) {
                                    this->mTotalClusterCount = std::stoi(value);
                                }
                            } else if(currentKey == NUM_CORES) {
                                if(isConfigForCurrentTarget) {
                                    this->mTotalCoreCount = std::stoi(value);
                                }
                            }
                        }
                        currentKey.clear();
                    }

                default:
                    break;
            }
            yaml_event_delete(&event);
        }

        yaml_parser_delete(&parser);
        fclose(targetConfigFile);

        return RC_SUCCESS;
    }

public:
    ErrCode fetchBaseline() {
        return parseTestConfigYamlNode(baselineYamlFilePath);
    }

    void displayBaseline() {
        std::cout<<"Total Cluster Count: "<<this->getExpectedClusterCount()<<std::endl;
        std::cout<<"Total Core Count: "<<this->getExpectedCoreCount()<<std::endl;

        for(auto it = this->mLogicalToPhysicalClusterMapping.begin();
             it != this->mLogicalToPhysicalClusterMapping.end(); ++it) {
            std::cout<<"Logical ID "<<it->first<<" mapped to Physical ID: "<<it->second<<std::endl;
        }
    }

    int32_t getExpectedClusterCount() {
        return this->mTotalClusterCount;
    }

    int32_t getExpectedCoreCount() {
        return this->mTotalCoreCount;
    }

    int32_t getExpectedPhysicalCluster(int32_t logicalID) {
        if(this->mLogicalToPhysicalClusterMapping.find(logicalID) ==
           this->mLogicalToPhysicalClusterMapping.end()) {
            return -1;
        }

        return this->mLogicalToPhysicalClusterMapping[logicalID];
    }
};

#endif
