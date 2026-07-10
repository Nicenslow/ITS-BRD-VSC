/**
 ******************************************************************************
 * @file    encoder_irq.c
 * @brief   EXTI-Initialisierung und ISRs fuer Drehgeber AUX0/AUX1.
 ******************************************************************************
 */

#include "encoder_irq.h"

#include "config.h"
#include "fsm.h"
#include "gpio_in.h"
#include "timer_util.h"

#include "stm32f429xx.h"

/**
 * @brief  Routet PG0/PG1 auf EXTI0/EXTI1 und aktiviert Flankeninterrupts.
 * @param  None
 * @retval None
 */
static void encoderIrq_configureHardware(void) {
    RCC->APB2ENR |= RCC_APB2ENR_SYSCFGEN;

    SYSCFG->EXTICR[0] &= ~(0x0Fu << (4u * ENCODER_AUX0_EXTI_LINE));
    SYSCFG->EXTICR[0] |= (EXTI_PORT_GPIOG << (4u * ENCODER_AUX0_EXTI_LINE));

    SYSCFG->EXTICR[0] &= ~(0x0Fu << (4u * ENCODER_AUX1_EXTI_LINE));
    SYSCFG->EXTICR[0] |= (EXTI_PORT_GPIOG << (4u * ENCODER_AUX1_EXTI_LINE));

    EXTI->RTSR |= (1u << ENCODER_AUX0_EXTI_LINE) | (1u << ENCODER_AUX1_EXTI_LINE);
    EXTI->FTSR |= (1u << ENCODER_AUX0_EXTI_LINE) | (1u << ENCODER_AUX1_EXTI_LINE);

    EXTI->IMR |= (1u << ENCODER_AUX0_EXTI_LINE) | (1u << ENCODER_AUX1_EXTI_LINE);

    NVIC_SetPriority(EXTI0_IRQn, 0u);
    NVIC_SetPriority(EXTI1_IRQn, 0u);
    NVIC_EnableIRQ(EXTI0_IRQn);
    NVIC_EnableIRQ(EXTI1_IRQn);
}

/**
 * @brief  Gemeinsame ISR-Logik fuer AUX0 und AUX1.
 * @param  extiLine  EXTI-Leitung (0 oder 1)
 * @retval None
 */
static void encoderIrq_handleEdge(uint32_t extiLine) {
    if ((EXTI->PR & (1u << extiLine)) == 0u) {
        return;
    }

    uint32_t   timestamp = timerUtil_getTimestamp();
    FsmState_t phase     = gpioIn_readPhase();

    fsm_isrUpdate(phase, timestamp);

    EXTI->PR = (1u << extiLine);
}

/**
 * @brief  Liest Startphase, synchronisiert FSM und aktiviert EXTI-Interrupts.
 * @param  None
 * @retval None
 */
void encoderIrq_init(void) {
    FsmState_t initialPhase = gpioIn_readPhase();
    fsm_syncToPhase(initialPhase);
    encoderIrq_configureHardware();
}

/**
 * @brief  ISR fuer Flankenwechsel an AUX0 (PG0 / EXTI0).
 * @param  None
 * @retval None
 */
void EXTI0_IRQHandler(void) {
    encoderIrq_handleEdge(ENCODER_AUX0_EXTI_LINE);
}

/**
 * @brief  ISR fuer Flankenwechsel an AUX1 (PG1 / EXTI1).
 * @param  None
 * @retval None
 */
void EXTI1_IRQHandler(void) {
    encoderIrq_handleEdge(ENCODER_AUX1_EXTI_LINE);
}
