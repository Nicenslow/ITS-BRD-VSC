/**
 ******************************************************************************
 * @file    main.c
 * @brief   Aufgabe 5: Drehgeber per EXTI-ISR, LCD-Anzeige (DDC-Superloop).
 ******************************************************************************
 */

#include <stdbool.h>
#include <stdint.h>

#include "init.h"
#include "timer.h"
#include "display.h"
#include "encoder.h"
#include "encoder_irq.h"
#include "fsm.h"
#include "gpio_in.h"
#include "gpio_out.h"
#include "timer_util.h"

/** @brief Fruehestes Ende des Zeitfensters bei Phasenwechsel (Aufgabenblatt: 250 ms) */
#define TIME_WINDOW_SHORT_SEC (0.25)
/** @brief Spaetestes Ende des Zeitfensters (Aufgabenblatt: 500 ms) */
#define TIME_WINDOW_LONG_SEC (0.5)

/**
 * @brief  Einstieg: Board, Timer, LCD, Module; Superloop mit Input–Verarbeitung–Output.
 * @param  None
 * @retval wird nicht erreicht
 */
int main(void) {
    initITSboard();
    initTimer();
    initLCDTouch();

    timerUtil_init();
    gpioOut_init();
    fsm_init();
    gpioIn_init();
    encoderIrq_init();
    encoder_init();
    display_init();

    uint32_t t_window_start   = timerUtil_getTimestamp();
    int32_t  pulseCount_start = 0;
    bool     measurementPinState = false;
    bool     processHalted    = false;
    bool     resetBtnPrev     = false;

    while (1) {
        measurementPinState = !measurementPinState;
        gpioOut_setMeasurementPin(measurementPinState);

        uint32_t now      = timerUtil_getTimestamp();
        bool     resetBtn = gpioIn_readResetButton();

        /* S6 (Loslassen): Fehler loeschen (Aufgabenblatt) + Vorgang anhalten/fortsetzen (Prof-Feedback) */
        if (resetBtnPrev && !resetBtn) {
            fsm_resetError(gpioIn_readPhase());
            processHalted = !processHalted;
            if (!processHalted) {
                int32_t snapshotPulse = 0;
                uint32_t snapshotTs   = 0;
                (void)fsm_getSnapshot(&snapshotPulse, &snapshotTs);
                t_window_start   = now;
                pulseCount_start = snapshotPulse;
            }
        }
        resetBtnPrev = resetBtn;

        if (processHalted) {
            gpioOut_setPulseCountLEDs(fsm_getPulseCount());
            gpioOut_setDirectionLEDs(fsm_getDirection());
            gpioOut_setErrorLED(fsm_hasError());
            continue;
        }

        int32_t pulseBefore = fsm_getPulseCount();
        double  elapsed     = timerUtil_elapsedSeconds(t_window_start, now);
        bool    phaseChanged = false;

        if (elapsed >= TIME_WINDOW_SHORT_SEC) {
            int32_t pulseAfter = fsm_getPulseCount();
            phaseChanged = (pulseAfter != pulseBefore);
        }

        if ((elapsed >= TIME_WINDOW_SHORT_SEC && phaseChanged) || elapsed >= TIME_WINDOW_LONG_SEC) {
            int32_t  pulseNow    = 0;
            uint32_t snapshotTs  = 0;
            (void)fsm_getSnapshot(&pulseNow, &snapshotTs);

            int32_t pulseDelta = pulseNow - pulseCount_start;
            double  angle      = encoder_getAngleDeg(pulseNow);
            double  velocity   = encoder_getAngularVelocityDegPerSec(
                                     pulseDelta, t_window_start, now);

            display_update(angle, velocity);

            t_window_start   = now;
            pulseCount_start = pulseNow;
        }

        gpioOut_setPulseCountLEDs(fsm_getPulseCount());
        gpioOut_setDirectionLEDs(fsm_getDirection());
        gpioOut_setErrorLED(fsm_hasError());
    }
}
