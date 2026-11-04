/**
 * @file angle.h
 * @author Daniel Biedermann, HAW Hamburg
 * @date November 2026
 * @brief Winkel und Winkelgeschwindigkeit aus dem Phasenzaehler.
 */

#ifndef ANGLE_H
#define ANGLE_H

#include <stdint.h>

/* 300 Schlitze, vier Phasen je Schlitz. */
#define PHASES_PER_REVOLUTION 1200

double angle_degrees(int32_t phase_count);

/*
 * Geschwindigkeit aus zwei Zaehlerstaenden und den Zeitstempeln der
 * Phasenwechsel, die diese Staende erzeugt haben.
 * Ohne fortgeschrittene Zeit oder ohne neuen Phasenwechsel: 0.
 */
double angle_velocity_degrees_per_second(int32_t count_start,
                                         uint64_t timestamp_start,
                                         int32_t count_end,
                                         uint64_t timestamp_end);

#endif /* ANGLE_H */
