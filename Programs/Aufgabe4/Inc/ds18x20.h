/**
 ******************************************************************************
 * @file    ds18x20.h
 * @brief   DS18B20 / DS18S20 Temperatursensor-Funktionen.
 ******************************************************************************
 */

#ifndef AUFGABE4_DS18X20_H
#define AUFGABE4_DS18X20_H

#include <stdbool.h>
#include <stdint.h>

/** @brief Family-Code DS18B20 */
#define DS18X20_FAMILY_DS18B20 0x28U
/** @brief Family-Code DS18S20 */
#define DS18X20_FAMILY_DS18S20 0x10U

/** @brief Funktionskommando Convert T */
#define DS18X20_CMD_CONVERT_T 0x44U
/** @brief Funktionskommando Read Scratchpad */
#define DS18X20_CMD_READ_SCRATCHPAD 0xBEU

/** @brief Laenge des ROM-Codes in Bytes */
#define DS18X20_ROM_LEN 8U
/** @brief Anzahl Scratchpad-Bytes inkl. CRC */
#define DS18X20_SCRATCHPAD_LEN 9U

/** @brief Maximale Konversionszeit (alle Aufloesungen) in ms */
#define DS18X20_CONVERSION_MS 750U

/** @brief Temperaturaufloesung DS18B20: 0,0625 °C/LSB */
#define DS18X20_B20_TEMP_LSB_C 0.0625f
/** @brief Temperaturaufloesung DS18S20: 0,5 °C/LSB (9 Bit) */
#define DS18X20_S20_TEMP_LSB_C 0.5f

bool ds18x20_read_rom(uint8_t rom[8]);
bool ds18x20_read_rom_after_presence(uint8_t rom[8]);
bool ds18x20_start_conversion(const uint8_t rom[8]);
bool ds18x20_read_temperature(const uint8_t rom[8], float *temp_celsius);

#endif /* AUFGABE4_DS18X20_H */
