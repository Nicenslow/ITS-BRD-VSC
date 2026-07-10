/**
 ******************************************************************************
 * @file    ds18x20.c
 * @brief   DS18B20 / DS18S20: Sensor-Befehle ueber den 1-Wire-Bus.
 *
 * === Typischer Bus-Ablauf (nach jedem ow_reset) ===
 *   1. Match ROM (0x55) + 8 Bytes ROM  – einen bestimmten Sensor waehlen
 *      (oder Read ROM 0x33 bei genau einem Sensor – Teilaufgabe 1)
 *   2. Funktionsbefehl senden (Convert T, Read Scratchpad, ...)
 *   3. Daten lesen/schreiben
 *   4. CRC pruefen (crc8.c, AN27)
 *
 * === Parasitaere Versorgung ===
 *   Waehrend Convert T (0x44) braucht der Sensor viel Strom.
 *   -> ow_strong_pullup_enable() haelt PD0+PD1 750 ms auf High.
 ******************************************************************************
 */

#include "ds18x20.h"

#include <stddef.h>

#include "1wire.h"
#include "crc8.h"
#include "timer_util.h"
#include "stm32f429xx.h"

/** Match ROM (0x55): danach 8 ROM-Bytes senden, nur dieser Sensor hoert zu */
static bool ds18x20_match_rom(const uint8_t rom[8]) {
    ow_write_byte(OW_CMD_MATCH_ROM);

    for (uint8_t i = 0U; i < DS18X20_ROM_LEN; i++) {
        ow_write_byte(rom[i]);
    }

    return true;
}

/** ROM-CRC: crc8 ueber Bytes 0..6 muss Byte 7 ergeben (sonst ungueltig) */
static bool ds18x20_crc_rom_valid(const uint8_t rom[8]) {
    return (crc8_buf(rom, CRC8_ROM_DATA_LEN) == rom[7]);
}

/** Scratchpad-CRC: Bytes 0..7 muessen Byte 8 ergeben */
static bool ds18x20_crc_scratchpad_valid(const uint8_t scratchpad[9]) {
    return (crc8_buf(scratchpad, CRC8_SCRATCHPAD_DATA_LEN) == scratchpad[8]);
}

/**
 * Rohwert aus Scratchpad in °C umrechnen.
 * DS18B20: 16-Bit signed, 0,0625 °C/Bit (12 Bit Standard).
 * DS18S20: nur oberes Byte mit 0,5 °C/Bit.
 */
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

/** Read ROM (0x33) – liest 8 Bytes; funktioniert NUR bei genau einem Sensor */
static bool ds18x20_read_rom_bytes(uint8_t rom[8]) {
    ow_write_byte(OW_CMD_READ_ROM);   /* Befehl 0x33 */

    for (uint8_t i = 0U; i < DS18X20_ROM_LEN; i++) {
        rom[i] = ow_read_byte();      /* 64 Bit Registration ROM, LSB first */
    }

    return ds18x20_crc_rom_valid(rom);
}

bool ds18x20_read_rom_after_presence(uint8_t rom[8]) {
    ow_strong_pullup_enable();
    bool ok = ds18x20_read_rom_bytes(rom);
    ow_strong_pullup_disable();
    return ok;
}

/**
 * @brief  ROM lesen inkl. Reset – Haupteinstieg fuer Teilaufgabe 1.
 *
 * Ablauf: ow_reset() -> Read ROM (0x33) -> 8 Bytes -> CRC pruefen.
 * IRQs gesperrt, damit die 64 Bits ohne Unterbrechung uebertragen werden.
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
 * @brief  Temperaturmessung starten (Convert T, Befehl 0x44).
 *
 * Ablauf:
 *   Reset -> Match ROM -> 0x44 senden
 *   -> 750 ms starker Pull-up (PD0+PD1 High, parasit. Stromversorgung)
 *   Sensor misst intern waehrend dieser Zeit.
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
 * @brief  Scratchpad lesen (0xBE) und Temperatur in °C berechnen.
 *
 * Scratchpad-Bytes 0+1 = Temperatur-Rohwert (16-Bit signed).
 * Byte 8 = CRC – muss zu Bytes 0..7 passen.
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
