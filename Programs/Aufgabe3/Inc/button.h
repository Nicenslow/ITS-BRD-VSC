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
 * @brief  Wartet auf Druck und Loslassen des blauen User-Tasters.
 * @param  Keine
 * @retval Keiner (void)
 */
void button_waitForPress(void);

/**
 * @brief  Prueft den blauen User-Taster.
 * @retval true  Taster gedrueckt
 * @retval false Taster nicht gedrueckt
 */
bool button_isPressed(void);

#endif /* AUFGABE3_BUTTON_H */
