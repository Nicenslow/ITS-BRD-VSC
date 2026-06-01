/**
 * @file    button.c
 * @brief   Blauer User-Taster auf dem Nucleo (PC13).
 */

#include "button.h"

#include "delay.h"
#include "stm32f4xx_hal.h"
#include "stm32f4xx_nucleo_144.h"

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
    return (HAL_GPIO_ReadPin(USER_BUTTON_GPIO_PORT, USER_BUTTON_PIN) == GPIO_PIN_RESET);
}

void button_waitForPress(void) {
    while (!button_isPressed()) {
    }
    delay(30u);
    while (button_isPressed()) {
    }
    delay(30u);
}
