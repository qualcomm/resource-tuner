// Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
// SPDX-License-Identifier: BSD-3-Clause-Clear

#include "DLManager.h"

DLManager::DLManager(int32_t linkerInUse) {
    this->mLinkerInUse = linkerInUse;
    this->mHead = this->mTail = nullptr;
    this->mSize = 0;
}

// Insert at end
ErrCode DLManager::insert(CoreIterable* node) {
    if(node == nullptr) {
        return RC_INVALID_VALUE;
    }

    CoreIterable* head = this->mHead;
    CoreIterable* tail = this->mTail;

    if(tail != nullptr) {
        this->mTail = node;
        node->mLinkages[this->mLinkerInUse].next = nullptr;
        node->mLinkages[this->mLinkerInUse].prev = tail;
        tail->mLinkages[this->mLinkerInUse].next = node;
    } else {
        // Means set head and tail to newNode
        this->mHead = node;
        this->mTail = node;
        node->mLinkages[this->mLinkerInUse].next = nullptr;
        node->mLinkages[this->mLinkerInUse].prev = nullptr;
    }

    this->mSize++;
    return RC_SUCCESS;
}

// Insert with options, covers:
// - insertion at head and insertion at n nodes from the head.
// - insertion at tail and insertion at n nodes from the tail
ErrCode DLManager::insert(CoreIterable* node, DLOptions option, int32_t n) {
    if(node == nullptr) {
        return RC_INVALID_VALUE;
    }

    switch(option) {
        case DLOptions::INSERT_START: {
            CoreIterable* head = this->mHead;

            if(head != nullptr) {
                this->mHead = node;
                node->mLinkages[this->mLinkerInUse].next = head;
                node->mLinkages[this->mLinkerInUse].prev = nullptr;
                head->mLinkages[this->mLinkerInUse].prev = node;
            } else {
                node->mLinkages[this->mLinkerInUse].prev = nullptr;
                node->mLinkages[this->mLinkerInUse].next = nullptr;
                this->mHead = node;
                this->mTail = node;
            }

            this->mSize++;
            return RC_SUCCESS;
        }

        case DLOptions::INSERT_END: {
            return this->insert(node);
        }

        case DLOptions::INSERT_N_NODE_START: {
            if(n < 0) return RC_BAD_ARG;
            if(n == 0) return this->insert(node, INSERT_START);
            if(n == this->getLen() + 1) return this->insert(node);
            if(n > this->getLen()) return RC_BAD_ARG;

            int32_t curPos = 1;
            CoreIterable* curNode = this->mHead;
            while(curNode != nullptr) {
                if(curPos == n) break;
                curNode = curNode->mLinkages[this->mLinkerInUse].next;
                curPos++;
            }

            CoreIterable* prevNode = curNode->mLinkages[this->mLinkerInUse].prev;
            // Insert b/w prevNode and curNode
            if(prevNode == nullptr) {
                // Insert at head
                return this->insert(node, INSERT_START);
            }

            // Actual Manipulation
            prevNode->mLinkages[this->mLinkerInUse].next = node;
            node->mLinkages[this->mLinkerInUse].prev = prevNode;
            curNode->mLinkages[this->mLinkerInUse].prev = node;
            node->mLinkages[this->mLinkerInUse].next = curNode;

            this->mSize++;
            return RC_SUCCESS;
        }

        default:
            return RC_BAD_ARG;
    }

    return RC_BAD_ARG;
}

// Insert according to policy
// Begin traversing beginning at the head, until the element is inserted.
// In such iteration the policy callback is called, waiting for it to return true.
// Assume 1-based indexing, then in the n'th iteration the n'th position element will be encountered.
// If the policyCB returns true at this point, then the new node will be inserted just before this element.
// i.e. the node will be inserted at the (n - 1)th position.
// Where n is the position in the DLL, where policyCB returns true (policyCB(newNode, nth_pos_node) == true).
// Once inserted, break out of the loop
// If empty list is encountered, initialize a new head and tail for it.
ErrCode DLManager::insertWithPolicy(CoreIterable* node, DLPolicy policyCB) {
    if(node == nullptr) {
        return RC_INVALID_VALUE;
    }

    CoreIterable* currNode = this->mHead;
    int8_t inserted = false;

    while(currNode != nullptr) {
        CoreIterable* currNext = currNode->mLinkages[this->mLinkerInUse].next;

        if(!inserted && policyCB(node, currNode)) {
            node->mLinkages[this->mLinkerInUse].next = currNode;
            node->mLinkages[this->mLinkerInUse].prev = currNode->mLinkages[this->mLinkerInUse].prev;

            if(currNode->mLinkages[this->mLinkerInUse].prev == nullptr) {
                currNode->mLinkages[this->mLinkerInUse].prev = node;
                if(this->mHead == nullptr) {
                    this->mTail = nullptr;
                    this->mHead = nullptr;
                } else {
                    this->mHead = node;
                }

            } else {
                currNode->mLinkages[this->mLinkerInUse].prev->mLinkages[this->mLinkerInUse].next = node;
                currNode->mLinkages[this->mLinkerInUse].prev = node;
            }
            inserted = true;
            break;
        }
        currNode = currNext;
    }

    if(!inserted) {
        CoreIterable* tail = this->mTail;
        node->mLinkages[this->mLinkerInUse].next = nullptr;
        node->mLinkages[this->mLinkerInUse].prev = tail;

        if(tail != nullptr) {
            tail->mLinkages[this->mLinkerInUse].next = node;
        } else {
            this->mHead = node;
        }

        this->mTail = node;
    }

    this->mSize++;
    return RC_SUCCESS;
}

ErrCode DLManager::insertAsc(CoreIterable* node) {
    if(this->mSavedPolicies.mAscPolicy == nullptr) {
        return RC_BAD_ARG;
    }
    return this->insertWithPolicy(node, this->mSavedPolicies.mAscPolicy);
}

ErrCode DLManager::insertDesc(CoreIterable* node) {
    if(this->mSavedPolicies.mDescPolicy == nullptr) {
        return RC_BAD_ARG;
    }
    return this->insertWithPolicy(node, this->mSavedPolicies.mDescPolicy);
}

int8_t DLManager::isNodeNth(int32_t n, CoreIterable* node) {
    int32_t position = 0;
    CoreIterable* currNode = this->mHead;

    while(currNode != nullptr) {
        if(position == n) {
            return (currNode == node);
        }
        currNode = currNode->mLinkages[this->mLinkerInUse].next;
        position++;
    }

    return false;
}

int8_t DLManager::matchAgainst(DLManager* target, DLPolicy cmpPolicy) {
    if(target == nullptr) return false;
    if(this->getLen() != target->getLen()) return false;

    CoreIterable* srcCur = this->mHead;
    CoreIterable* targetCur = target->mHead;

    while(srcCur != nullptr && targetCur != nullptr) {
        if(cmpPolicy == nullptr) {
            // Match raw addresses
            if(srcCur != targetCur) return false;
        } else {
            if(!cmpPolicy(srcCur, targetCur)) {
                return false;
            }
        }

        srcCur = srcCur->mLinkages[this->mLinkerInUse].next;
        targetCur = targetCur->mLinkages[target->mLinkerInUse].next;
    }

    if(srcCur == nullptr && targetCur == nullptr) return true;
    if(srcCur == nullptr || targetCur == nullptr) return false;
    return true;
}

int32_t DLManager::getLen() {
    return this->mSize;
}

ErrCode DLManager::deleteNode(CoreIterable* node) {
    if(node == nullptr) {
        return RC_INVALID_VALUE;
    }
    if(node->mLinkages[this->mLinkerInUse].prev) {
        node->mLinkages[this->mLinkerInUse].prev->mLinkages[this->mLinkerInUse].next =
            node->mLinkages[this->mLinkerInUse].next;
    } else {
        // Node is at the head
        this->mHead = node->mLinkages[this->mLinkerInUse].next;
        if(this->mHead == nullptr) {
            this->mTail = nullptr;
        }
    }

    if(node->mLinkages[this->mLinkerInUse].next) {
        node->mLinkages[this->mLinkerInUse].next->mLinkages[this->mLinkerInUse].prev =
            node->mLinkages[this->mLinkerInUse].prev;
    } else {
        // Node is at the tail
        this->mTail = node->mLinkages[this->mLinkerInUse].prev;
        if(this->mTail == nullptr) {
            this->mHead = nullptr;
        }
    }

    this->mSize--;
    return RC_SUCCESS;
}

void DLManager::destroy() {
    this->mHead = this->mTail = nullptr;
    this->mSize = 0;
}
