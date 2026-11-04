/**
 * @file angle.c
 * @author Daniel Biedermann, HAW Hamburg
 * @date November 2026
 * @brief Rechnet den Phasenzaehler in Grad und Grad je Sekunde um.
 */

#include "angle.h"

#include "timebase.h"

static const double DEGREES_PER_PHASE = 360.0 / (double)PHASES_PER_REVOLUTION;

double angle_degrees(int32_t phase_count) {
   return (double)phase_count * DEGREES_PER_PHASE;
}

double angle_velocity_degrees_per_second(CountSample start, CountSample end) {
   double elapsed_seconds;
   int32_t count_delta = end.count - start.count;

   if (count_delta == 0) {
      return 0.0;
   }

   elapsed_seconds = timebase_seconds_between(start.timestamp, end.timestamp);
   if (elapsed_seconds <= 0.0) {
      return 0.0;
   }

   return ((double)count_delta * DEGREES_PER_PHASE) / elapsed_seconds;
}
