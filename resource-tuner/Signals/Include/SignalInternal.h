// Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
// SPDX-License-Identifier: BSD-3-Clause-Clear

/*!
 * \file SignalInternal.h
 */

#ifndef SIGNAL_INTERNAL_H
#define SIGNAL_INTERNAL_H

#include "ErrCodes.h"
#include "Signal.h"
#include "ExtFeaturesRegistry.h"
#include "ClientDataManager.h"
#include "RateLimiter.h"
#include "ResourceRegistry.h"
#include "SignalQueue.h"
#include "ServerInternal.h"

/**
 * @brief Internal API for submitting Signal Requests for Processing if the Signals
 *        module is enabled.
 * @details Resource Tuner Modules can directly use this API to submit Requests rather than
 *          using the Client Interface.
 */
ErrCode submitSignalRequest(void* clientReq);

#endif
