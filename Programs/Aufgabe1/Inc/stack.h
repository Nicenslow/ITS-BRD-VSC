/**
 * @file stack.h
 * @author Daniel Biedermann, HAW Hamburg
 * @date Oktober 2026
 * @brief Stack des RPN-Rechners.
 */

#ifndef STACK_H
#define STACK_H

#include "result.h"

#include <stdbool.h>

/* Maximale Anzahl von Werten auf dem Rechnerstack. */
#define STACK_CAPACITY 32

/* Array-Stack. top ist der Index des naechsten freien Platzes. */
typedef struct {
   int data[STACK_CAPACITY];
   int top;
} Stack;

/*
 ****************************************************************************************
 * @brief Stack leeren und als leer markieren.
 *
 * @param stack Zu initialisierender Stack.
 ****************************************************************************************
 */
void stack_init(Stack *stack);

/*
 ****************************************************************************************
 * @brief Alle Eintraege entfernen.
 *
 * @param stack Zu loeschender Stack.
 ****************************************************************************************
 */
void stack_clear(Stack *stack);

/*
 ****************************************************************************************
 * @brief Pruefen, ob der Stack leer ist.
 *
 * @param stack Zu pruefender Stack.
 *
 * @return true, wenn kein Eintrag vorhanden ist.
 ****************************************************************************************
 */
bool stack_is_empty(const Stack *stack);

/*
 ****************************************************************************************
 * @brief Anzahl der gespeicherten Werte.
 *
 * @param stack Zu pruefender Stack.
 *
 * @return Anzahl der Eintraege, 0 wenn der Stack leer ist.
 ****************************************************************************************
 */
int stack_size(const Stack *stack);

/*
 ****************************************************************************************
 * @brief Wert oben auf den Stack legen.
 *
 * @param stack Zielstack.
 * @param value Zu speichernder Wert.
 *
 * @return RESULT_OK oder ERROR_STACK_OVERFLOW.
 ****************************************************************************************
 */
Result stack_push(Stack *stack, int value);

/*
 ****************************************************************************************
 * @brief Obersten Wert entnehmen.
 *
 * @param stack Quellstack.
 * @param out_value Entnommener Wert.
 *
 * @return RESULT_OK oder ERROR_STACK_UNDERFLOW.
 ****************************************************************************************
 */
Result stack_pop(Stack *stack, int *out_value);

/*
 ****************************************************************************************
 * @brief Obersten Wert lesen, ohne ihn zu entfernen.
 *
 * @param stack Quellstack.
 * @param out_value Gelesener Wert.
 *
 * @return RESULT_OK oder ERROR_STACK_UNDERFLOW.
 ****************************************************************************************
 */
Result stack_peek(const Stack *stack, int *out_value);

/*
 ****************************************************************************************
 * @brief Die obersten Werte kopieren, ohne den Stack zu veraendern.
 *
 * Es werden hoechstens max_count Werte geliefert. Sind weniger vorhanden,
 * werden alle geliefert. out_oldest_first[0] ist der aelteste dieser Werte,
 * der letzte Eintrag ist der oberste Stackwert.
 *
 * @param stack Quellstack.
 * @param max_count Groesse des Zielpuffers.
 * @param out_oldest_first Zielpuffer.
 *
 * @return Anzahl kopierter Werte, 0 wenn der Stack leer ist oder max_count <=
 *0.
 ****************************************************************************************
 */
int stack_copy_top(const Stack *stack, int max_count, int *out_oldest_first);

/*
 ****************************************************************************************
 * @brief Obersten Eintrag duplizieren (Befehl d).
 *
 * @param stack Zu veraendernder Stack.
 *
 * @return RESULT_OK, ERROR_STACK_UNDERFLOW oder ERROR_STACK_OVERFLOW.
 ****************************************************************************************
 */
Result stack_duplicate(Stack *stack);

/*
 ****************************************************************************************
 * @brief Die beiden obersten Eintraege vertauschen (Befehl r).
 *
 * Bei weniger als zwei Eintraegen bleibt der Stack unveraendert.
 *
 * @param stack Zu veraendernder Stack.
 *
 * @return RESULT_OK oder ERROR_STACK_UNDERFLOW.
 ****************************************************************************************
 */
Result stack_swap(Stack *stack);

#endif /* STACK_H */
