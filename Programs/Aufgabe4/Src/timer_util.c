/**
 ******************************************************************************
 * @file    timer_util.c
 * @brief   Wartezeiten fuer 1-Wire und DS18B20 (Erweiterung aus Aufgabe 2).
 *
 * === Zwei Timing-Quellen ===
 *   Mikrosekunden: DWT-Busy-Wait (180 MHz) – exakt fuer 1-Wire-Slots
 *   Millisekunden: timerUtil_sleepMs() – z.B. 750 ms Convert T, 1 s Startup
 *
 * timerUtil_sleepUs() nutzt primaer TIM2 (getTimeStamp), faellt bei Timeout
 * auf DWT-Busy-Wait zurueck.
 ******************************************************************************
 */

#include "timer_util.h"
#include "timer.h"

#include "stm32f429xx.h"

#define CPU_CYCLES_PER_US 180U

/** DWT-Zykluszaehler – unabhaengig von TIM2, fuer kurze exakte Delays */
static void timerUtil_delayUsBusy(uint32_t us) {
    uint32_t start  = DWT->CYCCNT;
    uint32_t cycles = us * CPU_CYCLES_PER_US;

    while ((uint32_t)(DWT->CYCCNT - start) < cycles) {
    }
}

/** DWT-Zykluszaehler einmalig aktivieren (wird auch in 1wire.c genutzt) */
void timerUtil_init(void) {
    CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;
    DWT->CYCCNT = 0U;
    DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;
}

uint32_t timerUtil_getTimestamp(void) {
    return getTimeStamp();
}

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
 * @brief  Blockierende Wartezeit in Millisekunden.
 * Wird fuer parasitaere Sensor-Aufladung (1000 ms) und Convert T (750 ms) genutzt.
 */
void timerUtil_sleepMs(uint32_t ms) {
    while (ms > 0U) {
        uint32_t chunk = (ms > 10U) ? 10U : ms;
        timerUtil_delayUsBusy(chunk * 1000U);
        ms -= chunk;
    }
}
