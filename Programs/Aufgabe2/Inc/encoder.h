/**
 ******************************************************************************
 * @file    encoder.h
 * @brief   Winkel- und Winkelgeschwindigkeitsberechnung aus Phasenimpulsen.
 ******************************************************************************
 */

#ifndef AUFGABE2_ENCODER_H
#define AUFGABE2_ENCODER_H

#include <stdint.h>

void   encoder_init(void);
double encoder_getAngleDeg(int32_t pulseCount);
double encoder_getAngularVelocityDegPerSec(int32_t pulseDelta,
                                            uint32_t t_start,
                                            uint32_t t_end);

#endif /* AUFGABE2_ENCODER_H */
