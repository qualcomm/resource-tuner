#ifndef SYSTUNE_RESOURCE_H
#define SYSTUNE_RESOURCE_H

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

    Resource() : mOpCode(0), mOpInfo(0), mOptionalInfo(0), mNumValues(0) {
        mConfigValue.singleValue = 0;
        mConfigValue.valueArray = nullptr;
    }
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

#endif
