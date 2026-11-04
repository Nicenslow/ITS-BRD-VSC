/**
 * @file test_motion.c
 * @brief Prueft Phasenfolge, Fehler und Winkel ohne Board.
 *
 * gcc oder tcc, im Ordner Programs/Aufgabe2:
 * tcc -std=c99 -I Inc tests/test_motion.c tests/host_timebase.c Src/motion.c Src/angle.c -o test_motion
 */

#include "angle.h"
#include "motion.h"
#include "timebase.h"

#include <stdio.h>

static int failed = 0;

static void expect_int(const char *name, int got, int want) {
   if (got != want) {
      printf("FAIL %s: got %d, want %d\n", name, got, want);
      failed++;
   }
}

static void expect_near(const char *name, double got, double want) {
   double delta = got - want;
   if (delta < 0.0) {
      delta = -delta;
   }
   if (delta > 0.0001) {
      printf("FAIL %s: got %f, want %f\n", name, got, want);
      failed++;
   }
}

static PhaseSample sample_of(Phase phase, uint64_t timestamp) {
   PhaseSample sample;
   sample.phase = phase;
   sample.timestamp = timestamp;
   return sample;
}

static void test_forward_revolution_step(void) {
   motion_init();
   motion_update(sample_of(PHASE_A, 0));
   motion_update(sample_of(PHASE_B, 1000));
   motion_update(sample_of(PHASE_C, 2000));
   motion_update(sample_of(PHASE_D, 3000));
   motion_update(sample_of(PHASE_A, 4000));
   expect_int("forward count", motion_count(), 4);
   expect_int("forward direction", (int)motion_direction(), (int)DIRECTION_FORWARD);
   expect_int("forward error", motion_has_error() ? 1 : 0, 0);
   expect_near("four phases", angle_degrees(motion_count()), 1.2);
}

static void test_backward_from_b(void) {
   motion_init();
   motion_update(sample_of(PHASE_B, 0));
   motion_update(sample_of(PHASE_A, 1000));
   expect_int("back count", motion_count(), -1);
   expect_int("back direction", (int)motion_direction(), (int)DIRECTION_BACKWARD);
}

static void test_skipped_phase_is_error(void) {
   motion_init();
   motion_update(sample_of(PHASE_B, 0));
   motion_update(sample_of(PHASE_D, 1000));
   expect_int("skip error", motion_has_error() ? 1 : 0, 1);
   expect_int("frozen count", motion_count(), 0);
   motion_update(sample_of(PHASE_A, 2000));
   expect_int("still frozen", motion_count(), 0);
}

static void test_full_turn(void) {
   expect_near("full turn", angle_degrees(PHASES_PER_REVOLUTION), 360.0);
}

static CountSample count_at(int32_t count, uint64_t timestamp) {
   CountSample sample;
   sample.count = count;
   sample.timestamp = timestamp;
   return sample;
}

static void test_velocity(void) {
   double velocity = angle_velocity_degrees_per_second(
       count_at(0, 0), count_at(4, TICKS_PER_SECOND));
   expect_near("1.2 deg/s", velocity, 1.2);
   expect_near("no change",
               angle_velocity_degrees_per_second(count_at(3, 10), count_at(3, 99)),
               0.0);
}

int main(void) {
   test_forward_revolution_step();
   test_backward_from_b();
   test_skipped_phase_is_error();
   test_full_turn();
   test_velocity();
   if (failed == 0) {
      printf("Alle Tests bestanden.\n");
      return 0;
   }
   printf("%d Test(s) fehlgeschlagen.\n", failed);
   return 1;
}
