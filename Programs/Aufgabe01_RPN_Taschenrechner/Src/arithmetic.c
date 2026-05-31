/**
 * @file arithmetic.c
 * @author Daniel Biedermann, HAW Hamburg
 * @date April 2026
 * @brief This module implements arithmetic operations on stack values.
 */

#include "arithmetic.h"

#include <limits.h>

/*
 ****************************************************************************************
 * @brief Check whether a + b would overflow signed int.
 *
 * @param a Left operand.
 * @param b Right operand.
 *
 * @return true if overflow would occur, otherwise false.
 ****************************************************************************************/
static bool add_would_overflow(int a, int b) {
    return ((b > 0) && (a > INT_MAX - b)) || ((b < 0) && (a < INT_MIN - b));
}

/*
 ****************************************************************************************
 * @brief Check whether a - b would overflow signed int.
 *
 * @param a Left operand.
 * @param b Right operand.
 *
 * @return true if overflow would occur, otherwise false.
 ****************************************************************************************/
static bool sub_would_overflow(int a, int b) {
    return ((b < 0) && (a > INT_MAX + b)) || ((b > 0) && (a < INT_MIN + b));
}

/*
 ****************************************************************************************
 * @brief Check whether a * b would overflow signed int.
 *
 * @param a Left operand.
 * @param b Right operand.
 *
 * @return true if overflow would occur, otherwise false.
 ****************************************************************************************/
static bool mul_would_overflow(int a, int b) {
    if ((a == 0) || (b == 0)) {
        return false;
    }

    if (a > 0) {
        if (b > 0) {
            return a > INT_MAX / b;
        }
        return b < INT_MIN / a;
    }

    if (b > 0) {
        return a < INT_MIN / b;
    }

    return a < INT_MAX / b;
}

Result arithmetic_add(Stack *stack) {
    int rhs;
    int lhs;
    int sum;
    Result rc = stack_pop(stack, &rhs);
    if (rc != RESULT_OK) {
        return rc;
    }

    rc = stack_pop(stack, &lhs);
    if (rc != RESULT_OK) {
        return rc;
    }

    if (add_would_overflow(lhs, rhs)) {
        return ERROR_ARITHMETIC_OVERFLOW;
    }

    sum = lhs + rhs;
    return stack_push(stack, sum);
}

Result arithmetic_sub(Stack *stack) {
    int rhs;
    int lhs;
    int difference;
    Result rc = stack_pop(stack, &rhs);
    if (rc != RESULT_OK) {
        return rc;
    }

    rc = stack_pop(stack, &lhs);
    if (rc != RESULT_OK) {
        return rc;
    }

    if (sub_would_overflow(lhs, rhs)) {
        return ERROR_ARITHMETIC_OVERFLOW;
    }

    difference = lhs - rhs;
    return stack_push(stack, difference);
}

Result arithmetic_mul(Stack *stack) {
    int rhs;
    int lhs;
    int product;
    Result rc = stack_pop(stack, &rhs);
    if (rc != RESULT_OK) {
        return rc;
    }

    rc = stack_pop(stack, &lhs);
    if (rc != RESULT_OK) {
        return rc;
    }

    if (mul_would_overflow(lhs, rhs)) {
        return ERROR_ARITHMETIC_OVERFLOW;
    }

    product = lhs * rhs;
    return stack_push(stack, product);
}

Result arithmetic_div(Stack *stack) {
    int rhs;
    int lhs;
    int quotient;
    Result rc = stack_pop(stack, &rhs);
    if (rc != RESULT_OK) {
        return rc;
    }

    rc = stack_pop(stack, &lhs);
    if (rc != RESULT_OK) {
        return rc;
    }

    if (rhs == 0) {
        return ERROR_DIV_ZERO;
    }
    if ((lhs == INT_MIN) && (rhs == -1)) {
        return ERROR_ARITHMETIC_OVERFLOW;
    }

    quotient = lhs / rhs;
    return stack_push(stack, quotient);
}
