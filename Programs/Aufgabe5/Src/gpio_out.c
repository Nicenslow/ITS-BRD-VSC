/**
 ******************************************************************************
 * @file    gpio_out.c
 * @brief   LED-Ausgaenge: PD0–7 Zaehler, PE4 Messpin, PE5–PE7 Richtung/Fehler.
 ******************************************************************************
 */

#include "gpio_out.h"

#include "stm32f429xx.h"

/** @brief unteres Byte der Phasen-LEDs (D8–D15) */
#define PULSE_LED_MASK 0xFFu

/** @brief Fehler-LED D21 -> PE5 */
#define LED_ERROR_PIN 5u
/** @brief Messpin D20 -> PE4 */
#define MEASUREMENT_PIN 4u
/** @brief Rueckwaerts D22 -> PE6 */
#define LED_BACKWARD_PIN 6u
/** @brief Vorwaerts D23 -> PE7 */
#define LED_FORWARD_PIN 7u

/**
 * @brief  Ausgangs-GPIOs sind durch initITSboard() konfiguriert.
 * @param  None
 * @retval None
 */
void gpioOut_init(void) {
    gpioOut_setMeasurementPin(false);
}

/**
 * @brief  Schreibt die unteren 8 Bit des Zaehlers auf PD0–PD7.
 * @param  pulseCount Phasenimpulszaehler
 * @retval None
 */
void gpioOut_setPulseCountLEDs(int32_t pulseCount) {
    uint32_t pattern = ((uint32_t)pulseCount) & PULSE_LED_MASK;
    uint32_t setMask = pattern;
    uint32_t clrMask = ((~pattern) & PULSE_LED_MASK) << 16u;
    GPIOD->BSRR = setMask | clrMask;
}

/**
 * @brief  Steuert Richtungs-LEDs D22/D23 (PE6/PE7).
 * @param  dir erkannte Drehrichtung
 * @retval None
 */
void gpioOut_setDirectionLEDs(Direction_t dir) {
    uint32_t bsrr = 0u;

    switch (dir) {
        case DIR_FORWARD:
            bsrr = (1u << LED_FORWARD_PIN) | (1u << (LED_BACKWARD_PIN + 16u));
            break;
        case DIR_BACKWARD:
            bsrr = (1u << LED_BACKWARD_PIN) | (1u << (LED_FORWARD_PIN + 16u));
            break;
        case DIR_UNKNOWN:
        default:
            bsrr = (1u << (LED_FORWARD_PIN + 16u)) | (1u << (LED_BACKWARD_PIN + 16u));
            break;
    }
    GPIOE->BSRR = bsrr;
}

/**
 * @brief  Schaltet die Fehler-LED D21 (PE5).
 * @param  on true = LED ein
 * @retval None
 */
void gpioOut_setErrorLED(bool on) {
    if (on) {
        GPIOE->BSRR = (1u << LED_ERROR_PIN);
    } else {
        GPIOE->BSRR = (1u << (LED_ERROR_PIN + 16u));
    }
}

/**
 * @brief  Schaltet den Messpin D20 (PE4) fuer Oszilloskopmessungen.
 * @param  on true = high, false = low
 * @retval None
 */
void gpioOut_setMeasurementPin(bool on) {
    if (on) {
        GPIOE->BSRR = (1u << MEASUREMENT_PIN);
    } else {
        GPIOE->BSRR = (1u << (MEASUREMENT_PIN + 16u));
    }
}
