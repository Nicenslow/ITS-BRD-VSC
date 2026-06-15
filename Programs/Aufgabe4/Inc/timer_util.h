/**
 ******************************************************************************
 * @file    timer_util.h
 * @brief   Zeitstempel und Wartezeiten auf Basis von timer.h (TIM2).
 ******************************************************************************
 */

#ifndef AUFGABE4_TIMER_UTIL_H
#define AUFGABE4_TIMER_UTIL_H

#include <stdint.h>

void     timerUtil_init(void);
uint32_t timerUtil_getTimestamp(void);
void     timerUtil_sleepUs(uint32_t us);
void     timerUtil_sleepMs(uint32_t ms);

#endif /* AUFGABE4_TIMER_UTIL_H */
