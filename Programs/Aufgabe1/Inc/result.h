/**
 * @file result.h
 * @author Daniel Biedermann, HAW Hamburg
 * @date Oktober 2026
 * @brief Einheitliche Rueckgabecodes fuer den RPN-Rechner.
 */

#ifndef RESULT_H
#define RESULT_H

/*
 ****************************************************************************************
 * Ergebnis einer Rechneroperation.
 * Der Rueckgabewert einer Funktion ist nur dieser Status. Berechnete Zahlen
 * werden ueber einen Zeiger zurueckgegeben, damit negative Werte nicht mit
 * einem Fehler verwechselt werden.
 ****************************************************************************************
 */
typedef enum {
   RESULT_OK = 0,
   ERROR_STACK_OVERFLOW,
   ERROR_STACK_UNDERFLOW,
   ERROR_DIV_ZERO,
   ERROR_ARITHMETIC_OVERFLOW,
   ERROR_INPUT_OVERFLOW
} Result;

#endif /* RESULT_H */
