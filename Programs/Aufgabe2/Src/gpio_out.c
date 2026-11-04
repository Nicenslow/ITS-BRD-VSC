/**
 * @file gpio_out.c
 * @author Daniel Biedermann, HAW Hamburg
 * @date November 2026
 * @brief D8-D15 auf PD0-PD7, D20-D23 auf PE4-PE7.
 */

#include "gpio_out.h"

#include "stm32f429xx.h"

#define COUNT_LED_MASK 0xFFU
#define MEASURE_PIN 4U
#define ERROR_LED_PIN 5U
#define BACKWARD_LED_PIN 6U
#define FORWARD_LED_PIN 7U
/* Bits 16..31 im BSRR setzen den Pin mit derselben Nummer auf 0. */
#define BSRR_RESET_SHIFT 16U

static void write_pin(uint32_t pin, bool is_high) {
   if (is_high) {
      GPIOE->BSRR = (1U << pin);
   } else {
      GPIOE->BSRR = (1U << (pin + BSRR_RESET_SHIFT));
   }
}

void gpio_out_init(void) {
   gpio_out_show_count(0);
   gpio_out_show_direction(DIRECTION_UNKNOWN);
   gpio_out_show_error(false);
   gpio_out_set_measure_pin(false);
}

void gpio_out_show_count(int32_t phase_count) {
   uint32_t pattern = ((uint32_t)phase_count) & COUNT_LED_MASK;
   uint32_t clear_mask = ((~pattern) & COUNT_LED_MASK) << BSRR_RESET_SHIFT;
   GPIOD->BSRR = pattern | clear_mask;
}

void gpio_out_show_direction(Direction direction) {
   uint32_t bsrr = (1U << (FORWARD_LED_PIN + BSRR_RESET_SHIFT)) |
                   (1U << (BACKWARD_LED_PIN + BSRR_RESET_SHIFT));

   if (direction == DIRECTION_FORWARD) {
      bsrr = (1U << FORWARD_LED_PIN) |
             (1U << (BACKWARD_LED_PIN + BSRR_RESET_SHIFT));
   } else if (direction == DIRECTION_BACKWARD) {
      bsrr = (1U << BACKWARD_LED_PIN) |
             (1U << (FORWARD_LED_PIN + BSRR_RESET_SHIFT));
   }

   GPIOE->BSRR = bsrr;
}

void gpio_out_show_error(bool is_on) { write_pin(ERROR_LED_PIN, is_on); }

void gpio_out_set_measure_pin(bool is_high) { write_pin(MEASURE_PIN, is_high); }
