/**
 * @file motion.h
 * @author Daniel Biedermann, HAW Hamburg
 * @date November 2026
 * @brief Phasenwechsel, Phasenzaehler und Fehler des Drehgebers.
 */

#ifndef MOTION_H
#define MOTION_H

#include "phase.h"

#include <stdbool.h>
#include <stdint.h>

void motion_init(void);
void motion_reset(PhaseSample sample);
void motion_update(PhaseSample sample);

bool motion_has_error(void);
int32_t motion_count(void);
uint64_t motion_change_timestamp(void);
Direction motion_direction(void);

#endif /* MOTION_H */
