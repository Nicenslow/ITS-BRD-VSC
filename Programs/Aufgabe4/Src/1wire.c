/**
 ******************************************************************************
 * @file    1wire.c
 * @brief   1-Wire-Bus-Treiber (ow = One-Wire) auf PD0/PD1.
 *
 * PD0 (OUT0): Open-Drain fuer DQ – Master zieht Bus auf Low oder laesst los.
 * PD1 (OUT1): Push-Pull High ueber Widerstand = schwacher Pull-up + parasit. VCC.
 *
 * Mikrosekunden-Timing ueber DWT-Zykluszaehler (180 MHz CPU).
 * IRQs werden bei Reset und Bit-Zugriff kurz gesperrt, damit das Timing stimmt.
 ******************************************************************************
 */

#include "1wire.h"

#include "stm32f429xx.h"
#include "stm32f4xx_hal.h"

#define OW_CPU_CYCLES_PER_US 180U

static bool s_data_pin_ready = false;

/** Letztes Ergebnis von ow_reset() – fuer Anzeige auf dem Display */
static OwResetResult_t s_last_reset_result = OW_RESET_NO_PRESENCE;

/* --- Hilfsfunktionen: Timing und GPIO-Modus fuer PD0 --- */

static void ow_ensure_dwt(void) {
    static bool s_dwt_ready = false;

    if (s_dwt_ready) {
        return;
    }

    CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;
    DWT->CYCCNT = 0U;
    DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;
    s_dwt_ready = true;
}

/** Busy-wait in Mikrosekunden (genau fuer 1-Wire-Timing noetig) */
static void ow_delay_us(uint32_t us) {
    uint32_t start;
    uint32_t cycles;

    if (us == 0U) {
        return;
    }

    ow_ensure_dwt();
    start  = DWT->CYCCNT;
    cycles = us * OW_CPU_CYCLES_PER_US;

    while ((uint32_t)(DWT->CYCCNT - start) < cycles) {
    }
}

/** PD1 als Ausgang High – parasitaere Versorgung / Pull-up-Quelle */
static void ow_power_pin_init(void) {
    GPIO_InitTypeDef gpio = {0};

    __HAL_RCC_GPIOD_CLK_ENABLE();

    gpio.Pin   = OW_PIN_POWER;
    gpio.Mode  = GPIO_MODE_OUTPUT_PP;
    gpio.Pull  = GPIO_NOPULL;
    gpio.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(OW_GPIO_PORT, &gpio);
    HAL_GPIO_WritePin(OW_GPIO_PORT, OW_PIN_POWER, GPIO_PIN_SET);
}

/** PD0 einmalig als Open-Drain konfigurieren (1-Wire-Standard) */
static void ow_data_pin_init_once(void) {
    GPIO_InitTypeDef gpio = {0};

    if (s_data_pin_ready) {
        return;
    }

    __HAL_RCC_GPIOD_CLK_ENABLE();

    gpio.Pin   = OW_PIN_DATA;
    gpio.Mode  = GPIO_MODE_OUTPUT_OD;
    gpio.Pull  = GPIO_NOPULL;
    gpio.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(OW_GPIO_PORT, &gpio);

    s_data_pin_ready = true;
}

/** PD0 kurzzeitig Push-Pull (fuer Reset-Puls und Verdrahtungstest) */
static void ow_data_mode_output_pp(void) {
    ow_data_pin_init_once();
    OW_GPIO_PORT->MODER = (OW_GPIO_PORT->MODER & ~GPIO_MODER_MODER0) | GPIO_MODER_MODER0_0;
    OW_GPIO_PORT->OTYPER &= ~GPIO_OTYPER_OT0;
}

static void ow_data_mode_output_od(void) {
    ow_data_pin_init_once();
    OW_GPIO_PORT->MODER = (OW_GPIO_PORT->MODER & ~GPIO_MODER_MODER0) | GPIO_MODER_MODER0_0;
    OW_GPIO_PORT->OTYPER |= GPIO_OTYPER_OT0;
}

/** Zurueck zu Open-Drain, Bus loslassen (High-Z, externer Pull-up zieht hoch) */
static void ow_data_restore_open_drain(void) {
    s_data_pin_ready = false;
    ow_data_mode_output_od();
    OW_GPIO_PORT->BSRR = GPIO_BSRR_BS0;
}

static void ow_data_mode_output(void) {
    ow_data_mode_output_od();
}

/** PD0 als Eingang – Pegel vom Bus lesen (kein interner Pull-up) */
static void ow_data_mode_input(void) {
    ow_data_pin_init_once();
    OW_GPIO_PORT->MODER &= ~GPIO_MODER_MODER0;
    OW_GPIO_PORT->PUPDR &= ~GPIO_PUPDR_PUPD0;
}

static void ow_data_low(void) {
    ow_data_mode_output();
    OW_GPIO_PORT->BSRR = GPIO_BSRR_BR0;
}

static void ow_data_release(void) {
    ow_data_mode_output();
    OW_GPIO_PORT->BSRR = GPIO_BSRR_BS0;
}

static bool ow_data_read(void) {
    return (OW_GPIO_PORT->IDR & GPIO_IDR_ID0) != 0U;
}

/* --- Oeffentliche Diagnose-Funktionen --- */

/**
 * @brief  Test ob PD0 den Bus direkt treiben kann (Push-Pull, ohne externen Pull-up).
 *        Wird nur einmal beim Start ausgewertet – ersetzt keinen PD1-Pull-up-Test.
 */
OwWiringTest_t ow_wiring_test(void) {
    OwWiringTest_t result = {false, false};

    HAL_GPIO_WritePin(OW_GPIO_PORT, OW_PIN_POWER, GPIO_PIN_SET);

    ow_data_mode_output_pp();
    OW_GPIO_PORT->BSRR = GPIO_BSRR_BR0;
    ow_delay_us(200U);
    result.pd0_low_ok = !ow_data_read();

    OW_GPIO_PORT->BSRR = GPIO_BSRR_BS0;
    ow_delay_us(200U);
    result.pd0_high_ok = ow_data_read();

    ow_data_restore_open_drain();
    HAL_GPIO_WritePin(OW_GPIO_PORT, OW_PIN_POWER, GPIO_PIN_SET);

    return result;
}

/**
 * @brief  Test des echten Pull-up-Pfads PD1 -> Widerstand -> DQ.
 *        PD0 bleibt freigegeben; nur PD1 darf den Bus beeinflussen.
 *
 *        WICHTIG: PD1 kurz auf Low entlaedt den parasitischen Sensor.
 *        Deshalb nur einmal beim Programmstart aufrufen, nicht in jeder Schleife.
 */
OwPullupDiag_t ow_pullup_diag(void) {
    OwPullupDiag_t result = {false, false};

    HAL_GPIO_WritePin(OW_GPIO_PORT, OW_PIN_POWER, GPIO_PIN_SET);
    ow_data_restore_open_drain();
    ow_delay_us(200U);
    result.bus_high_pd1_on = ow_bus_read();

    HAL_GPIO_WritePin(OW_GPIO_PORT, OW_PIN_POWER, GPIO_PIN_RESET);
    ow_delay_us(200U);
    result.bus_low_pd1_off = !ow_bus_read();

    HAL_GPIO_WritePin(OW_GPIO_PORT, OW_PIN_POWER, GPIO_PIN_SET);
    ow_data_restore_open_drain();

    return result;
}

/** Freier Bus-Pegel (nach Reset oder fuer Diagnose) */
bool ow_bus_read(void) {
    GPIO_InitTypeDef gpio = {0};

    gpio.Pin   = OW_PIN_DATA;
    gpio.Mode  = GPIO_MODE_INPUT;
    gpio.Pull  = GPIO_NOPULL;
    gpio.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(OW_GPIO_PORT, &gpio);

    ow_delay_us(10U);

    s_data_pin_ready = false;
    return HAL_GPIO_ReadPin(OW_GPIO_PORT, OW_PIN_DATA) == GPIO_PIN_SET;
}

/* --- Bus-Initialisierung und starker Pull-up --- */

void ow_init(void) {
    ow_power_pin_init();
    ow_data_release();
}

/**
 * @brief  Starker Pull-up fuer parasitaere Temperaturmessung (Convert T).
 *        PD0 und PD1 treiben beide direkt auf 3,3 V (750 ms laut ds18x20.c).
 */
void ow_strong_pullup_enable(void) {
    GPIO_InitTypeDef gpio = {0};

    gpio.Pin   = OW_PIN_DATA;
    gpio.Mode  = GPIO_MODE_OUTPUT_PP;
    gpio.Pull  = GPIO_NOPULL;
    gpio.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(OW_GPIO_PORT, &gpio);

    HAL_GPIO_WritePin(OW_GPIO_PORT, OW_PIN_DATA, GPIO_PIN_SET);
    HAL_GPIO_WritePin(OW_GPIO_PORT, OW_PIN_POWER, GPIO_PIN_SET);
}

void ow_strong_pullup_disable(void) {
    ow_data_restore_open_drain();
    HAL_GPIO_WritePin(OW_GPIO_PORT, OW_PIN_POWER, GPIO_PIN_SET);
}

OwResetResult_t ow_last_reset_result(void) {
    return s_last_reset_result;
}

/**
 * @brief  Haelt den Bus kurz auf High – parasit. Sensor vor Reset aufladen.
 */
static void ow_parasite_precharge(void) {
    HAL_GPIO_WritePin(OW_GPIO_PORT, OW_PIN_POWER, GPIO_PIN_SET);
    ow_data_restore_open_drain();
    ow_delay_us(2000U);
}

/**
 * @brief  1-Wire Reset und Presence-Erkennung (DS18B20 Figure 15).
 *
 * Ablauf: Open-Drain Low -> freigeben -> Pull-up wartet ->
 *         bei ~70 us samplet der Master (Low = Presence).
 */
bool ow_reset(void) {
    bool     presence = false;
    uint32_t primask;

    primask = __get_PRIMASK();
    __disable_irq();

    HAL_GPIO_WritePin(OW_GPIO_PORT, OW_PIN_POWER, GPIO_PIN_SET);
    ow_parasite_precharge();

    ow_data_low();
    ow_delay_us(OW_RESET_LOW_US);
    ow_data_mode_input();

    /* Warten bis Pull-up den Bus auf High gezogen hat (max. 60 us) */
    uint32_t waited_us = 0U;
    bool     bus_high  = false;

    while (waited_us < 60U) {
        ow_delay_us(10U);
        waited_us += 10U;
        if (ow_data_read()) {
            bus_high = true;
            break;
        }
    }

    if (!bus_high) {
        s_last_reset_result = OW_RESET_NO_PULLUP;
        ow_delay_us(OW_RESET_RECOVERY_US);
        ow_data_restore_open_drain();
        if (primask == 0U) {
            __enable_irq();
        }
        return false;
    }

    /* Insgesamt 70 us nach Freigabe samplen (Dallas-Referenz) */
    if (waited_us < OW_PRESENCE_SAMPLE_US) {
        ow_delay_us(OW_PRESENCE_SAMPLE_US - waited_us);
    }
    presence = !ow_data_read();

    if (presence) {
        s_last_reset_result = OW_RESET_PRESENCE;
    } else {
        s_last_reset_result = OW_RESET_NO_PRESENCE;
    }

    ow_delay_us(OW_RESET_RECOVERY_US);
    ow_data_restore_open_drain();

    if (primask == 0U) {
        __enable_irq();
    }

    return presence;
}

/* --- Bit- und Byte-Zugriff (LSB first) --- */

void ow_write_bit(uint8_t bit) {
    uint32_t primask = __get_PRIMASK();

    __disable_irq();

    if (bit != 0U) {
        ow_data_low();
        ow_delay_us(OW_WRITE_1_LOW_US);
        ow_data_release();
        ow_delay_us(OW_SLOT_US - OW_WRITE_1_LOW_US);
    } else {
        ow_data_low();
        ow_delay_us(OW_WRITE_0_LOW_US);
        ow_data_release();
        ow_delay_us(OW_SLOT_US - OW_WRITE_0_LOW_US);
    }

    ow_delay_us(OW_RECOVERY_US);

    if (primask == 0U) {
        __enable_irq();
    }
}

uint8_t ow_read_bit(void) {
    uint8_t  bit;
    uint32_t primask = __get_PRIMASK();

    __disable_irq();

    ow_data_low();
    ow_delay_us(OW_READ_INIT_LOW_US);

    ow_data_mode_input();
    ow_delay_us(OW_READ_SAMPLE_US);

    bit = ow_data_read() ? 1U : 0U;

    ow_delay_us(OW_SLOT_US - OW_READ_INIT_LOW_US - OW_READ_SAMPLE_US);
    ow_delay_us(OW_RECOVERY_US);

    ow_data_release();

    if (primask == 0U) {
        __enable_irq();
    }

    return bit;
}

void ow_write_byte(uint8_t data) {
    for (uint8_t mask = 0x01U; mask != 0U; mask <<= 1U) {
        ow_write_bit((data & mask) != 0U ? 1U : 0U);
    }
}

uint8_t ow_read_byte(void) {
    uint8_t data = 0U;

    for (uint8_t mask = 0x01U; mask != 0U; mask <<= 1U) {
        if (ow_read_bit() != 0U) {
            data |= mask;
        }
    }

    return data;
}
