// Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
// SPDX-License-Identifier: BSD-3-Clause-Clear

#include "Signal.h"

Signal::Signal() {}

uint32_t Signal::getSignalID() {
    return this->mSignalID;
}

int32_t Signal::getNumArgs() {
    return this->mNumArgs;
}

const char* Signal::getAppName() {
    return this->mAppName;
}

const char* Signal::getScenario() {
    return this->mScenario;
}

std::vector<uint32_t>* Signal::getListArgs() {
    return this->mListArgs;
}

uint32_t Signal::getListArgAt(int32_t index) {
    return (*this->mListArgs)[index];
}

void Signal::setSignalID(uint32_t signalID) {
    this->mSignalID = signalID;
}

void Signal::setAppName(const char* appName) {
    this->mAppName = appName;
}

void Signal::setScenario(const char* scenario) {
    this->mScenario = scenario;
}

void Signal::setNumArgs(int32_t numArgs) {
    this->mNumArgs = numArgs;
}

void Signal::setList(std::vector<uint32_t>* listArgs) {
    this->mListArgs = listArgs;
}

Signal::~Signal() {}
