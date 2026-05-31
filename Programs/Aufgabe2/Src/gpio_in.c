/**
 ******************************************************************************
 * @file    gpio_in.c
 * @brief   Drehgeber (PF0/PF1) und Reset-Taster S6 (PF6, active-low).
 ******************************************************************************
 */

#include "gpio_in.h"

#include "stm32f429xx.h"

/** @brief Kanal A (IN0) */
#define ENCODER_A_PIN 0u
/** @brief Kanal B (IN1) */
#define ENCODER_B_PIN 1u

/** @brief Reset-Taster S6 liegt auf dem Eingang IN6 / PF6. */
#define RESET_BTN_PIN 6u
/** @brief Fallback fuer die INT0-INT5-Eingaenge auf PG0-PG5. */
#define RESET_INT_MASK 0x3Fu

/**
 * @brief  Initialisiert Eingangs-GPIOs ( erfolgt bereits durch initITSboard() ).
 * @param  None
 * @retval None
 */
void gpioIn_init(void) {
}

/**
 * @brief  Liest PF0/PF1 und bildet die Pico-Quadraturfolge auf Phase A-D ab.
 * @param  None
 * @retval Aktuelle Phase fuer die FSM
 */
FsmState_t gpioIn_readPhase(void) {
    uint32_t idr = GPIOF->IDR;
    unsigned int a = (idr >> ENCODER_A_PIN) & 1u;
    unsigned int b = (idr >> ENCODER_B_PIN) & 1u;

    if (a == 0u && b == 0u) {
        return FSM_PHASE_A;
    }
    if (a == 0u && b == 1u) {
        return FSM_PHASE_B;
    }
    if (a == 1u && b == 1u) {
        return FSM_PHASE_C;
    }
    return FSM_PHASE_D;
}

/**
 * @brief  Liest Reset/Fehler-Taste S6 (active-low).
 * @param  None
 * @retval true wenn gedrueckt
 */
bool gpioIn_readResetButton(void) {
    uint32_t pf = GPIOF->IDR;
    uint32_t pg = GPIOG->IDR;

    bool s6Pressed = (((pf >> RESET_BTN_PIN) & 1u) == 0u);
    bool intPressed = ((pg & RESET_INT_MASK) != RESET_INT_MASK);

    return s6Pressed || intPressed;
}
