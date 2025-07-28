// #include <gtest/gtest.h>
// #include "MemoryPool.h"

// // Test structure, used for allocation testing
// struct TestBuffer {
//     int32_t testId;
//     double score;
//     int8_t isDuplicate;
// };

// TEST(MemoryPoolAllocationTests, TestMemoryPoolBasicAllocation1) {
//     MakeAlloc<TestBuffer>(2);

//     void* block = GetBlock<TestBuffer>();
//     ASSERT_NE(block, nullptr);

//     FreeBlock<TestBuffer>(static_cast<void*>(block));
// }

// TEST(MemoryPoolAllocationTests, TestMemoryPoolBasicAllocation2) {
//     MakeAlloc<char[250]>(2);

//     void* firstBlock = GetBlock<char[250]>();
//     ASSERT_NE(firstBlock, nullptr);

//     void* secondBlock = GetBlock<char[250]>();
//     ASSERT_NE(secondBlock, nullptr);
// }

// struct ListNode {
//     int32_t val;
//     ListNode* next;
// };

// TEST(MemoryPoolAllocationTests, TestMemoryPoolBasicAllocation3) {
//     MakeAlloc<ListNode>(10);
//     ListNode* head = nullptr;
//     ListNode* cur = nullptr;

//     for(int32_t i = 0; i < 10; i++) {
//         ListNode* node = (ListNode*)GetBlock<ListNode>();
//         ASSERT_NE(node, nullptr);

//         node->val = i + 1;
//         node->next = nullptr;

//         if(head == nullptr) {
//             head = node;
//             cur = node;
//         } else {
//             cur->next = node;
//             cur = cur->next;
//         }
//     }

//     cur = head;
//     int32_t counter = 1;
//     while(cur != nullptr) {
//         ASSERT_EQ(cur->val, counter);
//         ListNode* next = cur->next;
//         FreeBlock<ListNode>(static_cast<void*>(cur));

//         cur = next;
//         counter++;
//     }
// }

// struct Request {
//     int32_t requestID;
//     int64_t requestTimestamp;
// };

// TEST(MemoryPoolAllocationTests, TestMemoryPoolBasicAllocation4) {
//     MakeAlloc<std::vector<Request*>>(1);
//     MakeAlloc<Request>(20);

//     std::vector<Request*>* requests =
//             (std::vector<Request*>*) GetBlock<std::vector<Request*>>();

//     ASSERT_NE(requests, nullptr);

//     // Add some elements to the vector
//     for(int32_t i = 0; i < 15; i++) {
//         Request* request = (Request*) GetBlock<Request>();
//         ASSERT_NE(request, nullptr);

//         request->requestID = i + 1;
//         request->requestTimestamp = 100 * (i + 3);
//         requests->push_back(request);
//     }

//     for(int32_t i = 0; i < requests->size(); i++) {
//         ASSERT_EQ((*requests)[i]->requestID, i + 1);
//         ASSERT_EQ((*requests)[i]->requestTimestamp, 100 * (i + 3));

//         FreeBlock<Request>(static_cast<void*>((*requests)[i]));
//     }

//     FreeBlock<std::vector<Request*>>(static_cast<void*>(requests));
// }

// class DataHub {
// private:
//     int32_t mFolderCount;
//     int32_t mUserCount;
//     std::string mOrgName;
// public:
//     DataHub(int32_t mFolderCount, int32_t mUserCount, std::string mOrgName) {
//         this->mFolderCount = mFolderCount;
//         this->mUserCount = mUserCount;
//         this->mOrgName = mOrgName;
//     }
// };

// TEST(MemoryPoolAllocationTests, TestMemoryPoolBasicAllocation6) {
//     MakeAlloc<DataHub>(1);

//     // Create an object of DataHub with the parametrized constructor
//     DataHub* dataHubObj = new(GetBlock<DataHub>()) DataHub(30, 17, "XYZ-co");

//     ASSERT_NE(dataHubObj, nullptr);
// }

// TEST(MemoryPoolAllocationTests, TestMemoryPoolBasicAllocation7) {

//     int8_t allocationFailed = false;
//     void* block = nullptr;

//     try {
//         block = GetBlock<char[250]>();
//     } catch(std::bad_alloc& e) {
//         allocationFailed = true;
//     }

//     ASSERT_EQ(block, nullptr);
//     ASSERT_EQ(allocationFailed, true);
// }

// TEST(MemoryPoolFreeTests, TestMemoryPoolFreeingMemory1) {
//     MakeAlloc<char[125]>(2);

//     void* firstBlock = GetBlock<char[125]>();
//     ASSERT_NE(firstBlock, nullptr);

//     void* secondBlock = GetBlock<char[125]>();
//     ASSERT_NE(secondBlock, nullptr);

//     // Free one of the blocks
//     FreeBlock<char[125]>(static_cast<void*>(firstBlock));

//     // The call for another allocation should not return null, since currently
//     // only one block is allocated.
//     void* thirdBlock = GetBlock<char[125]>();
//     ASSERT_NE(thirdBlock, nullptr);
// }

// TEST(MemoryPoolFreeTests, TestMemoryPoolFreeingMemory2) {
//     MakeAlloc<char[200]>(5);

//     std::vector<void*> allocatedBlocks;

//     for(int32_t i = 0; i < 5; i++) {
//         allocatedBlocks.push_back(GetBlock<char[200]>());
//         ASSERT_NE(allocatedBlocks.back(), nullptr);
//     }

//     for(int32_t i = 0; i < 5; i++) {
//         void* block = nullptr;
//         int8_t allocationFailed = false;

//         try {
//             block = GetBlock<char[200]>();
//         } catch(std::bad_alloc& e) {
//             allocationFailed = true;
//         }

//         ASSERT_EQ(block, nullptr);
//         ASSERT_EQ(allocationFailed, true);
//     }

//     // Free all the allocated blocks
//     for(int32_t i = 0; i < 5; i++) {
//         FreeBlock<char[200]>(static_cast<void*>(allocatedBlocks[i]));
//     }

//     for(int32_t i = 0; i < 5; i++) {
//         allocatedBlocks[i] = GetBlock<char[200]>();
//         ASSERT_NE(allocatedBlocks[i], nullptr);
//     }

//     // Free all the allocated blocks
//     for(int32_t i = 0; i < 5; i++) {
//         FreeBlock<char[200]>(static_cast<void*>(allocatedBlocks[i]));
//     }
// }

// class CustomDataType {
// private:
//     int8_t* mDestructorCalled;

// public:
//     CustomDataType(int8_t* mDestructorCalled) {
//         this->mDestructorCalled = mDestructorCalled;
//     }

//     ~CustomDataType() {
//         *this->mDestructorCalled = true;
//     }
// };

// TEST(MemoryPoolFreeTests, TestMemoryPoolFreeingMemory3) {
//     MakeAlloc<CustomDataType>(1);

//     int8_t* destructorCalled = (int8_t*) malloc(sizeof(int8_t));
//     *destructorCalled = false;

//     CustomDataType* customDTObject =
//         new(GetBlock<CustomDataType>()) CustomDataType(destructorCalled);

//     FreeBlock<CustomDataType>(static_cast<void*>(customDTObject));
//     ASSERT_EQ(*destructorCalled, false);
// }
