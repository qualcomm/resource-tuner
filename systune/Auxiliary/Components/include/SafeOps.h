// Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
// SPDX-License-Identifier: BSD-3-Clause-Clear

#ifndef SAFE_OPS_H
#define SAFE_OPS_H

#include <climits>
#include <limits>
#include <stdexcept>

/*******************************************
Note: Arguments must be of the same types,
      i.e. no implict conversions
******************************************** */
enum OperationStatus {
    SUCCESS,
    OVERFLOW,
    UNDERFLOW,
    DIVISION_BY_ZERO
};

//return max or min limit.

template < typename T >
T Add(T firstArg, T secondArg, OperationStatus& status) {
    status = SUCCESS;
    // Both arguments are +ve
    if(firstArg >= 0 && secondArg >= 0 && ((std::numeric_limits<T>::max() - firstArg) < secondArg)) {
        status = OVERFLOW;
        return std::numeric_limits<T>::max();
    }
    // Both arguments are -ve
    else if(firstArg < 0 && secondArg < 0 && ((std::numeric_limits<T>::lowest() - firstArg) > secondArg)) {
        status = UNDERFLOW;
        return std::numeric_limits<T>::lowest();
    }
    return firstArg + secondArg;
}

template < typename T >
T Subtract(T firstArg, T secondArg, OperationStatus& status)  {
    status = SUCCESS;
    //Only opposite signed arguments will cause undesired behaviour
    if(firstArg >= 0 && secondArg < 0 && ((firstArg - std::numeric_limits<T>::max()) > secondArg)) {
        status = OVERFLOW;
        return std::numeric_limits<T>::max();
    }
    else if(firstArg < 0 && secondArg >=0  && ((firstArg - std::numeric_limits<T>::lowest()) < secondArg)) {
        status = UNDERFLOW;
        return std::numeric_limits<T>::lowest();
    }
    return firstArg - secondArg;
}

template < typename T >
T Multiply(T firstArg, T secondArg, OperationStatus& status)  {
    status = SUCCESS;
    if(firstArg > 0 && secondArg >0  && ((std::numeric_limits<T>::max()/firstArg) < secondArg)) {
        status = OVERFLOW;
        return std::numeric_limits<T>::max();
    }
    else if(firstArg < 0 && secondArg < 0  && ((std::numeric_limits<T>::max()/firstArg) > secondArg)) {
        status = OVERFLOW;
        return std::numeric_limits<T>::max();
    }
    else if(firstArg > 0 && secondArg < 0  && ((std::numeric_limits<T>::lowest()/firstArg) > secondArg)) {
        status = UNDERFLOW;
        return std::numeric_limits<T>::lowest();
    }
    else if(firstArg < 0 && secondArg > 0  && ((std::numeric_limits<T>::lowest()/firstArg) < secondArg)) {
        status = UNDERFLOW;
        return std::numeric_limits<T>::lowest();
    }
    return firstArg*secondArg;
}


template < typename T >
T Divide(T firstArg, T secondArg, OperationStatus& status)  {
    status = SUCCESS;
    if(secondArg==0) {
        status = DIVISION_BY_ZERO;
        return firstArg;
    }
    if(secondArg > 0 && secondArg < 1 && firstArg > 0 && ((std::numeric_limits<T>::max()*secondArg) < firstArg)) {
        status = OVERFLOW;
        return std::numeric_limits<T>::max();
    }
    else if(secondArg > 0 && secondArg < 1 && firstArg < 0 && ((std::numeric_limits<T>::lowest()*secondArg) > firstArg)) {
        status = UNDERFLOW;
        return std::numeric_limits<T>::lowest();
    }
    else if(secondArg < 0 && secondArg > -1 && firstArg > 0 && ((std::numeric_limits<T>::lowest()*secondArg) < firstArg)) {
        status = UNDERFLOW;
        return std::numeric_limits<T>::lowest();
    }
    else if(secondArg < 0 && secondArg > -1 && firstArg < 0 &&  ((std::numeric_limits<T>::max()*secondArg) > firstArg)) {
        status = OVERFLOW;
        return std::numeric_limits<T>::max();
    }

    return firstArg/secondArg;
}

#define SafeDeref(ptr) \
    (ptr == nullptr) ? throw std::invalid_argument("Null Pointer Dereference") : *ptr

#define SafeAssignment(ptr, val) \
    (ptr == nullptr) ? throw std::invalid_argument("Null Pointer Assignment") : *ptr = val

#define ASSIGN_AND_INCR(ptr, val)        \
    SafeAssignment(ptr, val);            \
    ptr++;                               \

#define DEREF_AND_INCR(ptr, type) ({     \
    type val = (type)(SafeDeref(ptr));   \
    ptr++;                               \
    val;                                 \
})

#endif
