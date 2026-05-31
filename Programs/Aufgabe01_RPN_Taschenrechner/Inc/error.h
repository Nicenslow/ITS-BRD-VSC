/**
 * @file error.h
 * @author Daniel Biedermann, HAW Hamburg
 * @date April 2026
 * @brief Header file for calculator error handling/output.
 */

#ifndef ERROR_H
#define ERROR_H

#include "stack.h"

/*
 ****************************************************************************************
 * @brief Print a user-facing message for the given result code.
 *
 * @param err Error/result code.
 *
 * @return void
 ****************************************************************************************/
void error_print(Result err);

#endif /* ERROR_H */
