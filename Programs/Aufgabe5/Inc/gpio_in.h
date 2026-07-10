/**
 ******************************************************************************
 * @file    gpio_in.h
 * @brief   Eingaenge: Drehgeber AUX0/AUX1 (PG0/PG1), Taster S6.
 ******************************************************************************
 */

#ifndef AUFGABE5_GPIO_IN_H
#define AUFGABE5_GPIO_IN_H

#include <stdbool.h>
#include "fsm.h"

void       gpioIn_init(void);
FsmState_t gpioIn_readPhase(void);
bool       gpioIn_readResetButton(void);

#endif /* AUFGABE5_GPIO_IN_H */
