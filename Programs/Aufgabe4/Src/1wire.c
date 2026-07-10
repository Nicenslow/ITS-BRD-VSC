/**
 ******************************************************************************
 * @file    1wire.c
 * @brief   Niedrigste Schicht: 1-Wire-Protokoll auf PD0 (DQ) und PD1 (VCC).
 *
 * === Hardware (laut Aufgaben-PDF) ===
 *   PD0 (OUT0): Datenleitung DQ – Open-Drain, Master zieht Bus auf Low oder los
 *   PD1 (OUT1): Dauerhaft 3,3 V ueber 3,3 kOhm – schwacher Pull-up + parasit. Versorgung
 *   Bei Temperaturmessung: PD0 UND PD1 auf High = starker Pull-up (mehr Strom)
 *
 * === Timing ===
 *   Mikrosekunden genau per DWT-Zykluszaehler (CPU 180 MHz).
 *   IRQs werden bei Reset und jedem Bit kurz gesperrt (kein Jitter).
 *
 * === Wichtige Funktionen fuer die Erklaerung ===
 *   ow_reset()      – Bus-Reset + Presence-Erkennung (Start jedes Zugriffs)
 *   ow_write_bit()  – ein Bit senden (Write-0 = langes Low, Write-1 = kurzes Low)
 *   ow_read_bit()   – ein Bit empfangen (Master impulst kurz Low, liest Pegel)
 *   ow_write/read_byte() – 8 Bits LSB-first (= Reihenfolge im 1-Wire-Protokoll)
 ******************************************************************************
 */

#include "1wire.h"

#include "stm32f429xx.h"
#include "stm32f4xx_hal.h"

#define OW_CPU_CYCLES_PER_US 180U

static bool s_data_pin_ready = false;

/** Letztes Ergebnis von ow_reset() – fuer Anzeige auf dem Display */
static OwResetResult_t s_last_reset_result = OW_RESET_NO_PRESENCE;

/* --- Hilfsfunktionen: exaktes Timing und GPIO-Modus fuer PD0 --- */

/** DWT (Data Watchpoint and Trace) Zykluszaehler aktivieren – einmalig */
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

/** Busy-Wait in Mikrosekunden – noetig weil 1-Wire-Slots nur 60–70 us lang sind */
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

/** PD1 als Push-Pull-Ausgang auf High – liefert parasitaere Versorgung */
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

/** PD0 als Open-Drain – Standardmodus fuer 1-Wire (Master kann Bus nur auf Low ziehen) */
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

/** PD0 kurz Push-Pull – nur fuer Verdrahtungstest und Reset-Puls (direktes Treiben) */
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

/** Bus loslassen: Open-Drain + intern High setzen -> externer Pull-up zieht DQ hoch */
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

/** PD0 auf Low ziehen (Master dominierend) */
static void ow_data_low(void) {
    ow_data_mode_output();
    OW_GPIO_PORT->BSRR = GPIO_BSRR_BR0;
}

/** PD0 loslassen – Pull-up (PD1/R) muss Bus auf High ziehen */
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
 * @brief  1-Wire Reset + Presence-Erkennung (DS18B20 Datenblatt Figure 15).
 *
 * Ablauf in Kurzform:
 *   1. Master zieht Bus >= 480 us auf Low (Reset-Puls)
 *   2. Master gibt Bus frei -> Pull-up zieht High
 *   3. Sensor antwortet mit Presence-Puls (zieht Bus ~60–240 us auf Low)
 *   4. Master samplet nach ~70 us: Low = Sensor da, High = kein Sensor
 *
 * @return true wenn Presence-Puls erkannt wurde
 */
bool ow_reset(void) {
    bool     presence = false;
    uint32_t primask;

    primask = __get_PRIMASK();
    __disable_irq();

    HAL_GPIO_WritePin(OW_GPIO_PORT, OW_PIN_POWER, GPIO_PIN_SET);
    ow_parasite_precharge();

    /* Schritt 1: Reset-Puls – Bus mindestens 480 us auf Low */
    ow_data_low();
    ow_delay_us(OW_RESET_LOW_US);
    ow_data_mode_input();   /* Bus freigeben, Pull-up arbeitet */

    /* Schritt 2: Warten bis Pull-up den Bus auf High gezogen hat (max. 60 us) */
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

    /* Schritt 3: Nach insgesamt 70 us seit Freigabe den Presence-Puls abtasten */
    if (waited_us < OW_PRESENCE_SAMPLE_US) {
        ow_delay_us(OW_PRESENCE_SAMPLE_US - waited_us);
    }
    presence = !ow_data_read();   /* Low = Sensor hat geantwortet */

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

/* --- Bit- und Byte-Zugriff (jeweils LSB first = Bit 0 zuerst) --- */

/**
 * @brief  Ein Bit auf den Bus schreiben (ein Time-Slot = 70 us).
 *   Write-1: kurz Low (~6 us), dann loslassen – Bus bleibt High
 *   Write-0: lang Low (~65 us) – Bus bleibt Low
 */
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

/**
 * @brief  Ein Bit vom Bus lesen.
 *   Master impulst 1 us Low, gibt frei, samplet nach 14 us den Pegel.
 */
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

/** 8 Bits nacheinander senden – niedrigstes Bit (LSB) zuerst */
void ow_write_byte(uint8_t data) {
    for (uint8_t mask = 0x01U; mask != 0U; mask <<= 1U) {
        ow_write_bit((data & mask) != 0U ? 1U : 0U);
    }
}

/** 8 Bits nacheinander empfangen und zu einem Byte zusammensetzen */
uint8_t ow_read_byte(void) {
    uint8_t data = 0U;

    for (uint8_t mask = 0x01U; mask != 0U; mask <<= 1U) {
        if (ow_read_bit() != 0U) {
            data |= mask;
        }
    }

    return data;
}
