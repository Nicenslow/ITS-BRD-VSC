/**
 * @file gpio_out.h
 * @author Daniel Biedermann, HAW Hamburg
 * @date November 2026
 * @brief LEDs und Messpin fuer das Oszilloskop.
 */

#ifndef GPIO_OUT_H
#define GPIO_OUT_H

#include "phase.h"

#include <stdbool.h>
#include <stdint.h>

void gpio_out_init(void);
void gpio_out_show_count(int32_t phase_count);
void gpio_out_show_direction(Direction direction);
void gpio_out_show_error(bool is_on);
void gpio_out_set_measure_pin(bool is_high);

#endif /* GPIO_OUT_H */
