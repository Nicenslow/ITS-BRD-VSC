/**
 ******************************************************************************
 * @file    1wire.c
 * @brief   Bit-/Byte-Zugriff auf den 1-Wire-Bus mit timer-basiertem Timing.
 ******************************************************************************
 */

#include "1wire.h"

#include "stm32f429xx.h"
#include "stm32f4xx_hal.h"

/** @brief CPU-Takte pro us bei 180 MHz */
#define OW_CPU_CYCLES_PER_US 180U

/** @brief PD0 einmalig konfiguriert */
static bool s_data_pin_ready = false;

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

static void ow_data_restore_open_drain(void) {
    s_data_pin_ready = false;
    ow_data_mode_output_od();
    OW_GPIO_PORT->BSRR = GPIO_BSRR_BS0;
}

static void ow_data_mode_output(void) {
    ow_data_mode_output_od();
}

static void ow_data_mode_input(void) {
    ow_data_pin_init_once();
    OW_GPIO_PORT->MODER &= ~GPIO_MODER_MODER0;
    /* Internen Pull-up von initITSboard() abschalten – nur externer Pullup zaehlt */
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

/**
 * @brief  Prueft ob PD0 den Bus wirklich treiben kann (Verdrahtungstest).
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
 * @brief  Liest den aktuellen Pegel auf PD0 (Diagnose, ohne vorherigen Ausgangstest).
 */
bool ow_bus_read(void) {
    GPIO_InitTypeDef gpio = {0};

    gpio.Pin   = OW_PIN_DATA;
    gpio.Mode  = GPIO_MODE_INPUT;
    gpio.Pull  = GPIO_NOPULL;
    gpio.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(OW_GPIO_PORT, &gpio);

    /* Kurz warten, damit der externe Pegel am Pad anliegt */
    ow_delay_us(10U);

    s_data_pin_ready = false;
    return HAL_GPIO_ReadPin(OW_GPIO_PORT, OW_PIN_DATA) == GPIO_PIN_SET;
}

/**
 * @brief  Initialisiert PD0 (Open-Drain) und PD1 dauerhaft auf High.
 */
void ow_init(void) {
    ow_power_pin_init();
    ow_data_release();
}

/**
 * @brief  Aktiviert starken Pullup (PD0 + PD1 aktiv auf High, Push-Pull).
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

/**
 * @brief  Beendet starken Pullup, PD0 wieder Open-Drain freigegeben.
 */
void ow_strong_pullup_disable(void) {
    ow_data_restore_open_drain();
    HAL_GPIO_WritePin(OW_GPIO_PORT, OW_PIN_POWER, GPIO_PIN_SET);
}

/**
 * @brief  Reset-Puls und Presence-Erkennung (Maxim-Referenz-Timing).
 */
bool ow_reset(void) {
    bool presence = false;

    HAL_GPIO_WritePin(OW_GPIO_PORT, OW_PIN_POWER, GPIO_PIN_SET);

    ow_data_mode_output_pp();
    OW_GPIO_PORT->BSRR = GPIO_BSRR_BR0;
    ow_delay_us(OW_RESET_LOW_US);

    ow_data_mode_input();
    ow_delay_us(OW_PRESENCE_FIRST_US);

    /* Nach Reset muss der Pullup den Bus auf High ziehen – dauerhaft Low = Kurzschluss */
    if (!ow_data_read()) {
        ow_delay_us(OW_RESET_RECOVERY_US);
        ow_data_restore_open_drain();
        return false;
    }

    for (uint32_t elapsed_us = 0U; elapsed_us < OW_PRESENCE_WINDOW_US;
         elapsed_us += OW_PRESENCE_STEP_US) {
        if (!ow_data_read()) {
            presence = true;
            break;
        }
        ow_delay_us(OW_PRESENCE_STEP_US);
    }

    ow_delay_us(OW_RESET_RECOVERY_US);
    ow_data_restore_open_drain();

    return presence;
}

/**
 * @brief  Schreibt ein einzelnes Bit (LSB zuerst bei Byte-Operationen).
 */
void ow_write_bit(uint8_t bit) {
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
}

/**
 * @brief  Liest ein einzelnes Bit vom Bus.
 */
uint8_t ow_read_bit(void) {
    uint8_t bit;

    ow_data_low();
    ow_delay_us(OW_READ_INIT_LOW_US);

    ow_data_mode_input();
    ow_delay_us(OW_READ_SAMPLE_US);

    bit = ow_data_read() ? 1U : 0U;

    ow_delay_us(OW_SLOT_US - OW_READ_INIT_LOW_US - OW_READ_SAMPLE_US);
    ow_delay_us(OW_RECOVERY_US);

    ow_data_release();

    return bit;
}

/**
 * @brief  Schreibt ein Byte LSB-first auf den Bus.
 */
void ow_write_byte(uint8_t data) {
    for (uint8_t mask = 0x01U; mask != 0U; mask <<= 1U) {
        ow_write_bit((data & mask) != 0U ? 1U : 0U);
    }
}

/**
 * @brief  Liest ein Byte LSB-first vom Bus.
 */
uint8_t ow_read_byte(void) {
    uint8_t data = 0U;

    for (uint8_t mask = 0x01U; mask != 0U; mask <<= 1U) {
        if (ow_read_bit() != 0U) {
            data |= mask;
        }
    }

    return data;
}
