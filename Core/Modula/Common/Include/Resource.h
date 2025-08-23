// Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
// SPDX-License-Identifier: BSD-3-Clause-Clear

#ifndef RESOURCE_TUNER_RESOURCE_H
#define RESOURCE_TUNER_RESOURCE_H

#include <cstdint>
#include <vector>

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
    uint32_t mResCode;
    /**
     * @brief Holds Logical Core and Cluster Information:
     *        - The last 8 bits (25-32) hold the Logical Core Value
     *        - The next 8 bits (17-24) hold the Logical Cluster Value
     */
    int32_t mResInfo;
    int32_t mOptionalInfo; //!< Field to hold optional information for Request Processing
    /**
     * @brief Number of values to be configured for the Resource,
     *        both single-valued and multi-valued Resources are supported.
     */
    int32_t mNumValues;

public:
    union {
        int32_t value; //!< Use this field for single Valued Resources
        std::vector<int32_t>* values; //!< Use this field for Multi Valued Resources
    } mResValue; //!< The value to be Configured for this Resource Node.

    Resource() : mResCode(0), mResInfo(0), mOptionalInfo(0), mNumValues(0) {
        mResValue.value = 0;
        mResValue.values = nullptr;
    }
    ~Resource() {}

    int32_t getCoreValue();
    int32_t getClusterValue();
    int32_t getResInfo();
    int32_t getOptionalInfo();
    uint32_t getResCode();
    int32_t getValuesCount();

    void setCoreValue(int32_t core);
    void setClusterValue(int32_t cluster);
    void setResourceID(int16_t resID);
    void setResourceType(int8_t resType);
    void setResCode(uint32_t resCode);
    void setResInfo(int32_t resInfo);
    void setOptionalInfo(int32_t optionalInfo);
    void setNumValues(int32_t numValues);
    void setAsCustom();
};

#endif
