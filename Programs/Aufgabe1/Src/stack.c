/**
 * @file stack.c
 * @author Daniel Biedermann, HAW Hamburg
 * @date Oktober 2026
 * @brief Array-Stack fuer den RPN-Rechner.
 */

#include "stack.h"

void stack_init(Stack *stack) { stack->top = 0; }

void stack_clear(Stack *stack) { stack->top = 0; }

bool stack_is_empty(const Stack *stack) { return stack->top == 0; }

int stack_size(const Stack *stack) { return stack->top; }

Result stack_push(Stack *stack, int value) {
   if (stack->top >= STACK_CAPACITY) {
      return ERROR_STACK_OVERFLOW;
   }

   stack->data[stack->top] = value;
   stack->top++;
   return RESULT_OK;
}

Result stack_pop(Stack *stack, int *out_value) {
   if (stack->top <= 0) {
      return ERROR_STACK_UNDERFLOW;
   }

   stack->top--;
   *out_value = stack->data[stack->top];
   return RESULT_OK;
}

Result stack_peek(const Stack *stack, int *out_value) {
   if (stack->top <= 0) {
      return ERROR_STACK_UNDERFLOW;
   }

   *out_value = stack->data[stack->top - 1];
   return RESULT_OK;
}

int stack_copy_top(const Stack *stack, int max_count, int *out_oldest_first) {
   int count;
   int start;
   int index;

   if ((max_count <= 0) || (stack->top == 0)) {
      return 0;
   }

   count = stack->top;
   if (count > max_count) {
      count = max_count;
   }
   start = stack->top - count;

   for (index = 0; index < count; index++) {
      out_oldest_first[index] = stack->data[start + index];
   }
   return count;
}

Result stack_duplicate(Stack *stack) {
   int top_value;
   Result status = stack_peek(stack, &top_value);
   if (status != RESULT_OK) {
      return status;
   }
   return stack_push(stack, top_value);
}

Result stack_swap(Stack *stack) {
   int first;
   int second;
   Result status = stack_pop(stack, &first);
   if (status != RESULT_OK) {
      return status;
   }

   status = stack_pop(stack, &second);
   if (status != RESULT_OK) {
      (void)stack_push(stack, first);
      return status;
   }

   status = stack_push(stack, first);
   if (status != RESULT_OK) {
      return status;
   }
   return stack_push(stack, second);
}
