// Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
// SPDX-License-Identifier: BSD-3-Clause-Clear

#include "AuxRoutines.h"

std::string AuxRoutines::readFromFile(const std::string& fileName) {
    std::ifstream fileStream(fileName, std::ios::in);
    std::string value = "";

    if(!fileStream.is_open()) {
        LOGE("RESTUNE_AUX_ROUTINE", "Failed to open file: " + fileName + " Error: " + strerror(errno));
        return "";
    }

    if(!getline(fileStream, value)) {
        LOGE("RESTUNE_AUX_ROUTINE", "Failed to read from file: " + fileName + " Error: " + strerror(errno));
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

    file.open(ResourceTunerSettings::mPersistenceFile);
    if(!file.is_open()) {
        LOGE("RESTUNE_SERVER_INIT",
             "Failed to open sysfs original values file: " + ResourceTunerSettings::mPersistenceFile);
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

int8_t AuxRoutines::fileExists(const std::string& filePath) {
    return access(filePath.c_str(), F_OK) == 0;
}

int32_t AuxRoutines::createProcess() {
    return fork();
}

std::string AuxRoutines::getMachineName() {
    return AuxRoutines::readFromFile(ResourceTunerSettings::mDeviceNamePath);
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
    LOGD(LOG_TAG, "Background Processing Enabled?: " + std::to_string((int32_t)clientReq->getProcessingModes()));
    LOGD(LOG_TAG, "Number of Resources: " + std::to_string(clientReq->getResourcesCount()));

}

void AuxRoutines::dumpRequest(Signal* clientReq) {
    std::string LOG_TAG = "RESTUNE_SERVER";
    LOGD(LOG_TAG, "Print Signal details:");

    LOGD(LOG_TAG, "Print Signal Request");
    LOGD(LOG_TAG, "Signal ID: " + std::to_string(clientReq->getSignalCode()));
    LOGD(LOG_TAG, "Handle: " + std::to_string(clientReq->getHandle()));
    LOGD(LOG_TAG, "Duration: " + std::to_string(clientReq->getDuration()));
    LOGD(LOG_TAG, "App Name: " + std::string(clientReq->getAppName()));
    LOGD(LOG_TAG, "Scenario: " + std::string(clientReq->getScenario()));
    LOGD(LOG_TAG, "Num Args: " + std::to_string(clientReq->getNumArgs()));
    LOGD(LOG_TAG, "Priority: " + std::to_string(clientReq->getPriority()));
}

int64_t AuxRoutines::generateUniqueHandle() {
    static int64_t handleGenerator = 0;
    OperationStatus opStatus;
    int64_t nextHandle = Add(handleGenerator, (int64_t)1, opStatus);
    if(opStatus == SUCCESS) {
        handleGenerator = nextHandle;
        return nextHandle;
    }
    return -1;
}

int64_t AuxRoutines::getCurrentTimeInMilliseconds() {
    auto now = std::chrono::system_clock::now();
    return std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()).count();
}

FlatBuffEncoder::FlatBuffEncoder() {
    this->mBuffer = nullptr;
    this->mCurPtr = nullptr;
    this->mRunningIndex = 0;
}

void FlatBuffEncoder::setBuf(char* buffer) {
    this->mBuffer = buffer;
    this->mCurPtr = buffer;
    this->mRunningIndex = 0;
}

FlatBuffEncoder FlatBuffEncoder::appendString(const char* valStr) {
    if(this->mRunningIndex == -1 || this->mBuffer == nullptr) {
        return *this;
    }

    const char* charIterator = valStr;
    char* charPointer = reinterpret_cast<char*>(this->mCurPtr);

    while(*charIterator != '\0') {
        if(this->mRunningIndex != -1 && this->mRunningIndex + 1 < REQ_BUFFER_SIZE) {
            try {
                ASSIGN_AND_INCR(charPointer, *charIterator);
                this->mRunningIndex++;
                this->mCurPtr = reinterpret_cast<char*>(charPointer);

            } catch(const std::exception& e) {
                this->mRunningIndex = -1;
                break;
            }
        } else {
            // Prevent further updates on the current buffer
            this->mRunningIndex = REQ_BUFFER_SIZE;
            break;
        }

        charIterator++;
    }

    if(this->mRunningIndex >= 0 && this->mRunningIndex < REQ_BUFFER_SIZE) {
        return this->append<char>('\0');
    }

    return *this;
}

int8_t FlatBuffEncoder::isBufSane() {
    if(this->mRunningIndex >= REQ_BUFFER_SIZE || this->mBuffer == nullptr) {
        return false;
    }
    return true;
}
