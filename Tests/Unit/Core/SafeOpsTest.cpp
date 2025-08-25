// Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
// SPDX-License-Identifier: BSD-3-Clause-Clear

#include <iostream>
#include <cstdint>

#include "TestUtils.h"
#include "SafeOps.h"

// Test cases for Add function
static void Overflow1() {
    OperationStatus status;
    //demonstrating implicit conversion by compiler resulting in proper value but still considered overflow
    int64_t result = Add(std::numeric_limits<int32_t>::max(), 2, status);
    C_ASSERT(status == OVERFLOW);
    C_ASSERT(result == std::numeric_limits<int32_t>::max());
}

static void Underflow1() {
    OperationStatus status;
    int32_t result = Add(std::numeric_limits<int32_t>::lowest(), -1, status);
    C_ASSERT(status == UNDERFLOW);
    C_ASSERT(result == std::numeric_limits<int32_t>::lowest());
}

static void PositiveNoOverflow1() {
    OperationStatus status;
    int8_t result = Add(10, 20, status);
    C_ASSERT(status == SUCCESS);
    C_ASSERT(result == 30);
}

static void NegativeNoUnderflow1() {
    OperationStatus status;
    int8_t result = Add(-10, -20, status);
    C_ASSERT(status == SUCCESS);
    C_ASSERT(result == -30);
}

static void IncorrectType1() {
    OperationStatus status;
    // based on the return type, -2 is assigned
    uint8_t result = Add(1,-2,status);
    C_ASSERT(status == SUCCESS);
    C_ASSERT(result == 255);
}

static void DifferentTypes() {
    OperationStatus status;
    int8_t a = 127;
    int16_t b = 123;
    int16_t result = Add(static_cast<int16_t>(a),b,status);
    C_ASSERT(status == SUCCESS);
    C_ASSERT(result == 250);
}

// Test cases for Subtract function
static void Overflow2() {
    OperationStatus status;
    int64_t result = Subtract(std::numeric_limits<int64_t>::max(), static_cast<int64_t>(-1), status);
    C_ASSERT(status == OVERFLOW);
    C_ASSERT(result == std::numeric_limits<int64_t>::max());
}

static void Underflow2() {
    OperationStatus status;
    int32_t result = Subtract(std::numeric_limits<int32_t>::lowest(), 1, status);
    C_ASSERT(status == UNDERFLOW);
    C_ASSERT(result == std::numeric_limits<int32_t>::lowest());
}

static void PositiveNoOverflow2() {
    OperationStatus status;
    int8_t result = Subtract(20, 10, status);
    C_ASSERT(status == SUCCESS);
    C_ASSERT(result == 10);
}

static void NegativeNoUnderflow2() {
    OperationStatus status;
    int8_t result = Subtract(-20, -10, status);
    C_ASSERT(status == SUCCESS);
    C_ASSERT(result == -10);
}

static void Underflow3() {
     OperationStatus status;
     int64_t result = Multiply(std::numeric_limits<int64_t>::lowest(), static_cast<int64_t>(2), status);
     C_ASSERT(status == UNDERFLOW);
     C_ASSERT(result == std::numeric_limits<int64_t>::lowest());
}

static void PositiveNoOverflow3() {
     OperationStatus status;
     int64_t result = Multiply(10, 20, status);
     C_ASSERT(status == SUCCESS);
     C_ASSERT(result == 200);
}

static void DoublePositiveOverflow() {
    OperationStatus status;
    double result = Multiply(std::numeric_limits<double>::max(), 2.7, status);
    C_ASSERT(status == OVERFLOW);
    C_ASSERT(result == std::numeric_limits<double>::max());
}

static void DoubleUnderflow() {
    OperationStatus status;
    double result = Multiply(2.0, std::numeric_limits<double>::lowest(), status);
    C_ASSERT(status == UNDERFLOW);
    C_ASSERT(result == std::numeric_limits<double>::lowest());
}

static void DoublePositiveNoOverflow() {
    OperationStatus status;
    double result = Multiply(10.0, 2.0, status);
    C_ASSERT(status == SUCCESS);
    C_ASSERT(result == 20.0);
}

static void DivByZero() {
    OperationStatus status;
    double result = Divide(10.0, 0.0, status);
    C_ASSERT(status == DIVISION_BY_ZERO);
    C_ASSERT(result == 10.0);
}

static void PositiveOverflow() {
    OperationStatus status;
    double result = Divide(std::numeric_limits<double>::max(), 0.5, status);
    C_ASSERT(status == OVERFLOW);
    C_ASSERT(result == std::numeric_limits<double>::max());
}

static void Underflow4() {
    OperationStatus status;
    double result = Divide(std::numeric_limits<double>::max(), -0.5, status);
    C_ASSERT(status == UNDERFLOW);
    C_ASSERT(result == std::numeric_limits<double>::lowest());
}

static void TestSafeDerefMacro() {
    int32_t* int_ptr = nullptr;
    int8_t exceptionHit = false;
    try {
        SafeDeref(int_ptr);
    } catch(const std::invalid_argument& e) {
        exceptionHit = true;
    }

    C_ASSERT(exceptionHit == true);
}

static void TestSafeAssignmentMacro() {
    int32_t* int_ptr = nullptr;
    int8_t exceptionHit = false;
    try {
        SafeAssignment(int_ptr, 57);
    } catch(const std::invalid_argument& e) {
        exceptionHit = true;
    }

    C_ASSERT(exceptionHit == true);
}

static void TestSafeStaticCastMacro() {
    int32_t* int_ptr = nullptr;
    int8_t exceptionHit = false;
    try {
        SafeStaticCast(int_ptr, void*);
    } catch(const std::invalid_argument& e) {
        exceptionHit = true;
    }

    C_ASSERT(exceptionHit == true);
}

static void TestValidationMacro1() {
    int32_t val = -670;
    int8_t exceptionHit = false;
    try {
        VALIDATE_GT(val, 0);
    } catch(const std::invalid_argument& e) {
        exceptionHit = true;
    }

    C_ASSERT(exceptionHit == true);
}

static void TestValidationMacro2() {
    int32_t val = 100;
    int8_t exceptionHit = false;
    try {
        VALIDATE_GE(val, 100);
    } catch(const std::invalid_argument& e) {
        exceptionHit = true;
    }

    C_ASSERT(exceptionHit == false);
}

int32_t main() {
    std::cout<<"Running SafeOps Test Suite\n"<<std::endl;

    RUN_TEST(Overflow1);
    RUN_TEST(Underflow1);
    RUN_TEST(PositiveNoOverflow1);
    RUN_TEST(NegativeNoUnderflow1);
    RUN_TEST(IncorrectType1);
    RUN_TEST(DifferentTypes);
    RUN_TEST(Overflow2);
    RUN_TEST(Underflow2);
    RUN_TEST(PositiveNoOverflow2);
    RUN_TEST(NegativeNoUnderflow2);
    RUN_TEST(Underflow3);
    RUN_TEST(PositiveNoOverflow3);
    RUN_TEST(DoublePositiveOverflow);
    RUN_TEST(DoubleUnderflow);
    RUN_TEST(DoublePositiveNoOverflow);
    RUN_TEST(DivByZero);
    RUN_TEST(PositiveOverflow);
    RUN_TEST(Underflow4);
    RUN_TEST(TestSafeDerefMacro);
    RUN_TEST(TestSafeAssignmentMacro);
    RUN_TEST(TestSafeStaticCastMacro);
    RUN_TEST(TestValidationMacro1);
    RUN_TEST(TestValidationMacro2);

    std::cout<<"\nAll Tests from the suite: [SafeOps], executed successfully"<<std::endl;
}
