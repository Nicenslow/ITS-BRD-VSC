/**
 ******************************************************************************
 * @file    main.c
 * @brief   Aufgabe 2: Drehgeber per Polling, LCD-Anzeige (DDC-Superloop).
 ******************************************************************************
 */

#include <stdbool.h>
#include <stdint.h>

#include "init.h"
#include "timer.h"
#include "display.h"
#include "encoder.h"
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
    gpioIn_init();
    gpioOut_init();
    fsm_init();
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

        uint32_t   now      = timerUtil_getTimestamp();
        FsmState_t phase    = gpioIn_readPhase();
        bool       resetBtn = gpioIn_readResetButton();

        /* S6 (Loslassen): Fehler loeschen (Aufgabenblatt) + Vorgang anhalten/fortsetzen (Prof-Feedback) */
        if (resetBtnPrev && !resetBtn) {
            fsm_resetError(phase);
            processHalted = !processHalted;
            if (!processHalted) {
                t_window_start   = now;
                pulseCount_start = fsm_getPulseCount();
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
        fsm_update(phase, now);
        bool phaseChanged = (fsm_getPulseCount() != pulseBefore);

        double elapsed = timerUtil_elapsedSeconds(t_window_start, now);

        if ((elapsed >= TIME_WINDOW_SHORT_SEC && phaseChanged) || elapsed >= TIME_WINDOW_LONG_SEC) {
            int32_t pulseDelta = fsm_getPulseCount() - pulseCount_start;
            double  angle      = encoder_getAngleDeg(fsm_getPulseCount());
            double  velocity   = encoder_getAngularVelocityDegPerSec(
                                     pulseDelta, t_window_start, now);

            display_update(angle, velocity);

            t_window_start   = now;
            pulseCount_start = fsm_getPulseCount();
        }

        gpioOut_setPulseCountLEDs(fsm_getPulseCount());
        gpioOut_setDirectionLEDs(fsm_getDirection());
        gpioOut_setErrorLED(fsm_hasError());
    }
}
