/**
 * @file timebase.h
 * @author Daniel Biedermann, HAW Hamburg
 * @date November 2026
 * @brief Monotone Zeit aus dem 32-Bit-Timer der Board-Bibliothek.
 */

#ifndef TIMEBASE_H
#define TIMEBASE_H

#include <stdint.h>

/* TIM2 zaehlt 90-mal je Mikrosekunde. */
#define TICKS_PER_MICROSECOND 90u
#define TICKS_PER_SECOND ((uint64_t)TICKS_PER_MICROSECOND * 1000000ull)

/* Anzeige spaetestens alle 300 ms, sonst nach 3 s ohne Phasenwechsel. */
#define DISPLAY_INTERVAL_TICKS (TICKS_PER_SECOND * 3ull / 10ull)
#define DISPLAY_TIMEOUT_TICKS (TICKS_PER_SECOND * 3ull)

void timebase_init(void);

/* Muss in jedem Super-Loop-Durchlauf aufgerufen werden. */
uint64_t timebase_now(void);

double timebase_seconds_between(uint64_t start_tick, uint64_t end_tick);

#endif /* TIMEBASE_H */
