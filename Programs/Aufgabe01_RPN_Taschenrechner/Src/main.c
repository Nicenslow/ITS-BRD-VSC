/**
 * @file main.c
 * @author Daniel Biedermann, HAW Hamburg
 * @date April 2026
 * @brief Main control loop for the RPN calculator.
 */

#include "display.h"
#include "scanner.h"
#include "stack.h"
#include "token.h"
#include "arithmetic.h"
#include "stm32f4xx_hal.h"
#include "init.h"
#include "LCD_GUI.h"
#include "LCD_Touch.h"
#include "lcd.h"

#include "fontsFLASH.h"
#include "additionalFonts.h"
#include "error.h"

#include <stdbool.h>

/* Provided by the ITS board support package. */
extern void initITSboard(void);

/* Enough space for 32-bit int string plus '\0' (e.g. "-2147483648"). */
#define INT_STR_BUF_SIZE 16

/*
 * Convert integer to ASCII without sprintf:
 * - collect digits in reverse order
 * - append '-' for negative values
 * - reverse final buffer
 */
/*
 ****************************************************************************************
 * @brief Convert an integer value into a zero-terminated decimal string.
 *
 * @param val Value to convert.
 * @param buf Output buffer receiving the ASCII string.
 *
 * @return void
 ****************************************************************************************/
void int_to_string(int val, char *buf) {
    unsigned int magnitude;
    int idx = 0;
    int left;
    int right;

    if (val < 0) {
        /* Compute |val| without triggering UB for INT_MIN. */
        magnitude = (unsigned int)(-(val + 1)) + 1U;
    } else {
        magnitude = (unsigned int)val;
    }

    do {
        unsigned int digit = magnitude % 10U;
        buf[idx++] = (char)('0' + digit);
        magnitude /= 10U;
    } while (magnitude > 0U);

    if (val < 0) {
        buf[idx++] = '-';
    }

    buf[idx] = '\0';

    for (left = 0, right = idx - 1; left < right; left++, right--) {
        char tmp = buf[left];
        buf[left] = buf[right];
        buf[right] = tmp;
    }
}

/*
 ****************************************************************************************
 * @brief Read the next token from scanner module.
 *
 * @return Next input token.
 ****************************************************************************************/
static T_token scanner_get_token(void) {
    return nextToken();
}

/* Reset stack and terminal view after CLEAR. */
/*
 ****************************************************************************************
 * @brief Clear stack and reset display state.
 *
 * @param stack Stack instance to reset.
 *
 * @return void
 ****************************************************************************************/
static void reset_calculator(Stack *stack) {
    stack_clear(stack);
    clearEchoTerm();
    clearStdout();
    setNormalMode();
}

/* Print current stack content from bottom to top without modifying it. */
/*
 ****************************************************************************************
 * @brief Print all stack values from bottom to top.
 *
 * @param stack Stack to print.
 *
 * @return RESULT_OK after printing.
 ****************************************************************************************/
static Result print_full_stack(const Stack *stack) {
    int i;
    char out_buf[INT_STR_BUF_SIZE + 1]; // +1 Platz für das Trennzeichen

    if (stack->top == 0) {
        printStdout("[empty]");
        return RESULT_OK;
    }

    for (i = 0; i < stack->top; i++) {
        int_to_string(stack->data[i], out_buf);
        
        // Manuelles Anhängen eines Trennzeichens (z.B. Leerzeichen)
        int len = 0;
        while (out_buf[len] != '\0') len++; // Länge finden
        
        out_buf[len] = ' ';      // Leerzeichen setzen
        out_buf[len + 1] = '\0'; // Neuen Nullterminator setzen
        
        printStdout(out_buf);
    }
    return RESULT_OK;
}

/* Duplicate top element: x -> x x. */
/*
 ****************************************************************************************
 * @brief Duplicate the current top element.
 *
 * @param stack Stack to modify.
 *
 * @return RESULT_OK or corresponding stack error.
 ****************************************************************************************/
static Result duplicate_top(Stack *stack) {
    int top_value;
    Result rc = stack_peek(stack, &top_value);
    if (rc != RESULT_OK) {
        return rc;
    }
    return stack_push(stack, top_value);
}

/* Swap top two elements: a b -> b a. */
/*
 ****************************************************************************************
 * @brief Swap the two top-most stack elements.
 *
 * @param stack Stack to modify.
 *
 * @return RESULT_OK or corresponding stack error.
 ****************************************************************************************/
static Result swap_top_two(Stack *stack) {
    int first;
    int second;
    Result rc = stack_pop(stack, &first);
    if (rc != RESULT_OK) {
        return rc;
    }

    rc = stack_pop(stack, &second);
    if (rc != RESULT_OK) {
        /* Restore previous state if only one element existed. */
        (void)stack_push(stack, first);
        return rc;
    }

    rc = stack_push(stack, first);
    if (rc != RESULT_OK) {
        return rc;
    }
    return stack_push(stack, second);
}

/*
 ****************************************************************************************
 * @brief Run the calculator event loop.
 *
 * @return Never returns during normal operation.
 ****************************************************************************************/
int main(void) {
    Stack stack;
    bool error_latched = false;

    initITSboard();
    initDisplay();
    setNormalMode();
    stack_init(&stack);

    for (;;) {
        T_token token = scanner_get_token();
        Result op_result = RESULT_OK;

        if (error_latched) {
            if (token.tok == CLEAR) {
                reset_calculator(&stack);
                printStdout("Reset.");
                error_latched = false;
            }
            continue;
        }

        switch (token.tok) {
            case NUMBER:
                op_result = stack_push(&stack, token.val);
                break;

            case PLUS:
                op_result = arithmetic_add(&stack);
                break;

            case MINUS:
                op_result = arithmetic_sub(&stack);
                break;

            case MULT:
                op_result = arithmetic_mul(&stack);
                break;

            case DIV:
                op_result = arithmetic_div(&stack);
                break;

            case PRT: {
                int top_value;
                char out_buf[INT_STR_BUF_SIZE];
                op_result = stack_peek(&stack, &top_value);
                if (op_result == RESULT_OK) {
                    int_to_string(top_value, out_buf);
                    printStdout(out_buf);
                }
                break;
            }

            case PRT_ALL:
                op_result = print_full_stack(&stack);
                break;

            case DOUBLE:
                op_result = duplicate_top(&stack);
                break;

            case SWAP:
                op_result = swap_top_two(&stack);
                break;

            case OVERFLOW:
                op_result = ERROR_ARITHMETIC_OVERFLOW;
                break;

            case CLEAR:
                reset_calculator(&stack);
                break;

            default:
                /* Ignore unsupported or unexpected tokens. */
                break;
        }

        if (op_result != RESULT_OK) {
            error_print(op_result);
            error_latched = true;
        }
    }
}
