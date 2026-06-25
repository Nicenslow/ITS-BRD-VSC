/**
 ******************************************************************************
 * @file    timer_util.h
 * @brief   Wartezeiten (us/ms) fuer 1-Wire und DS18B20-Konvertierung.
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
