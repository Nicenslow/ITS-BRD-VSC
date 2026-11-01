/**
 * @file test_logic.c
 * @author Daniel Biedermann, HAW Hamburg
 * @date Oktober 2026
 * @brief Host-Test fuer Stack, Arithmetik und Zahlenformat.
 *
 * Uebersetzt auf dem PC, ohne ITS-Board:
 * gcc -std=c99 -Wall -Wextra -I Inc tests/test_logic.c Src/stack.c
 * Src/arithmetic.c Src/format.c -o test_logic
 */

#include "arithmetic.h"
#include "format.h"
#include "stack.h"

#include <limits.h>
#include <stdio.h>
#include <string.h>

static int g_failed = 0;

static void expect_int(const char *name, int got, int want) {
   if (got != want) {
      printf("FAIL %s: got %d, want %d\n", name, got, want);
      g_failed++;
   }
}

static void expect_str(const char *name, const char *got, const char *want) {
   if (strcmp(got, want) != 0) {
      printf("FAIL %s: got \"%s\", want \"%s\"\n", name, got, want);
      g_failed++;
   }
}

static void expect_result(const char *name, Result got, Result want) {
   if (got != want) {
      printf("FAIL %s: got result %d, want %d\n", name, (int)got, (int)want);
      g_failed++;
   }
}

static int top_of(const Stack *stack) {
   int value = 0;
   if (stack_peek(stack, &value) != RESULT_OK) {
      printf("FAIL peek on empty stack\n");
      g_failed++;
   }
   return value;
}

static void test_format(void) {
   char buf[INT_STR_BUF_SIZE];

   int_to_string(0, buf);
   expect_str("format 0", buf, "0");
   int_to_string(3, buf);
   expect_str("format 3", buf, "3");
   int_to_string(-7, buf);
   expect_str("format -7", buf, "-7");
   int_to_string(INT_MAX, buf);
   expect_str("format INT_MAX", buf, "2147483647");
   int_to_string(INT_MIN, buf);
   expect_str("format INT_MIN", buf, "-2147483648");
}

static void test_example_from_assignment(void) {
   Stack stack;
   stack_init(&stack);

   expect_result("push 25", stack_push(&stack, 25), RESULT_OK);
   expect_result("push 8", stack_push(&stack, 8), RESULT_OK);
   expect_result("push 67", stack_push(&stack, 67), RESULT_OK);
   expect_result("add", arithmetic_add(&stack), RESULT_OK);
   expect_int("after add", top_of(&stack), 75);
   expect_result("swap", stack_swap(&stack), RESULT_OK);
   expect_int("after swap", top_of(&stack), 25);
   expect_result("div", arithmetic_div(&stack), RESULT_OK);
   expect_int("example result", top_of(&stack), 3);
   expect_int("example depth", stack_size(&stack), 1);
}

static void test_operations(void) {
   Stack stack;
   int copied[3];
   int index;

   stack_init(&stack);
   stack_push(&stack, 10);
   stack_push(&stack, 3);
   expect_result("sub", arithmetic_sub(&stack), RESULT_OK);
   expect_int("10-3", top_of(&stack), 7);

   stack_init(&stack);
   stack_push(&stack, 3);
   stack_push(&stack, 10);
   expect_result("sub neg", arithmetic_sub(&stack), RESULT_OK);
   expect_int("3-10", top_of(&stack), -7);

   stack_init(&stack);
   stack_push(&stack, 6);
   stack_push(&stack, 7);
   expect_result("mul", arithmetic_mul(&stack), RESULT_OK);
   expect_int("6*7", top_of(&stack), 42);

   stack_init(&stack);
   stack_push(&stack, 7);
   stack_push(&stack, 2);
   expect_result("div trunc", arithmetic_div(&stack), RESULT_OK);
   expect_int("7/2", top_of(&stack), 3);

   stack_init(&stack);
   stack_push(&stack, -7);
   stack_push(&stack, 2);
   expect_result("div neg", arithmetic_div(&stack), RESULT_OK);
   expect_int("-7/2", top_of(&stack), -3);

   stack_init(&stack);
   stack_push(&stack, 5);
   expect_result("dup", stack_duplicate(&stack), RESULT_OK);
   expect_int("dup size", stack_size(&stack), 2);
   expect_int("dup top", top_of(&stack), 5);
   expect_result("peek unchanged", stack_peek(&stack, &index), RESULT_OK);
   expect_int("size after peek", stack_size(&stack), 2);

   stack_init(&stack);
   stack_push(&stack, 2);
   stack_push(&stack, 9);
   expect_result("swap 2 9", stack_swap(&stack), RESULT_OK);
   expect_int("swap top", top_of(&stack), 2);
   stack_pop(&stack, &index);
   expect_int("swap second", index, 2);
   expect_int("swap bottom now top", top_of(&stack), 9);

   stack_init(&stack);
   stack_push(&stack, 1);
   stack_push(&stack, 2);
   stack_push(&stack, 3);
   stack_push(&stack, 4);
   expect_int("copy count", stack_copy_top(&stack, 3, copied), 3);
   expect_int("P first", copied[0], 2);
   expect_int("P second", copied[1], 3);
   expect_int("P top", copied[2], 4);
   expect_int("P keeps depth", stack_size(&stack), 4);

   stack_clear(&stack);
   expect_int("cleared", stack_size(&stack), 0);
   expect_int("empty flag", stack_is_empty(&stack) ? 1 : 0, 1);
}

static void test_errors_keep_stack(void) {
   Stack stack;
   int value;
   int index;

   stack_init(&stack);
   expect_result("add empty", arithmetic_add(&stack), ERROR_STACK_UNDERFLOW);

   stack_push(&stack, 1);
   expect_result("add one", arithmetic_add(&stack), ERROR_STACK_UNDERFLOW);
   expect_int("kept single", top_of(&stack), 1);
   expect_int("kept depth", stack_size(&stack), 1);

   expect_result("swap one", stack_swap(&stack), ERROR_STACK_UNDERFLOW);
   expect_int("swap kept", top_of(&stack), 1);

   expect_result("dup empty prep", stack_pop(&stack, &value), RESULT_OK);
   expect_result("dup empty", stack_duplicate(&stack), ERROR_STACK_UNDERFLOW);

   stack_push(&stack, 5);
   stack_push(&stack, 0);
   expect_result("div0", arithmetic_div(&stack), ERROR_DIV_ZERO);
   expect_int("div0 depth", stack_size(&stack), 2);
   expect_int("div0 top", top_of(&stack), 0);

   stack_init(&stack);
   stack_push(&stack, INT_MAX);
   stack_push(&stack, 1);
   expect_result("add overflow", arithmetic_add(&stack),
                 ERROR_ARITHMETIC_OVERFLOW);
   expect_int("add overflow top", top_of(&stack), 1);
   expect_int("add overflow depth", stack_size(&stack), 2);

   stack_init(&stack);
   stack_push(&stack, INT_MIN);
   stack_push(&stack, 1);
   expect_result("sub overflow", arithmetic_sub(&stack),
                 ERROR_ARITHMETIC_OVERFLOW);
   expect_int("sub overflow depth", stack_size(&stack), 2);

   stack_init(&stack);
   stack_push(&stack, INT_MAX);
   stack_push(&stack, -1);
   expect_result("sub max-(-1)", arithmetic_sub(&stack),
                 ERROR_ARITHMETIC_OVERFLOW);

   stack_init(&stack);
   stack_push(&stack, 0);
   stack_push(&stack, INT_MIN);
   expect_result("sub 0-INT_MIN", arithmetic_sub(&stack),
                 ERROR_ARITHMETIC_OVERFLOW);

   stack_init(&stack);
   stack_push(&stack, 100000);
   stack_push(&stack, 100000);
   expect_result("mul overflow", arithmetic_mul(&stack),
                 ERROR_ARITHMETIC_OVERFLOW);
   expect_int("mul overflow depth", stack_size(&stack), 2);

   stack_init(&stack);
   stack_push(&stack, INT_MIN);
   stack_push(&stack, -1);
   expect_result("mul INT_MIN*-1", arithmetic_mul(&stack),
                 ERROR_ARITHMETIC_OVERFLOW);

   stack_init(&stack);
   stack_push(&stack, INT_MIN);
   stack_push(&stack, 2);
   expect_result("mul INT_MIN*2", arithmetic_mul(&stack),
                 ERROR_ARITHMETIC_OVERFLOW);

   stack_init(&stack);
   stack_push(&stack, INT_MAX);
   stack_push(&stack, 2);
   expect_result("mul INT_MAX*2", arithmetic_mul(&stack),
                 ERROR_ARITHMETIC_OVERFLOW);

   stack_init(&stack);
   stack_push(&stack, 0);
   stack_push(&stack, INT_MIN);
   expect_result("mul 0*INT_MIN", arithmetic_mul(&stack), RESULT_OK);
   expect_int("mul zero", top_of(&stack), 0);

   stack_init(&stack);
   stack_push(&stack, -46340);
   stack_push(&stack, -46340);
   expect_result("mul neg boundary", arithmetic_mul(&stack), RESULT_OK);
   expect_int("(-46340)^2", top_of(&stack), 2147395600);

   stack_init(&stack);
   stack_push(&stack, -46341);
   stack_push(&stack, -46341);
   expect_result("mul neg overflow", arithmetic_mul(&stack),
                 ERROR_ARITHMETIC_OVERFLOW);

   stack_init(&stack);
   stack_push(&stack, INT_MIN);
   stack_push(&stack, -1);
   expect_result("div INT_MIN/-1", arithmetic_div(&stack),
                 ERROR_ARITHMETIC_OVERFLOW);
   expect_int("div overflow depth", stack_size(&stack), 2);
   expect_int("div overflow top", top_of(&stack), -1);

   stack_init(&stack);
   stack_push(&stack, INT_MAX);
   stack_push(&stack, -1);
   expect_result("div INT_MAX/-1", arithmetic_div(&stack), RESULT_OK);
   expect_int("INT_MAX/-1", top_of(&stack), -INT_MAX);

   stack_init(&stack);
   stack_push(&stack, INT_MIN);
   stack_push(&stack, 1);
   expect_result("div INT_MIN/1", arithmetic_div(&stack), RESULT_OK);
   expect_int("INT_MIN/1", top_of(&stack), INT_MIN);

   stack_init(&stack);
   for (index = 0; index < STACK_CAPACITY; index++) {
      expect_result("fill", stack_push(&stack, index), RESULT_OK);
   }
   expect_result("push full", stack_push(&stack, 1), ERROR_STACK_OVERFLOW);
   expect_int("full depth", stack_size(&stack), STACK_CAPACITY);
   expect_result("dup full", stack_duplicate(&stack), ERROR_STACK_OVERFLOW);
   expect_int("dup full depth", stack_size(&stack), STACK_CAPACITY);
}

int main(void) {
   int dummy = 0;
   test_format();
   test_example_from_assignment();
   test_operations();
   test_errors_keep_stack();

   {
      Stack stack;
      stack_init(&stack);
      expect_int("copy empty", stack_copy_top(&stack, 3, &dummy), 0);
   }

   if (g_failed == 0) {
      printf("Alle Tests bestanden.\n");
      return 0;
   }
   printf("%d Test(s) fehlgeschlagen.\n", g_failed);
   return 1;
}
