// Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
// SPDX-License-Identifier: BSD-3-Clause-Clear

#include "MemoryPool.h"

std::shared_ptr<PoolWrapper> PoolWrapper::mPoolWrapperInstance = nullptr;

MemoryPool::MemoryPool(int32_t blockSize) {
    this->mFreeListHead = this->mFreeListTail = nullptr;
    this->mAllocatedListHead = nullptr;
    this->mfreeBlocks = 0;
    this->mBlockSize = blockSize;
}

int32_t MemoryPool::addNodesToFreeList(int32_t blockCount) {
    int32_t allocatedCount = 0;

    for(int32_t i = 0; i < blockCount; i++) {
        MemoryNode* allocationBlock = nullptr;

        try {
            allocationBlock = new MemoryNode;
            allocationBlock->block = new char[this->mBlockSize];
            allocationBlock->next = nullptr;
            allocatedCount++;

        } catch(const std::bad_alloc& e) {
            TYPELOGV(MEMORY_POOL_ALLOCATION_FAILURE, this->mBlockSize,
                     blockCount, allocatedCount);
            return allocatedCount;
        }

        if(this->mFreeListHead == nullptr) {
            this->mFreeListHead = allocationBlock;
            this->mFreeListTail = allocationBlock;
        } else {
            this->mFreeListTail->next = allocationBlock;
            this->mFreeListTail = this->mFreeListTail->next;
        }
    }
    return allocatedCount;
}

int32_t MemoryPool::makeAllocation(int32_t blockCount) {
    int32_t blocksAllocated = 0;
    try {
        const std::lock_guard<std::mutex> lock(this->mMemoryPoolMutex);

        int32_t blocksAllocated = addNodesToFreeList(blockCount);
        this->mfreeBlocks += blockCount;
        return blocksAllocated;

    } catch(const std::bad_alloc& e) {
        TYPELOGV(MEMORY_POOL_ALLOCATION_FAILURE, this->mBlockSize, blockCount, blocksAllocated);

        return 0;
    }
    catch (const std::exception& e) {
        TYPELOGV(MEMORY_POOL_ALLOCATION_FAILURE, this->mBlockSize,
                 blockCount, blocksAllocated);

        return 0;
    }

    return blocksAllocated;
}

void* MemoryPool::getBlock() {
    try {
        const std::lock_guard<std::mutex> lock(this->mMemoryPoolMutex);

        if(this->mfreeBlocks == 0) {
            TYPELOGV(MEMORY_POOL_BLOCK_RETRIEVAL_FAILURE, this->mBlockSize);
            throw std::bad_alloc();
        }

        // Get a free block from the free list
        MemoryNode* memNode = this->mFreeListHead;
        this->mFreeListHead = this->mFreeListHead->next;
        memNode->next = nullptr;

        if(this->mFreeListHead == nullptr) {
            this->mFreeListTail = nullptr;
        }

        void* freeBlock = memNode->block;
        memNode->block = nullptr;

        if(this->mAllocatedListHead == nullptr) {
            this->mAllocatedListHead = memNode;
            this->mAllocatedListHead->next = nullptr;
        } else {
            memNode->next = this->mAllocatedListHead;
            this->mAllocatedListHead = memNode;
        }

        this->mfreeBlocks--;
        return freeBlock;

    } catch(const std::system_error& e){
        TYPELOGV(MEMORY_POOL_BLOCK_RETRIEVAL_FAILURE, this->mBlockSize);

    } catch(const std::bad_alloc& e) {
        TYPELOGV(MEMORY_POOL_BLOCK_RETRIEVAL_FAILURE, this->mBlockSize);

        throw;
    }

    throw std::bad_alloc();
}

void MemoryPool::freeBlock(void* block) {
    try {
        const std::lock_guard<std::mutex> lock(this->mMemoryPoolMutex);

        if(block == nullptr ||
           this->mAllocatedListHead == nullptr) {
            // Edge Cases, which will be hit if
            // 1. Client tries to free some block of memory which was not Allocated by the Memory Pool
            // 2. Client acquires "n" block of memory of certain size, and tries to free m (> n) blocks of that size

            // Under normal operations, these conditions should not be hit
            // As a General Rule, Perform as many Allocations as possible from the Memory Pool and not
            // via the Memory Allocation APIs
            return;
        }

        MemoryNode* memNode = this->mAllocatedListHead;
        this->mAllocatedListHead = this->mAllocatedListHead->next;
        memNode->block = block;
        memNode->next = nullptr;

        if(this->mFreeListHead == nullptr) {
            this->mFreeListHead = memNode;
            this->mFreeListTail = memNode;
        } else {
            this->mFreeListTail->next = memNode;
            this->mFreeListTail = this->mFreeListTail->next;
        }

        this->mfreeBlocks++;

    } catch(const std::system_error& e){
        TYPELOGV(MEMORY_POOL_INVALID_BLOCK_SIZE, this->mBlockSize);

    } catch(const std::bad_alloc& e) {
        TYPELOGV(MEMORY_POOL_INVALID_BLOCK_SIZE, this->mBlockSize);
    }
}

MemoryPool::~MemoryPool() {
    try {
        MemoryNode* curNode = this->mFreeListHead;
        while(curNode != nullptr) {
            MemoryNode* nextNode = curNode->next;
            if(curNode->block != nullptr) {
                delete[] static_cast<char*> (curNode->block);
                curNode->block = nullptr;
            }
            delete curNode;
            curNode = nextNode;
        }

        curNode = this->mAllocatedListHead;
        while(curNode != nullptr) {
            MemoryNode* nextNode = curNode->next;

            delete curNode;
            curNode = nextNode;
        }

    } catch(const std::bad_alloc& e) {}
}

MemoryPool* PoolWrapper::getMemoryPool(std::type_index typeIndex) {
    if(this->mMemoryPoolRefs.find(typeIndex) == this->mMemoryPoolRefs.end()) {
        return nullptr;
    }
    return this->mMemoryPoolRefs[typeIndex];
}

int32_t PoolWrapper::makeAllocation(int32_t blockCount, int32_t blockSize, std::type_index typeIndex) {
    // Sanity Checks
    if(blockCount <= 0) return 0;

    try {
        const std::lock_guard<std::mutex> lock(this->mPoolWrapperMutex);
        MemoryPool* memoryPool = getMemoryPool(typeIndex);

        if(memoryPool == nullptr) {
            memoryPool = new MemoryPool(blockSize);
            this->mMemoryPoolRefs[typeIndex] = memoryPool;
        }

    } catch(std::bad_alloc& e) {
        TYPELOGV(MEMORY_POOL_ALLOCATION_FAILURE, blockSize, blockCount, 0);
        return 0;

    } catch(const std::system_error& e) {
        TYPELOGV(MEMORY_POOL_ALLOCATION_FAILURE, blockSize, blockCount, 0);
        return 0;
    }

    // Now make the Actual Allocation
    return this->mMemoryPoolRefs[typeIndex]->makeAllocation(blockCount);
}

void* PoolWrapper::getBlock(int32_t blockSize, std::type_index typeIndex) {
     MemoryPool* memoryPool = nullptr;

    try {
        const std::lock_guard<std::mutex> lock(this->mPoolWrapperMutex);
        memoryPool = getMemoryPool(typeIndex);

        // Propagate the Exception to the Client, indicating Memory Block
        // Could not be retrieved.
        // Since the block of Memory returned by the pool will be directly
        // Used in combination with the Placement-New Operator, Hence simply returning
        // A Null Pointer will not work here.
        if(memoryPool == nullptr) {
            TYPELOGV(MEMORY_POOL_BLOCK_RETRIEVAL_FAILURE, blockSize);
            throw std::bad_alloc();
        }

    } catch(const std::system_error& e) {
        TYPELOGV(MEMORY_POOL_BLOCK_RETRIEVAL_FAILURE, blockSize);
        throw std::bad_alloc();
    }

    return memoryPool->getBlock();
}

void PoolWrapper::freeBlock(std::type_index typeIndex, void* block) {
    MemoryPool* memoryPool = nullptr;

    try {
        const std::lock_guard<std::mutex> lock(this->mPoolWrapperMutex);
        memoryPool = getMemoryPool(typeIndex);

        // Edge Case
        // This will be hit if the Client tries to free some block of Memory
        // which was never allocated through the MemoryManager
        // In such cases, simply ignore the freeBlock call.
        if(memoryPool == nullptr) {
            return;
        }

    } catch(const std::system_error& e) {
        return;
    }

    if(memoryPool != nullptr) {
        memoryPool->freeBlock(block);
    }
}
