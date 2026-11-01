/**
 * @file arithmetic.h
 * @author Daniel Biedermann, HAW Hamburg
 * @date Oktober 2026
 * @brief Ganzzahlige Rechenoperationen auf dem Stack.
 */

#ifndef ARITHMETIC_H
#define ARITHMETIC_H

#include "stack.h"

/*
 ****************************************************************************************
 * @brief Die beiden obersten Werte addieren und die Summe ablegen.
 *
 * Der oberste Wert ist der rechte Operand. Bei einem Fehler bleibt der Stack
 * unveraendert.
 *
 * @param stack Stack des Rechners.
 *
 * @return RESULT_OK, Stackfehler oder ERROR_ARITHMETIC_OVERFLOW.
 ****************************************************************************************
 */
Result arithmetic_add(Stack *stack);

/*
 ****************************************************************************************
 * @brief Vom zweitobersten Wert den obersten subtrahieren.
 *
 * @param stack Stack des Rechners.
 *
 * @return RESULT_OK, Stackfehler oder ERROR_ARITHMETIC_OVERFLOW.
 ****************************************************************************************
 */
Result arithmetic_sub(Stack *stack);

/*
 ****************************************************************************************
 * @brief Die beiden obersten Werte multiplizieren.
 *
 * @param stack Stack des Rechners.
 *
 * @return RESULT_OK, Stackfehler oder ERROR_ARITHMETIC_OVERFLOW.
 ****************************************************************************************
 */
Result arithmetic_mul(Stack *stack);

/*
 ****************************************************************************************
 * @brief Den zweitobersten Wert ganzzahlig durch den obersten teilen.
 *
 * Die Division rundet gegen 0. Division durch 0 und INT_MIN / -1 sind Fehler.
 * Bei einem Fehler bleibt der Stack unveraendert.
 *
 * @param stack Stack des Rechners.
 *
 * @return RESULT_OK, Stackfehler, ERROR_DIV_ZERO oder
 *ERROR_ARITHMETIC_OVERFLOW.
 ****************************************************************************************
 */
Result arithmetic_div(Stack *stack);

#endif /* ARITHMETIC_H */
