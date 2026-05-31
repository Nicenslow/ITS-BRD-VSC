/**
 * @file stack.h
 * @author Daniel Biedermann, HAW Hamburg
 * @date April 2026
 * @brief Header file for stack operations.
 */

#ifndef STACK_H
#define STACK_H

#include <stdbool.h>

/* Maximum amount of values stored on calculator stack. */
#define STACK_CAPACITY 32

/* Uniform result codes used by stack and arithmetic functions. */
typedef enum {
    RESULT_OK = 0,
    ERROR_STACK_OVERFLOW,
    ERROR_STACK_UNDERFLOW,
    ERROR_DIV_ZERO,
    ERROR_ARITHMETIC_OVERFLOW
} Result;

/* Stack implemented as array + index of next free slot. */
typedef struct {
    int data[STACK_CAPACITY];
    int top;
} Stack;

/*
 ****************************************************************************************
 * @brief Initialize stack as empty.
 *
 * @param stack Stack instance to initialize.
 *
 * @return void
 ****************************************************************************************/
void stack_init(Stack *stack);

/*
 ****************************************************************************************
 * @brief Remove all elements from stack.
 *
 * @param stack Stack instance to clear.
 *
 * @return void
 ****************************************************************************************/
void stack_clear(Stack *stack);

/*
 ****************************************************************************************
 * @brief Check whether stack currently contains no elements.
 *
 * @param stack Stack instance to inspect.
 *
 * @return true if empty, otherwise false.
 ****************************************************************************************/
bool stack_is_empty(const Stack *stack);

/*
 ****************************************************************************************
 * @brief Push one value onto the stack.
 *
 * @param stack Stack instance to modify.
 * @param value Value to push.
 *
 * @return RESULT_OK or ERROR_STACK_OVERFLOW.
 ****************************************************************************************/
Result stack_push(Stack *stack, int value);

/*
 ****************************************************************************************
 * @brief Pop top value from stack.
 *
 * @param stack Stack instance to modify.
 * @param out_value Receives popped value on success.
 *
 * @return RESULT_OK or ERROR_STACK_UNDERFLOW.
 ****************************************************************************************/
Result stack_pop(Stack *stack, int *out_value);

/*
 ****************************************************************************************
 * @brief Read top value without removing it.
 *
 * @param stack Stack instance to inspect.
 * @param out_value Receives top value on success.
 *
 * @return RESULT_OK or ERROR_STACK_UNDERFLOW.
 ****************************************************************************************/
Result stack_peek(const Stack *stack, int *out_value);

#endif /* STACK_H */
