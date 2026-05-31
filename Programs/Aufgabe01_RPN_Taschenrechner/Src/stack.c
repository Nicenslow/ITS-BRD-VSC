/**
 * @file stack.c
 * @author Daniel Biedermann, HAW Hamburg
 * @date April 2026
 * @brief This module implements basic stack operations.
 */

#include "stack.h"

/*
 ****************************************************************************************
 * @brief Initialize stack as empty.
 *
 * @param stack Stack instance to initialize.
 *
 * @return void
 ****************************************************************************************/
void stack_init(Stack *stack) {
    /* Empty stack: no valid elements yet. */
    stack->top = 0;
}

/*
 ****************************************************************************************
 * @brief Remove all elements from stack.
 *
 * @param stack Stack instance to clear.
 *
 * @return void
 ****************************************************************************************/
void stack_clear(Stack *stack) {
    stack->top = 0;
}

/*
 ****************************************************************************************
 * @brief Check whether stack currently contains no elements.
 *
 * @param stack Stack instance to inspect.
 *
 * @return true if empty, otherwise false.
 ****************************************************************************************/
bool stack_is_empty(const Stack *stack) {
    return stack->top == 0;
}

/*
 ****************************************************************************************
 * @brief Push one value onto the stack.
 *
 * @param stack Stack instance to modify.
 * @param value Value to push.
 *
 * @return RESULT_OK or ERROR_STACK_OVERFLOW.
 ****************************************************************************************/
Result stack_push(Stack *stack, int value) {
    if (stack->top >= STACK_CAPACITY) {
        return ERROR_STACK_OVERFLOW;
    }

    stack->data[stack->top++] = value;
    return RESULT_OK;
}

/*
 ****************************************************************************************
 * @brief Pop top value from stack.
 *
 * @param stack Stack instance to modify.
 * @param out_value Receives popped value on success.
 *
 * @return RESULT_OK or ERROR_STACK_UNDERFLOW.
 ****************************************************************************************/
Result stack_pop(Stack *stack, int *out_value) {
    if (stack->top <= 0) {
        return ERROR_STACK_UNDERFLOW;
    }

    stack->top--;
    *out_value = stack->data[stack->top];
    return RESULT_OK;
}

/*
 ****************************************************************************************
 * @brief Read top value without removing it.
 *
 * @param stack Stack instance to inspect.
 * @param out_value Receives top value on success.
 *
 * @return RESULT_OK or ERROR_STACK_UNDERFLOW.
 ****************************************************************************************/
Result stack_peek(const Stack *stack, int *out_value) {
    if (stack->top <= 0) {
        return ERROR_STACK_UNDERFLOW;
    }

    *out_value = stack->data[stack->top - 1];
    return RESULT_OK;
}

