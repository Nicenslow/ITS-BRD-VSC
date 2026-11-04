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

/* Ein Zaehlerstand mit dem Zeitpunkt des Phasenwechsels, der ihn erzeugt hat.
 */
typedef struct {
   int32_t count;
   uint64_t timestamp;
} CountSample;

double angle_degrees(int32_t phase_count);

/*
 * Geschwindigkeit aus zwei Zaehlerstaenden.
 * Ohne fortgeschrittene Zeit oder ohne neuen Phasenwechsel: 0.
 */
double angle_velocity_degrees_per_second(CountSample start, CountSample end);

#endif /* ANGLE_H */
