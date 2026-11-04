/**
 * @file timebase.c
 * @author Daniel Biedermann, HAW Hamburg
 * @date November 2026
 * @brief 32-Bit-Ueberlaeufe des Timerregisters in einen 64-Bit-Zaehler uebernehmen.
 */

#include "timebase.h"

#include "timer.h"

static uint32_t last_raw_count = 0u;
static uint64_t accumulated_ticks = 0ull;

void timebase_init(void) {
   last_raw_count = getTimeStamp();
   accumulated_ticks = 0ull;
}

uint64_t timebase_now(void) {
   uint32_t raw_count = getTimeStamp();
   uint32_t elapsed = raw_count - last_raw_count;
   last_raw_count = raw_count;
   accumulated_ticks += elapsed;
   return accumulated_ticks;
}

double timebase_seconds_between(uint64_t start_tick, uint64_t end_tick) {
   if (end_tick <= start_tick) {
      return 0.0;
   }
   return (double)(end_tick - start_tick) / (double)TICKS_PER_SECOND;
}
