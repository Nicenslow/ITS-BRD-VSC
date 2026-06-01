/**
 * @file    button.c
 * @brief   Taster am ITS-Adapter (PG0..PG5) und Nucleo-User-Taster (PC13).
 */

#include "button.h"

#include "delay.h"
#include "stm32f429xx.h"
#include "stm32f4xx_hal.h"
#include "stm32f4xx_nucleo_144.h"

#define ITS_BUTTON_FIRST_PIN 0u
#define ITS_BUTTON_LAST_PIN  5u

/**
 * @brief  Prueft, ob einer der Adapter-Taster S1..S6 gedrueckt ist.
 * @param  Keine
 * @retval true  Mindestens ein Taster aktiv (active-low)
 * @retval false Kein Taster gedrueckt
 */
static bool isItsAdapterButtonPressed(void) {
    uint32_t idr = GPIOG->IDR;

    for (unsigned int pin = ITS_BUTTON_FIRST_PIN; pin <= ITS_BUTTON_LAST_PIN; pin++) {
        if (((idr >> pin) & 1u) == 0u) {
            return true;
        }
    }
    return false;
}

/**
 * @brief  Prueft den blauen User-Taster auf dem Nucleo-Board.
 * @param  Keine
 * @retval true  Taster gedrueckt
 * @retval false Taster nicht gedrueckt
 */
static bool isNucleoUserButtonPressed(void) {
    return (HAL_GPIO_ReadPin(USER_BUTTON_GPIO_PORT, USER_BUTTON_PIN) == GPIO_PIN_RESET);
}

void button_init(void) {
    USER_BUTTON_GPIO_CLK_ENABLE();

    GPIO_InitTypeDef gpio = {0};
    gpio.Pin   = USER_BUTTON_PIN;
    gpio.Mode  = GPIO_MODE_INPUT;
    gpio.Pull  = GPIO_NOPULL;
    gpio.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(USER_BUTTON_GPIO_PORT, &gpio);
}

bool button_isPressed(void) {
    return isItsAdapterButtonPressed() || isNucleoUserButtonPressed();
}

void button_waitForPress(void) {
    /* Entprellen: Druck abwarten, kurz warten, Loslassen abwarten */
    while (!button_isPressed()) {
    }
    delay(30u);
    while (button_isPressed()) {
    }
    delay(30u);
}
