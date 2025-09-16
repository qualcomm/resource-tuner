// Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
// SPDX-License-Identifier: BSD-3-Clause-Clear

#ifndef MEMORY_POOL_H
#define MEMORY_POOL_H

/*!
 * \file  MemoryPool.h
 */

/*!
 * \ingroup MEMORY_POOL
 * \defgroup MEMORY_POOL Memory Pool
 * \details Used to Pre-Allocate Memory for commonly used types Capacity.
 *          - To pre-allocate memory make use of the MakeAlloc API. This API takes
 *            the type for which reservation needs to be made and the number of blocks of
 *            memory which need to be reserved for this type. For example: MakeAlloc<X>(y), refers
 *            to a request to reserve "y" number of blocks of type "X".\n\n
 *          - To get a memory block make use of the GetBlock API. This API returns a pointer to a
 *            block of memory, if blocks are available, else it throws a std::bad_alloc exception.
 *            This API is intended to be used in conjunction with the placement new operator. For example:
 *            To get a memory block of type X, use the API as follows:\n
 *            => X* xPtr = new (GetBlock<X>()) X(...);\n\n
 *          - To free a memory block make use of the FreeBlock API, as follows:\n
 *            => FreeBlock<X>(xPtr);
 *
 * @{
 */

#include <vector>
#include <exception>
#include <mutex>
#include <memory>
#include <unordered_map>
#include <typeindex>
#include <typeinfo>

#include "Utils.h"
#include "Logger.h"

typedef struct _memoryNode {
    _memoryNode* next;
    void* block;
} MemoryNode;

/**
 * @brief MemoryPool
 * @details Preallocate Memory for Commonly Used types, to decrease the
 *          Runtime Overhead of Memory Allocation and Deallocation System Calls
 *          while Processing Requests.
 */
class MemoryPool {
private:
    static std::shared_ptr<MemoryPool> mMemoryPoolInstance;
    std::mutex mMemoryPoolMutex;

    MemoryNode* mFreeListHead;
    MemoryNode* mFreeListTail;
    MemoryNode* mAllocatedListHead;

    int32_t mBlockSize;
    int32_t mfreeBlocks;

    int32_t addNodesToFreeList(int32_t blockCount);

public:
    MemoryPool(int32_t blockSize);
    ~MemoryPool();

    /**
     * @brief Allocate memory for the specified type T.
     * @details This routine will allocate the number of memory blocks for the type specified by the client.
     * @param blockCount Number of blocks to be allocated.
     * @return int32_t:\n
     *            - Number of blocks which were actually allocated (might be smaller than blockCount)
     */
    int32_t makeAllocation(int32_t blockCount);

    /**
     * @brief Get an allocated block for the already allocated type T.
     * @details This routine should only be called after the makeAllocation call for a particular type
     *          Note: If a block is not available then the Routine throws a std::bad_alloc exception.
     * @return void*:\n
     *           - Pointer to the allocated type.
     */
    void* getBlock();

    /**
     * @brief Free an allocated block of the specified type T.
     * @param block Pointer to the block to be freed.
     */
    void freeBlock(void* block);
};

class PoolWrapper {
private:
    std::unordered_map<std::type_index, MemoryPool*> mMemoryPoolRefs;
    std::mutex mPoolWrapperMutex;

    MemoryPool* getMemoryPool(std::type_index typeIndex);

    int32_t makeAllocation(int32_t blockCount, int32_t blockSize, std::type_index typeIndex);
    void* getBlock(int32_t blockSize, std::type_index typeIndex);
    void freeBlock(std::type_index typeIndex, void* block);

public:
    PoolWrapper() {}
    ~PoolWrapper() {}

    /**
     * @brief Allocate memory for the specified type T.
     * @details This routine will allocate the number of memory blocks for the type specified by the client.
     * @param int32_t Number of blocks to be allocated.
     */
    template <typename T>
    int32_t makeAllocation(int32_t blockCount) {
        return makeAllocation(blockCount, sizeof(T), std::type_index(typeid(T)));
    }

    /**
     * @brief Get an allocated block for the already allocated type T.
     * @details This routine should only be called after the makeAllocation call for a particular type
     * @return void*:\n
     *           - Pointer to the allocated type.
     */
    template <typename T>
    void* getBlock() {
        return getBlock(sizeof(T), std::type_index(typeid(T)));
    }

    /**
     * @brief Free an allocated block of the specified type T.
     * @details As part of this routine, the destructor of the type will be invoked.
     * @param block Pointer to the block to be freed.
     */
    template<typename T>
    typename std::enable_if<std::is_class<T>::value, void>::type
    freeBlock(void* block) {
        reinterpret_cast<T*>(block)->~T();
        freeBlock(std::type_index(typeid(T)), block);
    }

    template<typename T>
    typename std::enable_if<!std::is_class<T>::value, void>::type
    freeBlock(void* block) {
        freeBlock(std::type_index(typeid(T)), block);
    }
};

std::shared_ptr<PoolWrapper> getPoolWrapper();

template <typename T>
inline void MakeAlloc(int32_t blockCount) {
    getPoolWrapper()->makeAllocation<T>(blockCount);
}

template <typename T>
inline void* GetBlock() {
    return getPoolWrapper()->getBlock<T>();
}

template <typename T>
inline void FreeBlock(void* block) {
    getPoolWrapper()->freeBlock<T>(block);
}

#endif

/*! @} */
