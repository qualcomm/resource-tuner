// Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
// SPDX-License-Identifier: BSD-3-Clause-Clear

#ifndef MEMORY_POOL_H
#define MEMORY_POOL_H

#include <iostream>
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
     *
     * @param mBlockCount Number of blocks to be allocated.
     */
    int32_t makeAllocation(int32_t blockCount);

    /**
     * @brief Get an allocated block for the already allocated type T.
     * @details This routine should only be called after the makeAllocation call for a particular type
     *
     * @return void* Pointer to the allocated type.
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
    static std::shared_ptr<PoolWrapper> mPoolWrapperInstance;

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
     *
     * @param mBlockCount Number of blocks to be allocated.
     */
    template <typename T>
    int32_t makeAllocation(int32_t blockCount) {
        return makeAllocation(blockCount, sizeof(T), std::type_index(typeid(T)));
    }

    /**
     * @brief Get an allocated block for the already allocated type T.
     * @details This routine should only be called after the makeAllocation call for a particular type
     *
     * @return void* Pointer to the allocated type.
     */
    template <typename T>
    void* getBlock() {
        return getBlock(sizeof(T), std::type_index(typeid(T)));
    }

    /**
     * @brief Free an allocated block of the specified type T.
     * @param block Pointer to the block to be freed.
     */
    template <typename T>
    void freeBlock(void* block) {
        reinterpret_cast<T*>(block)->~T();
        freeBlock(std::type_index(typeid(T)), block);
    }

    static std::shared_ptr<PoolWrapper> getInstance() {
        if(mPoolWrapperInstance == nullptr) {
            mPoolWrapperInstance = std::shared_ptr<PoolWrapper>(new PoolWrapper());
        }
        return mPoolWrapperInstance;
    }
};

template <typename T>
inline void MakeAlloc(int32_t blockCount) {
    PoolWrapper::getInstance()->makeAllocation<T>(blockCount);
}

template <typename T>
inline void* GetBlock() {
    return PoolWrapper::getInstance()->getBlock<T>();
}

template <typename T>
inline void FreeBlock(void* block) {
    PoolWrapper::getInstance()->freeBlock<T>(block);
}

#endif
