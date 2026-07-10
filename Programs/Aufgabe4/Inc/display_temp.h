/**
 ******************************************************************************
 * @file    display_temp.h
 * @brief   LCD-Ausgabe: Diagnose (Teil 1), Sensorliste mit Temperatur (Teil 2/3).
 ******************************************************************************
 */

#ifndef AUFGABE4_DISPLAY_TEMP_H
#define AUFGABE4_DISPLAY_TEMP_H

#include <stdbool.h>
#include <stdint.h>

#include "1wire.h"

#define DISPLAY_TEMP_MAX_SENSORS 8U

/** Ein Sensor-Eintrag fuer die Temperatur-Liste auf dem LCD */
typedef struct {
    uint8_t rom[8];       /* 64-Bit Registration ROM als 8 Bytes */
    float   temp_celsius; /* gemessene Temperatur in Grad Celsius */
    bool    temp_valid;   /* true wenn Messung erfolgreich war */
} DisplayTempSensor_t;

void display_temp_init(void);
void display_temp_show_no_sensor(void);
void display_temp_show_no_sensor_debug(bool bus_high);
void display_temp_show_diagnostic(const OwWiringTest_t *wiring, bool bus_high);
void display_temp_show_teil1_live(const OwWiringTest_t *wiring, const OwPullupDiag_t *pullup,
                                  OwResetResult_t reset_result, bool idle_high, bool presence,
                                  uint32_t cycle, bool rom_pending, bool rom_read_attempted,
                                  bool rom_ok, const uint8_t rom[8]);
void display_temp_show_crc_error(void);
void display_temp_show_sensor_list(const DisplayTempSensor_t sensors[], uint8_t count);
void display_temp_show_rom_only(const uint8_t rom[8]);

#endif /* AUFGABE4_DISPLAY_TEMP_H */
