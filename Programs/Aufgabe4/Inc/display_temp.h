/**
 ******************************************************************************
 * @file    display_temp.h
 * @brief   Temperatur- und Fehlerausgabe auf dem Waveshare 4"-TFT.
 ******************************************************************************
 */

#ifndef AUFGABE4_DISPLAY_TEMP_H
#define AUFGABE4_DISPLAY_TEMP_H

#include <stdbool.h>
#include <stdint.h>

#include "1wire.h"

/** @brief Maximale Anzahl gleichzeitig angezeigter Sensoren */
#define DISPLAY_TEMP_MAX_SENSORS 8U

typedef struct {
    uint8_t rom[8];
    float   temp_celsius;
    bool    temp_valid;
} DisplayTempSensor_t;

void display_temp_init(void);
void display_temp_show_no_sensor(void);
void display_temp_show_no_sensor_debug(bool bus_high);
void display_temp_show_diagnostic(const OwWiringTest_t *wiring, bool bus_high);
void display_temp_show_teil1_live(const OwWiringTest_t *wiring, bool idle_high, bool presence,
                                  uint32_t cycle, bool rom_pending, bool rom_read_attempted,
                                  bool rom_ok, const uint8_t rom[8]);
void display_temp_show_crc_error(void);
void display_temp_show_sensor_list(const DisplayTempSensor_t sensors[], uint8_t count);
void display_temp_show_rom_only(const uint8_t rom[8]);

#endif /* AUFGABE4_DISPLAY_TEMP_H */
