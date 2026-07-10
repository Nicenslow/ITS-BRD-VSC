/**
 ******************************************************************************
 * @file    gpio_in.c
 * @brief   Eingaenge: Drehgeber AUX0/AUX1 (PG0/PG1), Reset-Taster S6.
 ******************************************************************************
 */

#include "gpio_in.h"

#include "config.h"

#include "stm32f429xx.h"

/** @brief Reset-Taster S6 liegt auf dem Eingang IN6 / PF6. */
#define RESET_BTN_PIN 6u
/** @brief Fallback fuer die INT0-INT5-Eingaenge auf PG2-PG5 (PG0/PG1 = Drehgeber). */
#define RESET_INT_MASK 0x3Cu

/**
 * @brief  Eingangs-GPIOs sind durch initITSboard() konfiguriert.
 * @param  None
 * @retval None
 */
void gpioIn_init(void) {
}

/**
 * @brief  Liest PG0/PG1 und bildet die Quadraturfolge auf Phase A-D ab.
 * @param  None
 * @retval Aktuelle Phase fuer die FSM
 */
FsmState_t gpioIn_readPhase(void) {
    uint32_t idr = GPIOG->IDR;
    unsigned int a = (idr >> ENCODER_AUX0_PIN) & 1u;
    unsigned int b = (idr >> ENCODER_AUX1_PIN) & 1u;

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
 * @brief  Liest Reset-Taste S6 bzw. INT-Taster (active-low).
 * @param  None
 * @retval true wenn gedrueckt
 */
bool gpioIn_readResetButton(void) {
    uint32_t pf = GPIOF->IDR;
    uint32_t pg = GPIOG->IDR;

    bool s6Pressed  = (((pf >> RESET_BTN_PIN) & 1u) == 0u);
    bool intPressed = ((pg & RESET_INT_MASK) != RESET_INT_MASK);

    return s6Pressed || intPressed;
}
