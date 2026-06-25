/**
 ******************************************************************************
 * @file    ds18x20.h
 * @brief   DS18B20 / DS18S20: ROM, Temperaturmessung, parasitaere Versorgung.
 *
 * Baut auf 1wire.c auf. Fuer Temperaturmessung bei parasitaerer Versorgung
 * wird nach Convert T ein starker Pull-up (PD0+PD1) fuer 750 ms benoetigt.
 ******************************************************************************
 */

#ifndef AUFGABE4_DS18X20_H
#define AUFGABE4_DS18X20_H

#include <stdbool.h>
#include <stdint.h>

/** Family-Code im ersten ROM-Byte – 0x28 = DS18B20, 0x10 = DS18S20 */
#define DS18X20_FAMILY_DS18B20 0x28U
#define DS18X20_FAMILY_DS18S20 0x10U

#define DS18X20_CMD_CONVERT_T       0x44U
#define DS18X20_CMD_READ_SCRATCHPAD 0xBEU

#define DS18X20_ROM_LEN         8U
#define DS18X20_SCRATCHPAD_LEN  9U

/** Max. Konversionszeit bei 12-Bit-Aufloesung (Datenblatt tCONV) */
#define DS18X20_CONVERSION_MS 750U

#define DS18X20_B20_TEMP_LSB_C 0.0625f
#define DS18X20_S20_TEMP_LSB_C 0.5f

bool ds18x20_read_rom(uint8_t rom[8]);
bool ds18x20_read_rom_after_presence(uint8_t rom[8]);
bool ds18x20_start_conversion(const uint8_t rom[8]);
bool ds18x20_read_temperature(const uint8_t rom[8], float *temp_celsius);

#endif /* AUFGABE4_DS18X20_H */
