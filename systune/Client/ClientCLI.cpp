// Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
// SPDX-License-Identifier: BSD-3-Clause-Clear

#include <iostream>
#include <thread>
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <exception>
#include <sstream>
#include <memory>
#include <getopt.h>

#include "SystuneAPIs.h"

int8_t parseResources(const std::string& resources, std::vector<std::pair<uint32_t, int32_t>>& resourcePairs) {
    std::stringstream resourceStream(resources);
    std::string resourcePair;

    while(getline(resourceStream, resourcePair, ',')) {
        std::stringstream resourcePairStream(resourcePair);
        std::string resourcePairItem;
        std::pair<uint32_t, int32_t> resourcePair;

        int8_t index = 0;

        try {
            while(getline(resourcePairStream, resourcePairItem, ':')) {
                if(index > 2) {
                    return -1;
                }

                if(index == 0) {
                    resourcePair.first = std::stoi(resourcePairItem);
                } else {
                    resourcePair.second = std::stoi(resourcePairItem);
                }
                index++;
            }

        } catch(std::exception const& ex) {
            return -1;
        }

        resourcePairs.push_back(resourcePair);
    }
    return 0;
}

void sendTuneRequest(int64_t duration, int32_t priority, int32_t count, const std::string& resourceInfo) {
    std::vector<std::pair<uint32_t, int32_t>> resourcePairs;
    if(parseResources(resourceInfo, resourcePairs) == -1) {
        std::cout<<"Failed to parse Resource List"<<std::endl;
        return;
    }

    std::vector<Resource*>* resourceList = new std::vector<Resource*>();

    for(auto& resourcePair : resourcePairs) {
        Resource* resource = (Resource*) malloc(sizeof(Resource));
        resource->mOpCode = resourcePair.first;
        resource->mNumValues = 1;
        resource->mConfigValue.singleValue = resourcePair.second;

        resourceList->push_back(resource);
    }

    int64_t handle = tuneResources(duration, priority, count, resourceList);
    if(handle == -1) {
        std::cout<<"Failed to send Tune Request"<<std::endl;
    } else {
        std::cout<<"Handle Received from Server is: "<<handle<<std::endl;
    }
}

void sendRetuneRequest(int64_t handle, int64_t duration) {
    int8_t status = retuneResources(handle, duration);
    if(status == 0) {
        std::cout<<"Retune Request Successfully Submitted"<<std::endl;
    } else if(status == -1) {
        std::cout<<"Retune Request Could not be sent"<<std::endl;
    }
}

void sendUntuneRequest(int64_t handle) {
    int8_t status = untuneResources(handle);
    if(status == 0) {
        std::cout<<"Untune Request Successfully Submitted"<<std::endl;
    } else if(status == -1) {
        std::cout<<"Untune Request Could not be sent"<<std::endl;
    }
}

static int8_t processCommands() {
    std::string input;

    // read line
    std::getline(std::cin, input);

    if(input == "") {
        return true;
    }

    if(input == "exit" || input == "stop") {
        return false;
    }

    if(input == "help") {
        std::cout<<"Available commands: tune, retune, untune"<<std::endl;
        return true;
    }

    // break into tokens
    std::stringstream ss(input);
    std::vector<std::string> tokens;
    std::string token;
    while(std::getline(ss, token, ' ')) {
        tokens.push_back(token);
    }

    if(tokens[0] == "tune") {
        if(tokens.size() != 5) {
            std::cout<<"Invalid number of arguments for tune request"<<std::endl;
            std::cout<<"Usage: tune <duration> <priority> <numRes> <opcode>:<value>,<opcode>:<value>...."<<std::endl;
            std::cout<<"Example: tune 4000 3 2 1:1,2:2"<<std::endl;
            return true;
        }

        if(tokens[1].find(":") == std::string::npos) {
            std::cout<<"Invalid tune request"<<std::endl;
            std::cout<<"Usage: tune <duration> <priority> <numRes> <opcode>:<value>,<opcode>:<value>"<<std::endl;
            std::cout<<"Example: tune 4000 3 2 1:1,2:2"<<std::endl;
            return true;
        }

        int64_t duration = std::stoi(tokens[1]);
        int32_t priority = std::stoi(tokens[2]);
        int32_t count = std::stoi(tokens[3]);

        if(duration < -1 || duration == 0 || count <= 0) {
            std::cout<<"Invalid Params for Retune request" <<std::endl;
            std::cout<<"Usage: tune <duration> <priority> <numRes> <opcode>:<value>,<opcode>:<value>"<<std::endl;
            return true;
        } else {
            sendTuneRequest(duration, priority, count, tokens[4]);
        }


    } else if(tokens[0] == "untune") {
        if(tokens.size() != 2) {
            std::cout<<"Invalid number of arguments for Untune request" <<std::endl;
            std::cout<<"Usage: untune <handle>"<<std::endl;
            std::cout<<"Example: untune 4"<<std::endl;
            return true;
        }

        int64_t handle = std::stoi(tokens[1]);
        if(handle <= 0) {
            std::cout<<"Invalid Params for untune request" <<std::endl;
            std::cout<<"Usage: untune <handle>"<<std::endl;
            return true;

        } else {
            sendUntuneRequest(handle);
        }

        return true;

    } else if(tokens[0] == "retune") {
        if(tokens.size() != 3) {
            std::cout<<"Invalid number of arguments for Retune request"<<std::endl;
            std::cout<<"Usage: retune <handle> <duration>"<<std::endl;
            std::cout<<"Example: retune 1 5000" << std::endl;
            return true;
        }

        int64_t handle = std::stoi(tokens[1]);
        int32_t duration = std::stoi(tokens[2]);

        if(handle <= 0 || duration == 0 || duration < -1) {
            std::cout<<"Invalid Params for Retune request" <<std::endl;
            std::cout<<"Usage: retune <handle> <duration>"<<std::endl;
            return true;

        } else {
            sendRetuneRequest(handle, duration);
        }

        return true;

    } else {
        std::cout<<"Invalid command"<<std::endl;
        return true;
    }

    return true;
}

void startPersistentMode() {
    while(true) {
        try {
            if(!processCommands()) {
                return;
            }
        } catch(const std::bad_alloc& e) {
            std::cout<<"Invalid Command"<<std::endl;
        }
    }
}

int32_t main(int32_t argc, char* argv[]) {
    const char* short_prompts = "turd:p:l:n:h:s:";
    const struct option long_prompts[] = {
        {"tune", no_argument, nullptr, 't'},
        {"untune", no_argument, nullptr, 'u'},
        {"retune", no_argument, nullptr, 'r'},
        {"duration", required_argument, nullptr, 'd'},
        {"handle", required_argument, nullptr, 'h'},
        {"priority", required_argument, nullptr, 'p'},
        {"res", required_argument, nullptr, 'l'},
        {"num", required_argument, nullptr, 'n'},
        {"persistent", no_argument, nullptr, 's'},
        {nullptr, no_argument, nullptr, 0}
    };

    int32_t c;

    int8_t requestType = -1;
    int64_t handle = -1;
    int64_t duration = -1;
    int32_t priority = -1;
    int32_t numResources = -1;
    const char* resources = nullptr;
    int8_t persistent = false;

    while ((c = getopt_long(argc, argv, short_prompts, long_prompts, nullptr)) != -1) {
        switch (c) {
            case 't':
                requestType = REQ_RESOURCE_TUNING;
                break;
            case 'u':
                requestType = REQ_RESOURCE_UNTUNING;
                break;
            case 'r':
                requestType = REQ_RESOURCE_RETUNING;
                break;
            case 'd':
                duration = std::stoi(optarg);
                break;
            case 'p':
                priority = std::stoi(optarg);
                break;
            case 'h':
                handle = std::stoi(optarg);
                break;
            case 'l':
                resources = optarg;
                break;
            case 'n':
                numResources = std::stoi(optarg);
                break;
            case 's':
                persistent = true;
                break;
            default:
                break;
        }
    }

    if(persistent) {
        startPersistentMode();
        return 0;
    }

    switch(requestType) {
        case REQ_RESOURCE_TUNING:
            if(duration == 0 || duration < -1 || numResources <= 0 || priority == -1 ||
               resources == nullptr) {
                std::cout<<"Invalid Params for Tune Request"<<std::endl;
                std::cout << "Usage: --tune --duration <duration> --priority <priority> --num <numRes> -- res <opcode>:<value>,<opcode>:<value>" << std::endl;
                break;
            }
            if(resources != nullptr) {
                sendTuneRequest(duration, priority, numResources, resources);
            }
            break;

        case REQ_RESOURCE_RETUNING:
            if(duration == 0 || duration < -1 || handle <= 0) {
                std::cout<<"Invalid Params for Retune Request"<<std::endl;
                std::cout<<"Usage: --retune --handle <handle> --duration <duration>"<< std::endl;
                break;
            }
            sendRetuneRequest(handle, duration);
            break;

        case REQ_RESOURCE_UNTUNING:
            if(handle <= 0) {
                std::cout<<"Invalid Params for Untune request"<< std::endl;
                std::cout<<"Usage: --untune --handle <handle>"<< std::endl;
                break;
            }
            sendUntuneRequest(handle);
            break;

        default:
            return -1;
    }

    return 0;
}
