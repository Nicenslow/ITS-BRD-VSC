/**
 * @file main.c
 * @author Daniel Biedermann, HAW Hamburg
 * @date Oktober 2026
 * @brief Hauptschleife des RPN-Taschenrechners.
 */

#include "main.h"

#include "arithmetic.h"
#include "calc_error.h"
#include "display.h"
#include "format.h"
#include "init.h"
#include "scanner.h"
#include "stack.h"

#include <stdbool.h>

#define TOP_VALUES_TO_PRINT 3

static void reset_calculator(Stack *stack) {
   stack_clear(stack);
   clearEchoTerm();
   clearStdout();
   setNormalMode();
}

static Result print_top(const Stack *stack) {
   int value;
   char buffer[INT_STR_BUF_SIZE];
   Result status = stack_peek(stack, &value);
   if (status != RESULT_OK) {
      return status;
   }

   int_to_string(value, buffer);
   printStdout(buffer);
   printStdout("\n");
   return RESULT_OK;
}

/* Bis zu drei oberste Werte, der oberste zuletzt. Weniger Eintraege werden
 * komplett gezeigt. */
static Result print_top_three(const Stack *stack) {
   int values[TOP_VALUES_TO_PRINT];
   int count = stack_copy_top(stack, TOP_VALUES_TO_PRINT, values);
   int index;
   char buffer[INT_STR_BUF_SIZE];

   if (count == 0) {
      return ERROR_STACK_UNDERFLOW;
   }

   for (index = 0; index < count; index++) {
      int_to_string(values[index], buffer);
      printStdout(buffer);
      if (index + 1 < count) {
         printStdout(" ");
      }
   }
   printStdout("\n");
   return RESULT_OK;
}

static Result handle_token(Stack *stack, T_token token) {
   switch (token.tok) {
   case NUMBER:
      return stack_push(stack, token.val);
   case PLUS:
      return arithmetic_add(stack);
   case MINUS:
      return arithmetic_sub(stack);
   case MULT:
      return arithmetic_mul(stack);
   case DIV:
      return arithmetic_div(stack);
   case PRT:
      return print_top(stack);
   case PRT_ALL:
      return print_top_three(stack);
   case DOUBLE:
      return stack_duplicate(stack);
   case SWAP:
      return stack_swap(stack);
   case OVERFLOW:
      return ERROR_INPUT_OVERFLOW;
   case CLEAR:
      reset_calculator(stack);
      return RESULT_OK;
   default:
      return RESULT_OK;
   }
}

int main(void) {
   Stack stack;
   bool error_latched = false;

   initITSboard();
   initDisplay();
   setNormalMode();
   stack_init(&stack);

   for (;;) {
      T_token token = nextToken();
      Result op_result;

      if (error_latched) {
         if (token.tok == CLEAR) {
            reset_calculator(&stack);
            printStdout("Reset.");
            error_latched = false;
         }
         continue;
      }

      op_result = handle_token(&stack, token);
      if (op_result != RESULT_OK) {
         calc_error_print(op_result);
         error_latched = true;
      }
   }
}
