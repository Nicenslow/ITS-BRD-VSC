/**
 * @file gpio_in.h
 * @author Daniel Biedermann, HAW Hamburg
 * @date November 2026
 * @brief Einlesen von Drehgeber und Taste S6.
 */

#ifndef GPIO_IN_H
#define GPIO_IN_H

#include "phase.h"

#include <stdbool.h>

void gpio_in_init(void);

/* Liest Kanal A (PF0) und Kanal B (PF1) und den Zeitstempel direkt danach. */
PhaseSample gpio_in_read_phase(void);

/* S6 liegt auf PF6 und ist low-aktiv. */
bool gpio_in_read_reset_button(void);

#endif /* GPIO_IN_H */
