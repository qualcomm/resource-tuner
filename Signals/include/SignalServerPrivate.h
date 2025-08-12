// Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
// SPDX-License-Identifier: BSD-3-Clause-Clear

#ifndef SIGNAL_SERVER_PRIVATE_H
#define SIGNAL_SERVER_PRIVATE_H

#include <iostream>
#include <thread>

#include "SignalConfigProcessor.h"
#include "ExtFeaturesConfigProcessor.h"
#include "ExtFeaturesRegistry.h"
#include "SignalExtFeatureMapper.h"
#include "SignalQueue.h"
#include "RequestQueue.h"
#include "SignalRegistry.h"
#include "ServerInternal.h"
#include "ResourceTunerSettings.h"
#include "ClientDataManager.h"
#include "RateLimiter.h"
#include "MemoryPool.h"
#include "Extensions.h"
#include "Utils.h"
#include "Logger.h"

/**
 * @brief Main server thread for processing signal requests.
 *
 * @details Continuously monitors the request queue and dispatches requests
 * to appropriate internal handlers based on their type. The request queue goes idle if
 * there are no more requests. When a new Signal request arrives, it wakes up the queue.
 */
void* SignalsdServerThread();

#endif
