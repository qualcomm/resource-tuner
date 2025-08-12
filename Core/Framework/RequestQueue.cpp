// Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
// SPDX-License-Identifier: BSD-3-Clause-Clear

#include "RequestQueue.h"

std::shared_ptr<RequestQueue> RequestQueue::mRequestQueueInstance = nullptr;
std::mutex RequestQueue::instanceProtectionLock{};

RequestQueue::RequestQueue() {}

RequestQueue::~RequestQueue() {}
