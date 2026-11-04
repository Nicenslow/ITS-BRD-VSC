/**
 * @file host_timebase.c
 * @brief Zeitumrechnung fuer den PC-Test, ohne TIM2.
 */

#include "timebase.h"

double timebase_seconds_between(uint64_t start_tick, uint64_t end_tick) {
   if (end_tick <= start_tick) {
      return 0.0;
   }
   return (double)(end_tick - start_tick) / (double)TICKS_PER_SECOND;
}
