// Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
// SPDX-License-Identifier: BSD-3-Clause-Clear

#include "SignalQueue.h"

std::shared_ptr<SignalQueue> SignalQueue::mSignalQueueInstance = nullptr;
std::mutex SignalQueue::instanceProtectionLock{};

SignalQueue::SignalQueue() {}

SignalQueue::~SignalQueue() {}
