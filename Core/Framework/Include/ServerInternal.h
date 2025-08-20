// Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
// SPDX-License-Identifier: BSD-3-Clause-Clear

#ifndef FRAMEWORK_INTERNAL_H
#define FRAMEWORK_INTERNAL_H

#include <cstdint>

#include "ErrCodes.h"
#include "Request.h"

/**
* @brief Initializes the SysConfig module.
* @details This function will read and Parse the Property Config YAML file.
* @return int8_t:
*              RC_SUCCESS: If the module was correct initialized.
*              Non-Zero Status Code indicating an Error: If initialization failed
*/
ErrCode fetchProperties();

void toggleDisplayModes();

/**
* @brief Submit a Resource Provisioning Request from a Client for processing.
* @details Note: This API acts an interface for other Resource Tuner components like Signals
*          to submit a Resource Provisioning Request to the Resource Tuner Server, and
*          subsequently provision the desired Resources.
* @param request A buffer holding the Request.
*/
void submitResourceProvisioningRequest(void* request);

void submitResourceProvisioningRequest(Request* request, int8_t isVerified);

#endif
