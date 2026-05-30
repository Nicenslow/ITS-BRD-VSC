#include "button.h"

#include "delay.h"
#include "stm32f429xx.h"
#include "stm32f4xx_hal.h"
#include "stm32f4xx_nucleo_144.h"

/** Adapter-Taster S1..S6 = INT0..INT5 an PG0..PG5 (active-low, Pull-up in initITSboard). */
#define ITS_BUTTON_FIRST_PIN 0u
#define ITS_BUTTON_LAST_PIN  5u

static bool isItsAdapterButtonPressed(void) {
    uint32_t idr = GPIOG->IDR;

    for (unsigned int pin = ITS_BUTTON_FIRST_PIN; pin <= ITS_BUTTON_LAST_PIN; pin++) {
        if (((idr >> pin) & 1u) == 0u) {
            return true;
        }
    }
    return false;
}

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
    while (!button_isPressed()) {
    }
    delay(30u);
    while (button_isPressed()) {
    }
    delay(30u);
}
