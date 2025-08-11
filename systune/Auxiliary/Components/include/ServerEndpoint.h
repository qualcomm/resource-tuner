// Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
// SPDX-License-Identifier: BSD-3-Clause-Clear

#ifndef SERVER_ENDPOINT_H
#define SERVER_ENDPOINT_H

#include "Utils.h"

/**
* @brief ServerEndpoint
* @details Defines the Server Side Interface, which any Communication Medium
*          (for example Socket) must Implement.
*/
class ServerEndpoint {
public:
    virtual int32_t ListenForClientRequests() = 0;
    virtual int32_t closeConnection() = 0;
};

#endif
