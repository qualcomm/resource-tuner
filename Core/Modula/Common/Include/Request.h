// Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
// SPDX-License-Identifier: BSD-3-Clause-Clear

#ifndef REQUEST_H
#define REQUEST_H

#include <vector>

#include "ErrCodes.h"
#include "Timer.h"
#include "SafeOps.h"
#include "Utils.h"
#include "Message.h"
#include "Resource.h"
#include "DLManager.h"

#define REQUEST_DL_NR 1
#define COCO_TABLE_DL_NR 2

/**
 * @brief Encapsulation type for a Resource Provisioning Request.
 */
class Request : public Message {
private:
    Timer* mTimer; //!< Timer associated with the request.
    DLManager* mResourceList;

public:
    Request();
    ~Request();

    int32_t getResourcesCount();
    Timer* getTimer();
    DLManager* getResDlMgr();

    void addResource(ResIterable* resIterable);
    void setTimer(Timer* timer);
    void unsetTimer();
    void clearResources();

    ErrCode serialize(char* buf);
    ErrCode deserialize(char* buf);

    void populateUntuneRequest(Request* request);
    void populateRetuneRequest(Request* request, int64_t duration);
    static void cleanUpRequest(Request* request);
};

#endif
