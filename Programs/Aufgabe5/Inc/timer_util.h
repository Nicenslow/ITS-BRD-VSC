/**
 ******************************************************************************
 * @file    timer_util.h
 * @brief   Zeitstempel und Zeitspannen in Sekunden (timer.h / TIM2).
 ******************************************************************************
 */

#ifndef AUFGABE5_TIMER_UTIL_H
#define AUFGABE5_TIMER_UTIL_H

#include <stdint.h>

void     timerUtil_init(void);
uint32_t timerUtil_getTimestamp(void);
double   timerUtil_elapsedSeconds(uint32_t t_start, uint32_t t_end);

#endif /* AUFGABE5_TIMER_UTIL_H */
