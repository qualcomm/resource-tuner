// Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
// SPDX-License-Identifier: BSD-3-Clause-Clear

#include "AuxRoutines.h"
#include <cstring>

std::string AuxRoutines::readFromFile(const std::string& fileName) {
    std::ifstream fileStream(fileName, std::ios::in);
    std::string value = "";

    if(!fileStream.is_open()) {
        LOGE("RESTUNE_AUX_ROUTINE", "Failed to open file: " + fileName + " Error: " + strerror(errno));
        return "";
    }

    if(!getline(fileStream, value)) {
        LOGE("RESTUNE_AUX_ROUTINE", "Failed to read from file: " + fileName);
        return "";
    }

    fileStream.close();
    return value;
}

void AuxRoutines::writeToFile(const std::string& fileName, const std::string& value) {
    std::ofstream fileStream(fileName, std::ios::out | std::ios::trunc);

    if(!fileStream.is_open()) {
        LOGD("RESTUNE_AUX_ROUTINE", "Failed to open file: " + fileName + " Error: " + strerror(errno));
        return;
    }

    fileStream<<value;

    if(fileStream.fail()) {
        LOGD("RESTUNE_AUX_ROUTINE", "Failed to write to file: "+ fileName + " Error: " + strerror(errno));
    }

    fileStream.flush();
    fileStream.close();
}

void AuxRoutines::writeSysFsDefaults() {
    // Write Defaults
    std::ifstream file;

    file.open("sysfsOriginalValues.txt");
    if(!file.is_open()) {
        LOGE("RESTUNE_SERVER_INIT", "Failed to open sysfs original values file: sysfsOriginalValues.txt");
        return;
    }

    std::string line;
    while(std::getline(file, line)) {
        std::stringstream lineStream(line);
        std::string token;

        int8_t index = 0;
        std::string sysfsNodePath = "";
        int32_t sysfsNodeDefaultValue = -1;

        while(std::getline(lineStream, token, ',')) {
            if(index == 0) {
                sysfsNodePath = token;
            } else if(index == 1) {
                try {
                    sysfsNodeDefaultValue = std::stoi(token);
                } catch(const std::exception& e) {}
            }
            index++;
        }

        if(sysfsNodePath.length() > 0 && sysfsNodeDefaultValue != -1) {
            AuxRoutines::writeToFile(sysfsNodePath, std::to_string(sysfsNodeDefaultValue));
        }
    }
}

void AuxRoutines::deleteFile(const std::string& fileName) {
    remove(fileName.c_str());
}

void dumpRequest(Request* clientReq) {
    std::string LOG_TAG = "RESTUNE_SERVER";

    LOGD(LOG_TAG, "Request details:");
    LOGD(LOG_TAG, "reqType: " + std::to_string(clientReq->getRequestType()));
    LOGD(LOG_TAG, "handle: " + std::to_string(clientReq->getHandle()));
    LOGD(LOG_TAG, "Duration: " + std::to_string(clientReq->getDuration()));
    LOGD(LOG_TAG, "Priority: " + std::to_string(clientReq->getPriority()));
    LOGD(LOG_TAG, "client PID: " +std::to_string(clientReq->getClientPID()));
    LOGD(LOG_TAG, "client TID: " + std::to_string(clientReq->getClientTID()));
    LOGD(LOG_TAG, "Background Processing Enabled?: " + std::to_string((int32_t)clientReq->isBackgroundProcessingEnabled()));
    LOGD(LOG_TAG, "Number of Resources: " + std::to_string(clientReq->getResourcesCount()));

    LOGD(LOG_TAG, "Values for resources are as:");

    for(int32_t i = 0; i < clientReq->getResourcesCount(); i++) {
        Resource* res = clientReq->getResourceAt(i);
        LOGD(LOG_TAG, "Resource " + std::to_string(i + 1) + ":");
        LOGD(LOG_TAG, "ResCode: " + std::to_string(res->getResCode()));
        LOGD(LOG_TAG, "Number of Values: " + std::to_string(res->getValuesCount()));
        // LOGD(LOG_TAG, "-- Single Value: " + std::to_string(res->mResValue.value));
    }
}

void AuxRoutines::dumpRequest(Signal* clientReq) {
    std::string LOG_TAG = "RESTUNE_SERVER";
    LOGD(LOG_TAG, "Print Signal details:");

    LOGD(LOG_TAG, "Print Signal Request");
    LOGD(LOG_TAG, "Signal ID: " + std::to_string(clientReq->getSignalID()));
    LOGD(LOG_TAG, "Handle: " + std::to_string(clientReq->getHandle()));
    LOGD(LOG_TAG, "Duration: " + std::to_string(clientReq->getDuration()));
    LOGD(LOG_TAG, "App Name: " + std::string(clientReq->getAppName()));
    LOGD(LOG_TAG, "Scenario: " + std::string(clientReq->getScenario()));
    LOGD(LOG_TAG, "Num Args: " + std::to_string(clientReq->getNumArgs()));
    LOGD(LOG_TAG, "Priority: " + std::to_string(clientReq->getPriority()));
}

std::string AuxRoutines::requestTypeToString(int32_t requestType) {
    switch(requestType) {
        case REQ_RESOURCE_TUNING:
            return "Resource Tuning";
        case REQ_RESOURCE_UNTUNING:
            return "Resource Untuning";
        case REQ_RESOURCE_RETUNING:
            return "Resource Retuning";
        case SIGNAL_ACQ:
            return "Tune Signal";
        case SIGNAL_FREE:
            return "Untune Signal";
        case REQ_SYSCONFIG_GET_PROP:
            return "Get Property";
        case REQ_SYSCONFIG_SET_PROP:
            return "Set Property";
        default:
            break;
    }

    return "UNKNOWN";
}
