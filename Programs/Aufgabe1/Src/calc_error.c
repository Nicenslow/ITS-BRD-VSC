/**
 * @file calc_error.c
 * @author Daniel Biedermann, HAW Hamburg
 * @date Oktober 2026
 * @brief Fehlermeldungen auf dem oberen Terminal.
 */

#include "calc_error.h"

#include "display.h"

void calc_error_print(Result err) {
   setErrMode();
   switch (err) {
   case ERROR_STACK_OVERFLOW:
      printStdout("Error: stack overflow");
      break;
   case ERROR_STACK_UNDERFLOW:
      printStdout("Error: stack underflow");
      break;
   case ERROR_DIV_ZERO:
      printStdout("Error: division by zero");
      break;
   case ERROR_ARITHMETIC_OVERFLOW:
      printStdout("Error: arithmetic overflow");
      break;
   case ERROR_INPUT_OVERFLOW:
      printStdout("Error: input overflow");
      break;
   case RESULT_OK:
   default:
      break;
   }
}
