/**
 * @file gpio_in.c
 * @author Daniel Biedermann, HAW Hamburg
 * @date November 2026
 * @brief Kanal A auf PF0, Kanal B auf PF1, Taste S6 auf PF6.
 */

#include "gpio_in.h"

#include "stm32f429xx.h"
#include "timebase.h"

#define CHANNEL_A_PIN 0U
#define CHANNEL_B_PIN 1U
#define RESET_BUTTON_PIN 6U

void gpio_in_init(void) {}

PhaseSample gpio_in_read_phase(void) {
   uint32_t input_data = GPIOF->IDR;
   unsigned int level_a = (input_data >> CHANNEL_A_PIN) & 1U;
   unsigned int level_b = (input_data >> CHANNEL_B_PIN) & 1U;
   PhaseSample sample;

   sample.timestamp = timebase_now();

   if ((level_a == 0U) && (level_b == 0U)) {
      sample.phase = PHASE_A;
   } else if ((level_a == 1U) && (level_b == 0U)) {
      sample.phase = PHASE_B;
   } else if ((level_a == 1U) && (level_b == 1U)) {
      sample.phase = PHASE_C;
   } else {
      sample.phase = PHASE_D;
   }

   return sample;
}

bool gpio_in_read_reset_button(void) {
   uint32_t input_data = GPIOF->IDR;
   return ((input_data >> RESET_BUTTON_PIN) & 1U) == 0U;
}
