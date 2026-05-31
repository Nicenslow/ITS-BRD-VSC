/**
 ******************************************************************************
 * @file    timer_util.c
 * @brief   Zeitstempel-Hilfen auf Basis von timer.h (TIM2).
 ******************************************************************************
 */

#include "timer_util.h"
#include "timer.h"

/** @brief Umrechnung Ticks pro Sekunde aus timer.h */
#define TICKS_PER_SECOND ((double)TICKS_PER_US * 1000000.0)

/**
 * @brief  Modul-Initialisierung (TIM2 wird in main per initTimer gestartet).
 * @param  None
 * @retval None
 */
void timerUtil_init(void) {
}

/**
 * @brief  Liest den aktuellen Zeitstempel.
 * @param  None
 * @retval TIM2->CNT (Ticks)
 */
uint32_t timerUtil_getTimestamp(void) {
    return getTimeStamp();
}

/**
 * @brief  Zeitdifferenz zweier Zeitstempel in Sekunden (Wraparound-sicher).
 * @param  t_start erster Zeitstempel
 * @param  t_end   zweiter Zeitstempel
 * @retval Dauer in Sekunden als double
 */
double timerUtil_elapsedSeconds(uint32_t t_start, uint32_t t_end) {
    uint32_t elapsedTicks = (uint32_t)(t_end - t_start);
    return (double)elapsedTicks / TICKS_PER_SECOND;
}
