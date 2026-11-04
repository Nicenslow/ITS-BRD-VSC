/**
 * @file main.c
 * @author Daniel Biedermann, HAW Hamburg
 * @date November 2026
 * @brief Super-Loop nach dem DDC-Schema: erst einlesen, dann verarbeiten.
 */

#include "main.h"

#include "angle.h"
#include "display.h"
#include "gpio_in.h"
#include "gpio_out.h"
#include "init.h"
#include "motion.h"
#include "timebase.h"
#include "timer.h"

#include <stdbool.h>

static void reset_program(PhaseSample sample) {
   motion_reset(sample);
   gpio_out_show_count(0);
   gpio_out_show_direction(DIRECTION_UNKNOWN);
   gpio_out_show_error(false);
   display_submit(0.0, 0.0);
}

static void publish_measurement(int32_t count_start,
                                uint64_t timestamp_start,
                                bool count_changed) {
   int32_t count_end = motion_count();
   uint64_t timestamp_end = motion_change_timestamp();
   double velocity = 0.0;

   if (count_changed) {
      velocity = angle_velocity_degrees_per_second(count_start,
                                                   timestamp_start,
                                                   count_end,
                                                   timestamp_end);
   }

   display_submit(angle_degrees(count_end), velocity);
}

int main(void) {
   bool reset_was_down = false;
   int32_t published_count = 0;
   uint64_t published_timestamp = 0ull;
   uint64_t last_display_tick = 0ull;

   initITSboard();
   initTimer();
   initLCDTouch();
   timebase_init();
   gpio_in_init();
   gpio_out_init();
   motion_init();
   display_init();

   published_timestamp = timebase_now();
   last_display_tick = published_timestamp;

   while (1) {
      PhaseSample sample = gpio_in_read_phase();
      bool reset_is_down = gpio_in_read_reset_button();
      bool reset_pressed = reset_is_down && !reset_was_down;
      uint64_t now = sample.timestamp;
      bool count_changed;
      uint64_t since_display;

      reset_was_down = reset_is_down;

      if (reset_pressed) {
         reset_program(sample);
         published_count = 0;
         published_timestamp = sample.timestamp;
         last_display_tick = sample.timestamp;
         continue;
      }

      motion_update(sample);

      if (motion_has_error()) {
         gpio_out_show_error(true);
         continue;
      }

      count_changed = motion_count() != published_count;
      since_display = now - last_display_tick;

      if (!display_is_busy()) {
         if ((count_changed && (since_display >= DISPLAY_INTERVAL_TICKS)) ||
             (since_display >= DISPLAY_TIMEOUT_TICKS)) {
            publish_measurement(published_count, published_timestamp, count_changed);
            published_count = motion_count();
            published_timestamp = motion_change_timestamp();
            last_display_tick = now;
         }
      }

      display_pump();
      gpio_out_show_count(motion_count());
      gpio_out_show_direction(motion_direction());
      gpio_out_show_error(false);
   }
}
