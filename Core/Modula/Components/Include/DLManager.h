// Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
// SPDX-License-Identifier: BSD-3-Clause-Clear

#ifndef DL_MANAGER_H
#define DL_MANAGER_H

#include <cstdint>
#include "ErrCodes.h"

struct DLMover {
    struct DLMover* next;
    struct DLMover* prev;
};

typedef struct DLMover DLMover;

typedef struct _core_iterable {
    void* mData;
    // static, At most 8 linkages or movers are allowed as of now.
    // Avoid heap allocations for this node.
    struct {
        struct _core_iterable* next;
        struct _core_iterable* prev;
    } mLinkages[8];

    _core_iterable() : mData(nullptr) {}
} CoreIterable;

typedef int8_t (*DLPolicy)(CoreIterable* newNode, CoreIterable* targetNode);

typedef struct _policyDir {
    DLPolicy mAscPolicy;
    DLPolicy mDescPolicy;
    DLPolicy mReplacePolicy;

    _policyDir() : mAscPolicy(nullptr), mDescPolicy(nullptr) {};
} PolicyRepo;

enum DLOptions {
    INSERT_START,
    INSERT_END,
    INSERT_N_NODE_START,
};

class DLManager {
public:
    CoreIterable* mHead;
    CoreIterable* mTail;

    int32_t mTotalLinkers;
    int32_t mLinkerInUse; // must be a value b/w 0 to mTotalLinkers - 1
    int32_t mSize;

    // Declared as public so that the cb's can be selectively set at DLManager creation time.
    PolicyRepo mSavedPolicies;

    DLManager(int32_t linkerInUse = 0);

    ErrCode insert(CoreIterable* node);
    ErrCode insert(CoreIterable* node, DLOptions option, int32_t n = 0);
    ErrCode insertWithPolicy(CoreIterable* node, DLPolicy policy);

    // Specialized functions which require certain fields in the PolicyRepo to be non-nul
    ErrCode insertAsc(CoreIterable* node); // .mAscPolicy must be set
    ErrCode insertDesc(CoreIterable* node); // .mDescPolicy must be set

    ErrCode deleteNode(CoreIterable* node);
    ErrCode destroy();

    int8_t isNodeNth(int32_t n, CoreIterable* node); // 0 based indexing
    int32_t getLen();
};

#define DL_ITERATE(dlm)                                                                     \
    CoreIterable* iter;                                                                     \
    for(iter = dlm->mHead; iter != nullptr; iter = iter->mLinkages[dlm->mLinkerInUse].next)

#endif
