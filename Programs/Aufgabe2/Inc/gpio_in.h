/**
 ******************************************************************************
 * @file    gpio_in.h
 * @brief   Eingaenge: Drehgeber Phasen (PF0/PF1), Taster S6 (Fehler loeschen).
 ******************************************************************************
 */

#ifndef AUFGABE2_GPIO_IN_H
#define AUFGABE2_GPIO_IN_H

#include <stdbool.h>
#include "fsm.h"

void       gpioIn_init(void);
FsmState_t gpioIn_readPhase(void);
bool       gpioIn_readResetButton(void);

#endif /* AUFGABE2_GPIO_IN_H */
