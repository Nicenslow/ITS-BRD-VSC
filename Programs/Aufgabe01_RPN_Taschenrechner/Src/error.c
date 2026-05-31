/**
 * @file error.c
 * @author Daniel Biedermann, HAW Hamburg
 * @date April 2026
 * @brief This module prints user-facing error messages to the display.
 */

#include "error.h"

#include "display.h"

void error_print(Result err) {
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
        case RESULT_OK:
        default:
            break;
    }
}
