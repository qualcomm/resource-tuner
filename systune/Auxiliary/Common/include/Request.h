// Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
// SPDX-License-Identifier: BSD-3-Clause-Clear

#ifndef REQUEST_H
#define REQUEST_H

#include <cstdint>
#include <vector>

#include "Timer.h"
#include "SafeOps.h"
#include "Utils.h"
#include "Types.h"

/**
* @brief Encapsulation type for a Resource Provisioning Request.
*/
class Request : public Message {
private:
    int32_t mNumResources; //!< Number of resources to be tuned as Part of the Request.
    int32_t mNumCocoNodes; //!< Number of coco nodes Allocated for the Request.
    std::vector<Resource*>* mResources; //!< Pointer to a vector, storing all the Resources to be tuned.
    std::vector<CocoNode*>* mCocoNodes; //!< Pointer to a vector, storing all the CocoNodes for the Request.
    Timer* mTimer; //<! Timer associated with the request.
    int8_t mBackgroundProcessing; //<! Flag indicating if Background Processing is Enabled for the Request.

public:
    Request();
    ~Request();

    int32_t getResourcesCount();
    int32_t getCocoNodesCount();
    std::vector<Resource*>* getResources();
    Resource* getResourceAt(int32_t index);
    std::vector<CocoNode*>* getCocoNodes();
    CocoNode* getCocoNodeAt(int32_t index);

    void setNumResources(int32_t numResources);
    void setNumCocoNodes(int32_t numCocoNodes);
    void setTimer(Timer* timer);
    void updateTimer(int64_t newDuration);
    void setResources(std::vector<Resource*>* resources);
    void setCocoNodes(std::vector<CocoNode*>* cocoNodes);

    ErrCode serialize(char* buf);
    ErrCode deserialize(char* buf);

    void populateUntuneRequest(Request* request);
    void populateRetuneRequest(Request* request, int64_t duration);
    static void cleanUpRequest(Request* request);
};

#endif
