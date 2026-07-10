/**
 ******************************************************************************
 * @file    encoder.c
 * @brief   Drehwinkel und Winkelgeschwindigkeit aus Phasenimpulsen.
 *          Scheibe: 300 CPR -> 1200 Phasenwechsel pro Umdrehung (laut Aufgabenstellung).
 ******************************************************************************
 */

#include "encoder.h"
#include "timer_util.h"

#define PHASES_PER_REV    1200
#define DEGREES_PER_PHASE (360.0 / (double)PHASES_PER_REV)

/**
 * @brief  Initialisiert Encoder-Parameter (Platzhalter fuer spaetere Erweiterungen).
 * @param  None
 * @retval None
 */
void encoder_init(void) {
}

/**
 * @brief  Berechnet den Drehwinkel aus dem Impulszaehler.
 * @param  pulseCount Akkumulierter Phasenimpulszaehler
 * @retval Winkel in Grad (double)
 */
double encoder_getAngleDeg(int32_t pulseCount) {
    return (double)pulseCount * DEGREES_PER_PHASE;
}

/**
 * @brief  Berechnet die Winkelgeschwindigkeit aus Impulsdifferenz und Zeitfenster.
 * @param  pulseDelta Differenz der Impulse im Zeitfenster
 * @param  t_start    Startzeitstempel (TIM2)
 * @param  t_end      Endzeitstempel (TIM2)
 * @retval Grad pro Sekunde; 0 bei nicht positiver Zeitspanne
 */
double encoder_getAngularVelocityDegPerSec(int32_t pulseDelta,
                                           uint32_t t_start,
                                           uint32_t t_end) {
    double elapsedSec = timerUtil_elapsedSeconds(t_start, t_end);
    if (elapsedSec <= 0.0) {
        return 0.0;
    }
    return ((double)pulseDelta * DEGREES_PER_PHASE) / elapsedSec;
}
