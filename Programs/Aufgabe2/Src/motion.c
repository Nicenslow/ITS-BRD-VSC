/**
 * @file motion.c
 * @author Daniel Biedermann, HAW Hamburg
 * @date November 2026
 * @brief Quadraturfolge a-b-c-d als Zustandsautomat.
 */

#include "motion.h"

#define PHASE_COUNT 4U

static Phase last_phase = PHASE_A;
static int32_t phase_count = 0;
static uint64_t change_timestamp = 0ULL;
static Direction direction = DIRECTION_UNKNOWN;
static bool has_error = false;
static bool is_synced = false;

void motion_init(void) {
   last_phase = PHASE_A;
   phase_count = 0;
   change_timestamp = 0ULL;
   direction = DIRECTION_UNKNOWN;
   has_error = false;
   is_synced = false;
}

void motion_reset(PhaseSample sample) {
   last_phase = sample.phase;
   phase_count = 0;
   change_timestamp = sample.timestamp;
   direction = DIRECTION_UNKNOWN;
   has_error = false;
   is_synced = true;
}

void motion_update(PhaseSample sample) {
   uint32_t delta;

   if (has_error) {
      return;
   }

   if (!is_synced) {
      last_phase = sample.phase;
      change_timestamp = sample.timestamp;
      is_synced = true;
      return;
   }

   delta = ((uint32_t)sample.phase + PHASE_COUNT - (uint32_t)last_phase) & 3U;
   if (delta == 0U) {
      return;
   }

   if (delta == 1U) {
      phase_count += 1;
      change_timestamp = sample.timestamp;
      direction = DIRECTION_FORWARD;
   } else if (delta == 3U) {
      phase_count -= 1;
      change_timestamp = sample.timestamp;
      direction = DIRECTION_BACKWARD;
   } else {
      has_error = true;
      return;
   }

   last_phase = sample.phase;
}

bool motion_has_error(void) { return has_error; }

int32_t motion_count(void) { return phase_count; }

uint64_t motion_change_timestamp(void) { return change_timestamp; }

Direction motion_direction(void) { return direction; }
