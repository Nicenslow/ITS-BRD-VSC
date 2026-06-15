/**
 ******************************************************************************
 * @file    timer_util.c
 * @brief   Wartezeiten und Zeitstempel ueber TIM2 (Aufgabe-2-Timer-Modul).
 ******************************************************************************
 */

#include "timer_util.h"
#include "timer.h"

#include "stm32f429xx.h"

/** @brief CPU-Takte pro Mikrosekunde bei 180 MHz */
#define CPU_CYCLES_PER_US 180U

static void timerUtil_delayUsBusy(uint32_t us) {
    uint32_t start  = DWT->CYCCNT;
    uint32_t cycles = us * CPU_CYCLES_PER_US;

    while ((uint32_t)(DWT->CYCCNT - start) < cycles) {
    }
}

/**
 * @brief  Modul-Initialisierung (TIM2 + DWT-Fallback fuer Wartezeiten).
 */
void timerUtil_init(void) {
    CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;
    DWT->CYCCNT = 0U;
    DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;
}

/**
 * @brief  Liest den aktuellen Zeitstempel in Timer-Ticks.
 */
uint32_t timerUtil_getTimestamp(void) {
    return getTimeStamp();
}

/**
 * @brief  Wartet die angegebene Zeit in Mikrosekunden (timer-basiert).
 */
void timerUtil_sleepUs(uint32_t us) {
    if (us == 0U) {
        return;
    }

    uint32_t start      = getTimeStamp();
    uint32_t ticks      = us * (uint32_t)TICKS_PER_US;
    uint32_t spins      = 0U;
    const uint32_t limit = ticks + (ticks / 4U) + 50000U;

    while ((uint32_t)(getTimeStamp() - start) < ticks) {
        if (++spins > limit) {
            timerUtil_delayUsBusy(us);
            return;
        }
    }
}

/**
 * @brief  Wartet die angegebene Zeit in Millisekunden (timer-basiert).
 */
void timerUtil_sleepMs(uint32_t ms) {
    while (ms > 0U) {
        uint32_t chunk = (ms > 10U) ? 10U : ms;
        timerUtil_delayUsBusy(chunk * 1000U);
        ms -= chunk;
    }
}
