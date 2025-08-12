// Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
// SPDX-License-Identifier: BSD-3-Clause-Clear

#ifndef SYS_SIGNAL_INTERNAL_H
#define SYS_SIGNAL_INTERNAL_H

#include <cstdint>

/**
* @brief Internal API for submitting Signal Requests for Processing if the SysSignals
*        module is enabled.
* @details Resource Tuner Modules can directly use this API to submit Requests rather than
*          using the Client Interface.
*/
void submitSignalRequest(void* clientReq);

#endif
