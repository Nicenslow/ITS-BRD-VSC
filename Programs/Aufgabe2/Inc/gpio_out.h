/**
 ******************************************************************************
 * @file    gpio_out.h
 * @brief   LED-Ausgaenge: Binaerzaehler, Richtung, Fehler.
 ******************************************************************************
 */

#ifndef AUFGABE2_GPIO_OUT_H
#define AUFGABE2_GPIO_OUT_H

#include <stdbool.h>
#include <stdint.h>
#include "fsm.h"

void gpioOut_init(void);
void gpioOut_setPulseCountLEDs(int32_t pulseCount);
void gpioOut_setDirectionLEDs(Direction_t dir);
void gpioOut_setErrorLED(bool on);
void gpioOut_setMeasurementPin(bool on);

#endif /* AUFGABE2_GPIO_OUT_H */
