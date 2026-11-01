/**
 * @file arithmetic.c
 * @author Daniel Biedermann, HAW Hamburg
 * @date Oktober 2026
 * @brief Ganzzahlige Operationen mit explizitem Overflow-Test.
 */

#include "arithmetic.h"

#include <limits.h>
#include <stdbool.h>

/*
 * a + b laeuft ueber, wenn
 * b > 0 und a > INT_MAX - b oder
 * b < 0 und a < INT_MIN - b.
 */
static bool add_would_overflow(int left, int right) {
   if ((right > 0) && (left > INT_MAX - right)) {
      return true;
   }
   if ((right < 0) && (left < INT_MIN - right)) {
      return true;
   }
   return false;
}

/*
 * a - b laeuft ueber, wenn
 * b < 0 und a > INT_MAX + b oder
 * b > 0 und a < INT_MIN + b.
 */
static bool sub_would_overflow(int left, int right) {
   if ((right < 0) && (left > INT_MAX + right)) {
      return true;
   }
   if ((right > 0) && (left < INT_MIN + right)) {
      return true;
   }
   return false;
}

/*
 * Vorzeichen getrennt pruefen, damit die Division in der Grenze bleibt.
 * Positives Produkt: a * b > INT_MAX, fuer b != 0 also a > INT_MAX / b
 * bzw. die gespiegelte Form, wenn beide Faktoren negativ sind.
 * Negatives Produkt analog gegen INT_MIN.
 * Ein Faktor 0 ergibt kein Overflow.
 */
static bool mul_would_overflow(int left, int right) {
   if ((left == 0) || (right == 0)) {
      return false;
   }

   if (left > 0) {
      if (right > 0) {
         return left > INT_MAX / right;
      }
      return right < INT_MIN / left;
   }

   if (right > 0) {
      return left < INT_MIN / right;
   }

   return left < INT_MAX / right;
}

/* Beide Operanden holen. rhs lag oben. Bei Underflow wird zurueckgelegt. */
static Result pop_binary(Stack *stack, int *lhs, int *rhs) {
   Result status = stack_pop(stack, rhs);
   if (status != RESULT_OK) {
      return status;
   }

   status = stack_pop(stack, lhs);
   if (status != RESULT_OK) {
      (void)stack_push(stack, *rhs);
      return status;
   }

   return RESULT_OK;
}

static void restore_binary(Stack *stack, int lhs, int rhs) {
   (void)stack_push(stack, lhs);
   (void)stack_push(stack, rhs);
}

Result arithmetic_add(Stack *stack) {
   int rhs;
   int lhs;
   Result status = pop_binary(stack, &lhs, &rhs);
   if (status != RESULT_OK) {
      return status;
   }

   if (add_would_overflow(lhs, rhs)) {
      restore_binary(stack, lhs, rhs);
      return ERROR_ARITHMETIC_OVERFLOW;
   }

   return stack_push(stack, lhs + rhs);
}

Result arithmetic_sub(Stack *stack) {
   int rhs;
   int lhs;
   Result status = pop_binary(stack, &lhs, &rhs);
   if (status != RESULT_OK) {
      return status;
   }

   if (sub_would_overflow(lhs, rhs)) {
      restore_binary(stack, lhs, rhs);
      return ERROR_ARITHMETIC_OVERFLOW;
   }

   return stack_push(stack, lhs - rhs);
}

Result arithmetic_mul(Stack *stack) {
   int rhs;
   int lhs;
   Result status = pop_binary(stack, &lhs, &rhs);
   if (status != RESULT_OK) {
      return status;
   }

   if (mul_would_overflow(lhs, rhs)) {
      restore_binary(stack, lhs, rhs);
      return ERROR_ARITHMETIC_OVERFLOW;
   }

   return stack_push(stack, lhs * rhs);
}

Result arithmetic_div(Stack *stack) {
   int rhs;
   int lhs;
   Result status = pop_binary(stack, &lhs, &rhs);
   if (status != RESULT_OK) {
      return status;
   }

   if (rhs == 0) {
      restore_binary(stack, lhs, rhs);
      return ERROR_DIV_ZERO;
   }

   /* Zweierkomplement ist unsymmetrisch: INT_MIN / -1 passt nicht in int. */
   if ((lhs == INT_MIN) && (rhs == -1)) {
      restore_binary(stack, lhs, rhs);
      return ERROR_ARITHMETIC_OVERFLOW;
   }

   return stack_push(stack, lhs / rhs);
}
