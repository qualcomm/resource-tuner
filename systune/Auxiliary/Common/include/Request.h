// Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
// SPDX-License-Identifier: BSD-3-Clause-Clear

#ifndef REQUEST_H
#define REQUEST_H

#include <cstdint>
#include <vector>

#include "Timer.h"
#include "Utils.h"
#include "Types.h"

class Request : public Message {
private:
    int32_t mNumResources; //!< Number of resources requested.
    int32_t mNumCocoNodes; //!< Number of coco nodes Allocated.
    std::vector<Resource*>* mResources; //!< List of pointers to the requested Resource objects.
    std::vector<CocoNode*>* mCocoNodes; //!< List of pointers to CocoNode objects associated with this request.
    Timer* mTimer; //<! Timer associated with the request.
    int8_t mBackgroundProcessing;

public:
    Request();
    ~Request();

    int32_t getResourcesCount();
    int32_t getCocoNodesCount();
    int8_t isBackgroundProcessingEnabled();
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
    void setBackgroundProcessing(int8_t isBackgroundProcessingEnabled);

    void populateUntuneRequest(Request* request);
    void populateRetuneRequest(Request* request, int64_t duration);
    static void cleanUpRequest(Request* request);
};

#endif
