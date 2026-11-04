/**
 * @file phase.h
 * @author Daniel Biedermann, HAW Hamburg
 * @date November 2026
 * @brief Phase des Drehgebers und die dazugehoerige Messung.
 */

#ifndef PHASE_H
#define PHASE_H

#include <stdint.h>

/* Kanal A und Kanal B laut Zuordnungstabelle der Aufgabe. */
typedef enum {
   PHASE_A = 0, /* A=0, B=0 */
   PHASE_B = 1, /* A=1, B=0 */
   PHASE_C = 2, /* A=1, B=1 */
   PHASE_D = 3, /* A=0, B=1 */
} Phase;

/* Eine Phase zusammen mit dem Zeitpunkt des Einlesens. */
typedef struct {
   Phase phase;
   uint64_t timestamp;
} PhaseSample;

typedef enum {
   DIRECTION_UNKNOWN = 0,
   DIRECTION_FORWARD,
   DIRECTION_BACKWARD,
} Direction;

#endif /* PHASE_H */
