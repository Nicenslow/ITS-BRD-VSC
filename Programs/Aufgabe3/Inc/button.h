#ifndef AUFGABE3_BUTTON_H
#define AUFGABE3_BUTTON_H

#include <stdbool.h>

/**
 * @brief  Initialisiert den blauen Nucleo-User-Taster (PC13).
 * @param  Keine
 * @retval Keiner (void)
 */
void button_init(void);

/**
 * @brief  Wartet auf Taster S1..S6 (PG0..PG5) oder Nucleo-User-Taster.
 * @param  Keine
 * @retval Keiner (void)
 */
void button_waitForPress(void);

/**
 * @brief  Prueft den aktuellen Tasterzustand.
 * @retval true  Mindestens ein Taster gedrueckt
 * @retval false Kein Taster gedrueckt
 */
bool button_isPressed(void);

#endif /* AUFGABE3_BUTTON_H */
