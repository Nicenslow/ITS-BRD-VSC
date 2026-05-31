/**
 * @file arithmetic.h
 * @author Daniel Biedermann, HAW Hamburg
 * @date April 2026
 * @brief Header file for arithmetic stack operations.
 */

#ifndef ARITHMETIC_H
#define ARITHMETIC_H

#include "stack.h"

/*
 ****************************************************************************************
 * @brief Pop two values, add them, and push the result.
 *
 * @param stack Stack instance to modify.
 *
 * @return RESULT_OK or stack/arithmetic error.
 ****************************************************************************************/
Result arithmetic_add(Stack *stack);

/*
 ****************************************************************************************
 * @brief Pop two values, subtract rhs from lhs, and push the result.
 *
 * @param stack Stack instance to modify.
 *
 * @return RESULT_OK or stack/arithmetic error.
 ****************************************************************************************/
Result arithmetic_sub(Stack *stack);

/*
 ****************************************************************************************
 * @brief Pop two values, multiply them, and push the result.
 *
 * @param stack Stack instance to modify.
 *
 * @return RESULT_OK or stack/arithmetic error.
 ****************************************************************************************/
Result arithmetic_mul(Stack *stack);

/*
 ****************************************************************************************
 * @brief Pop two values, divide lhs by rhs, and push the result.
 *
 * @param stack Stack instance to modify.
 *
 * @return RESULT_OK or stack/arithmetic error.
 ****************************************************************************************/
Result arithmetic_div(Stack *stack);

#endif /* ARITHMETIC_H */
