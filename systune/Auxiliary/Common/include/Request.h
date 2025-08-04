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
 * @brief Used to store information regarding Resources / Tunables which need to be
 *        Provisioned as part of the tuneResources API.
 */
class Resource {
private:
    /**
     * @brief A uniqued 32-bit (unsigned) identifier for the Resource.
     *        - The last 16 bits (17-32) are used to specify the ResId
     *        - The next 8 bits (9-16) are used to specify the ResType (type of the Resource)
     *        - In addition for Custom Resources, then the MSB must be set to 1 as well
     */
    uint32_t mOpCode;
    /**
     * @brief Holds Logical Core and Cluster Information:
     *        - The last 8 bits (25-32) hold the Logical Core Value
     *        - The next 8 bits (17-24) hold the Logical Cluster Value
     */
    int32_t mOpInfo;
    int32_t mOptionalInfo; //!< Field to hold optional information for Request Processing
    /**
     * @brief Number of values to be configured for the Resource,
     *        both single-valued and multi-valued Resources are supported.
     */
    int32_t mNumValues;

public:
    union {
        int32_t singleValue; //!< Use this field for single Valued Resources
        std::vector<int32_t>* valueArray; //!< Use this field for Multi Valued Resources
    } mConfigValue; //!< The value to be Configured for this Resource Node.

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
    Timer* mTimer; //!< Timer associated with the request.

public:
    Request() : mNumResources(0), mNumCocoNodes(0), mResources(nullptr),
                mCocoNodes(nullptr), mTimer(nullptr) {}
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
