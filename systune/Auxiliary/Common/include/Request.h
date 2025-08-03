// Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
// SPDX-License-Identifier: BSD-3-Clause-Clear

#ifndef REQUEST_H
#define REQUEST_H

#include <cstdint>
#include <vector>

#include "Timer.h"
#include "SafeOps.h"
#include "Utils.h"

/**
 * @class Resource
 * @brief Represents a sysfs node that needs to be written to by the client.
 * @details If the number of values to write is just one, a single value is stored
 *          otherwise a pointer to a vector of values is stored.
 * @struct
 */
class Resource {
private:
    uint32_t mOpCode;
    int32_t mOpInfo;
    int32_t mOptionalInfo;
    int32_t mNumValues;

public:
    union {
        int32_t singleValue;
        std::vector<int32_t>* valueArray;
    } mConfigValue;

    Resource() : mOpCode(0), mOpInfo(0), mOptionalInfo(0), mNumValues(0) {}
    ~Resource() {}

    int32_t getCoreValue();
    int32_t getClusterValue();
    int32_t getOperationalInfo();
    int32_t getOptionalInfo();
    uint32_t getOpCode();
    int32_t getValuesCount();

    void setCoreValue(int32_t core);
    void setClusterValue(int32_t cluster);
    void setResourceID(int16_t resID);
    void setResourceType(int8_t resType);
    void setOpCode(uint32_t opCode);
    void setOperationalInfo(int32_t opInfo);
    void setOptionalInfo(int32_t optionalInfo);
    void setNumValues(int32_t numValues);
    void setAsCustom();
};

class CocoNode {
public:
    Resource* mResource;
    CocoNode* next;
    CocoNode* prev;

    CocoNode() : mResource(nullptr), next(nullptr), prev(nullptr) {}
};

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
    Request() : mNumResources(0), mNumCocoNodes(0), mResources(nullptr),
        mCocoNodes(nullptr), mTimer(nullptr), mBackgroundProcessing(false) {}
    ~Request();

    int32_t getResourcesCount();
    int32_t getCocoNodesCount();
    std::vector<Resource*>* getResources();
    Resource* getResourceAt(int32_t index);
    std::vector<CocoNode*>* getCocoNodes();
    CocoNode* getCocoNodeAt(int32_t index);
    Timer* getTimer();

    void setNumResources(int32_t numResources);
    void setNumCocoNodes(int32_t numCocoNodes);
    void setTimer(Timer* timer);
    void unsetTimer();
    void setResources(std::vector<Resource*>* resources);
    void setCocoNodes(std::vector<CocoNode*>* cocoNodes);

    ErrCode serialize(char* buf);
    ErrCode deserialize(char* buf);

    void populateUntuneRequest(Request* request);
    void populateRetuneRequest(Request* request, int64_t duration);
    static void cleanUpRequest(Request* request);
};

#endif
