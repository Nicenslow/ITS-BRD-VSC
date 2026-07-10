/**
 ******************************************************************************
 * @file    config.h
 * @brief   Pin- und EXTI-Konfiguration fuer Drehgeber AUX0/AUX1 (PG0/PG1).
 ******************************************************************************
 */

#ifndef AUFGABE5_CONFIG_H
#define AUFGABE5_CONFIG_H

/** @brief Drehgeber Kanal A an Buchse AUX0 (GPIO PG0) */
#define ENCODER_AUX0_PIN 0u
/** @brief Drehgeber Kanal B an Buchse AUX1 (GPIO PG1) */
#define ENCODER_AUX1_PIN 1u

/** @brief EXTI-Leitung fuer AUX0 (Pin 0 -> EXTI0) */
#define ENCODER_AUX0_EXTI_LINE 0u
/** @brief EXTI-Leitung fuer AUX1 (Pin 1 -> EXTI1) */
#define ENCODER_AUX1_EXTI_LINE 1u

/** @brief SYSCFG-Portauswahl fuer GPIO Port G */
#define EXTI_PORT_GPIOG 6u

#endif /* AUFGABE5_CONFIG_H */
