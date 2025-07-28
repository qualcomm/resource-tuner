#include <thread>
#include <cstdint>
#include <gtest/gtest.h>

#include "Types.h"

// Request Cleanup Tests

TEST(MiscTests, TestResourceStructCoreClusterSettingAndExtraction) {
    Resource* resource = (Resource*) malloc(sizeof(Resorce));
    ASSERT_NE(resource, nullptr);

    resource->mOpInfo = 0;
    resource->mOpInfo = SET_RESOURCE_CORE_VALUE(2);
    resource->mOpInfo = SET_RESOURCE_CLUSTER_VALUE(1);

    ASSERT_EQ(EXTRACT_RESOURCE_CORE_VALUE(resource->mOpInfo), 2);
    ASSERT_EQ(EXTRACT_RESOURCE_CLUSTER_VALUE(resource->mOpInfo), 1);

    free(resource);
}
