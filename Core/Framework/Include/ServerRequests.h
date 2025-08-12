// Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
// SPDX-License-Identifier: BSD-3-Clause-Clear

#ifndef RESOURCE_TUNER_SERVER_REQUEST_H
#define RESOURCE_TUNER_SERVER_REQUEST_H

#include "Utils.h"
#include "RequestQueue.h"
#include "ThreadPool.h"
#include "CocoTable.h"
#include "RateLimiter.h"
#include "RequestManager.h"
#include "Request.h"
#include "ResourceTunerSettings.h"
#include "ResourceProcessor.h"
#include "ResourceRegistry.h"
#include "RequestManager.h"
#include "ServerInternal.h"
#include "Logger.h"
#include "TargetRegistry.h"

/**
 * @brief Main server thread for processing Resource Provisioning requests.
 *
 * @details Continuously monitors the request queue and dispatches requests
 * to appropriate internal handlers based on their type. The request queue goes idle if
 * there are no more requests. When a new request arrives, it wakes up the queue.
 */
void* TunerServerThread();

#endif
