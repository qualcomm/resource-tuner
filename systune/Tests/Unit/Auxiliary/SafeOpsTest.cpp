#include <gtest/gtest.h>
#include "SafeOps.h"

// Test cases for Add function
TEST(AddTest, Overflow) {
    OperationStatus status;
    //demonstrating implicit conversion by compiler resulting in proper value but still considered overflow
    int64_t result = Add(std::numeric_limits<int32_t>::max(), 2, status);
    ASSERT_EQ(status, OVERFLOW);
    ASSERT_EQ(result, std::numeric_limits<int32_t>::max());

}

TEST(AddTest, Underflow) {
    OperationStatus status;
    int32_t result = Add(std::numeric_limits<int32_t>::lowest(), -1, status);
    ASSERT_EQ(status, UNDERFLOW);
    ASSERT_EQ(result, std::numeric_limits<int32_t>::lowest());
}

TEST(AddTest, PositiveNoOverflow) {
    OperationStatus status;
    int8_t result = Add(10, 20, status);
    ASSERT_EQ(status, SUCCESS);
    ASSERT_EQ(result, 30);
}

TEST(AddTest, NegativeNoUnderflow) {
    OperationStatus status;
    int8_t result = Add(-10, -20, status);
    ASSERT_EQ(status, SUCCESS);
    ASSERT_EQ(result, -30);
}

TEST(AddTest, IncorrectType) {
    OperationStatus status;
    // based on the return type, -2 is assigned
    uint8_t result = Add(1,-2,status);
    ASSERT_EQ(status, SUCCESS);
    ASSERT_EQ(result, 255);
}

TEST(AddTest, DifferentTypes) {
    OperationStatus status;
    int8_t a = 127;
    int16_t b = 123;
    int16_t result = Add(static_cast<int16_t>(a),b,status);
    ASSERT_EQ(status, SUCCESS);
    ASSERT_EQ(result, 250);
}

// Test cases for Subtract function
TEST(SubtractTest, Overflow) {
    OperationStatus status;
    int64_t result = Subtract(std::numeric_limits<int64_t>::max(), static_cast<int64_t>(-1), status);
    ASSERT_EQ(status, OVERFLOW);
    ASSERT_EQ(result, std::numeric_limits<int64_t>::max());
}

TEST(SubtractTest, Underflow) {
    OperationStatus status;
    int32_t result = Subtract(std::numeric_limits<int32_t>::lowest(), 1, status);
    ASSERT_EQ(status, UNDERFLOW);
    ASSERT_EQ(result, std::numeric_limits<int32_t>::lowest());
}

TEST(SubtractTest, PositiveNoOverflow) {
    OperationStatus status;
    int8_t result = Subtract(20, 10, status);
    ASSERT_EQ(status, SUCCESS);
    ASSERT_EQ(result, 10);
}

TEST(SubtractTest, NegativeNoUnderflow) {
    OperationStatus status;
    int8_t result = Subtract(-20, -10, status);
    ASSERT_EQ(status, SUCCESS);
    ASSERT_EQ(result, -10);
}

TEST(MultiplyTest, Underflow) {
     OperationStatus status;
     int64_t result = Multiply(std::numeric_limits<int64_t>::lowest(), static_cast<int64_t>(2), status);
     ASSERT_EQ(status, UNDERFLOW);
     ASSERT_EQ(result, std::numeric_limits<int64_t>::lowest());
}

TEST(MultiplyTest, PositiveNoOverflow) {
     OperationStatus status;
     int64_t result = Multiply(10, 20, status);
     ASSERT_EQ(status, SUCCESS);
     ASSERT_EQ(result, 200);
}

TEST(MultiplyTest, DoublePositiveOverflow) {
    OperationStatus status;
    double result = Multiply(std::numeric_limits<double>::max(), 2.7, status);
    ASSERT_EQ(status, OVERFLOW);
    ASSERT_EQ(result, std::numeric_limits<double>::max());
}

TEST(MultiplyTest, DoubleUnderflow) {
     OperationStatus status;
     double result = Multiply(2.0, std::numeric_limits<double>::lowest(), status);
     ASSERT_EQ(status, UNDERFLOW);
     ASSERT_EQ(result, std::numeric_limits<double>::lowest());
}

TEST(MultiplyTest, DoublePositiveNoOverflow) {
     OperationStatus status;
     double result = Multiply(10.0, 2.0, status);
     ASSERT_EQ(status, SUCCESS);
     ASSERT_EQ(result, 20.0);
}

TEST(DivideTest, DivByZero) {
    OperationStatus status;
    double result = Divide(10.0, 0.0, status);
    ASSERT_EQ(status, DIVISION_BY_ZERO);
    ASSERT_EQ(result, 10.0);
}

TEST(DivideTest, PositiveOverflow) {
    OperationStatus status;
    double result = Divide(std::numeric_limits<double>::max(), 0.5, status);
    ASSERT_EQ(status, OVERFLOW);
    ASSERT_EQ(result, std::numeric_limits<double>::max());
}

TEST(DivideTest, Underflow) {
    OperationStatus status;
    double result = Divide(std::numeric_limits<double>::max(), -0.5, status);
    ASSERT_EQ(status, UNDERFLOW);
    ASSERT_EQ(result, std::numeric_limits<double>::lowest());
}
