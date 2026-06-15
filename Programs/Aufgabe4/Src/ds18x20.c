/**
 ******************************************************************************
 * @file    ds18x20.c
 * @brief   ROM-Lesen, Konvertierung und Scratchpad-Auswertung fuer DS18x20.
 ******************************************************************************
 */

#include "ds18x20.h"

#include <stddef.h>

#include "1wire.h"
#include "crc8.h"
#include "timer_util.h"
#include "stm32f429xx.h"

static bool ds18x20_match_rom(const uint8_t rom[8]) {
    ow_write_byte(OW_CMD_MATCH_ROM);

    for (uint8_t i = 0U; i < DS18X20_ROM_LEN; i++) {
        ow_write_byte(rom[i]);
    }

    return true;
}

static bool ds18x20_crc_rom_valid(const uint8_t rom[8]) {
    return (crc8_buf(rom, CRC8_ROM_DATA_LEN) == rom[7]);
}

static bool ds18x20_crc_scratchpad_valid(const uint8_t scratchpad[9]) {
    return (crc8_buf(scratchpad, CRC8_SCRATCHPAD_DATA_LEN) == scratchpad[8]);
}

static bool ds18x20_temp_from_scratchpad(uint8_t family_code,
                                         const uint8_t scratchpad[9],
                                         float *temp_celsius) {
    int16_t raw = (int16_t)((uint16_t)scratchpad[1] << 8U | scratchpad[0]);

    if (family_code == DS18X20_FAMILY_DS18B20) {
        *temp_celsius = (float)raw * DS18X20_B20_TEMP_LSB_C;
        return true;
    }

    if (family_code == DS18X20_FAMILY_DS18S20) {
        *temp_celsius = (float)(raw >> 7) * DS18X20_S20_TEMP_LSB_C;
        return true;
    }

    return false;
}

static bool ds18x20_read_rom_bytes(uint8_t rom[8]) {
    ow_write_byte(OW_CMD_READ_ROM);

    for (uint8_t i = 0U; i < DS18X20_ROM_LEN; i++) {
        rom[i] = ow_read_byte();
    }

    return ds18x20_crc_rom_valid(rom);
}

/**
 * @brief  Liest ROM direkt nach erfolgreichem Reset/Presence (ohne zweiten Reset).
 */
bool ds18x20_read_rom_after_presence(uint8_t rom[8]) {
    ow_strong_pullup_enable();
    bool ok = ds18x20_read_rom_bytes(rom);
    ow_strong_pullup_disable();
    return ok;
}

/**
 * @brief  Liest den 64-Bit-ROM-Code eines einzelnen Sensors (Read ROM).
 */
bool ds18x20_read_rom(uint8_t rom[8]) {
    bool     ok;
    uint32_t primask;

    primask = __get_PRIMASK();
    __disable_irq();

    if (!ow_reset()) {
        ok = false;
    } else {
        ok = ds18x20_read_rom_bytes(rom);
    }

    if (primask == 0U) {
        __enable_irq();
    }

    return ok;
}

/**
 * @brief  Startet die Temperaturmessung fuer einen Sensor (Match ROM + Convert T).
 */
bool ds18x20_start_conversion(const uint8_t rom[8]) {
    if (!ow_reset()) {
        return false;
    }

    (void)ds18x20_match_rom(rom);
    ow_write_byte(DS18X20_CMD_CONVERT_T);

    ow_strong_pullup_enable();
    timerUtil_sleepMs(DS18X20_CONVERSION_MS);
    ow_strong_pullup_disable();

    return true;
}

/**
 * @brief  Liest Scratchpad und berechnet die Temperatur in Grad Celsius.
 */
bool ds18x20_read_temperature(const uint8_t rom[8], float *temp_celsius) {
    uint8_t scratchpad[DS18X20_SCRATCHPAD_LEN];

    if (temp_celsius == NULL) {
        return false;
    }

    if (!ow_reset()) {
        return false;
    }

    (void)ds18x20_match_rom(rom);
    ow_write_byte(DS18X20_CMD_READ_SCRATCHPAD);

    for (uint8_t i = 0U; i < DS18X20_SCRATCHPAD_LEN; i++) {
        scratchpad[i] = ow_read_byte();
    }

    if (!ds18x20_crc_scratchpad_valid(scratchpad)) {
        return false;
    }

    return ds18x20_temp_from_scratchpad(rom[0], scratchpad, temp_celsius);
}
