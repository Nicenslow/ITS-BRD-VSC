/**
 ******************************************************************************
 * @file    main.c
 * @brief   Aufgabe 4: 1-Wire-Temperatursensoren DS18B20 / DS18S20.
 ******************************************************************************
 */

#include <stdint.h>
#include <string.h>

#include "config.h"
#include "init.h"
#include "timer.h"
#include "1wire.h"
#include "ds18x20.h"
#include "display_temp.h"
#include "ow_search.h"
#include "timer_util.h"

#if (AUFGABE4_TEILAUFGABE == 2)

/**
 * Bekannte ROM-Codes (nach Teilaufgabe 1 eintragen).
 * Beispiel-Platzhalter – vor dem Test durch echte Werte ersetzen.
 */
static const uint8_t s_known_roms[][DS18X20_ROM_LEN] = {
    {0x28, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
    {0x28, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01},
};

#define S_KNOWN_SENSOR_COUNT ((uint8_t)(sizeof(s_known_roms) / sizeof(s_known_roms[0])))

#endif

/** @brief Pause zwischen Messzyklen bei Fehler oder nach erfolgreicher Anzeige */
#define MAIN_CYCLE_DELAY_MS 500U

/** @brief CRC-/ROM-Fehler mindestens so viele Zyklen anzeigen */
#define TEIL1_ERROR_HOLD_CYCLES 6U

/** @brief Presence muss so viele Zyklen stabil sein vor ROM-Lesen */
#define TEIL1_PRESENCE_STABLE_CYCLES 2U

/** @brief Aufladung parasitaerer Sensoren nach Power-On */
#define OW_STARTUP_DELAY_MS 1000U

static void hw_init(void) {
    initITSboard();
    initTimer();
    initLCDTouch();
    timerUtil_init();
    ow_init();
    timerUtil_sleepMs(OW_STARTUP_DELAY_MS);
    display_temp_init();
}

#if (AUFGABE4_TEILAUFGABE == 1)

static void run_teilaufgabe1(void) {
    uint8_t        rom[DS18X20_ROM_LEN];
    OwWiringTest_t wiring;
    uint32_t       cycle = 0U;
    bool           rom_pending = false;
    uint8_t        error_hold = 0U;
    bool           last_rom_ok = false;
    uint8_t        last_rom[DS18X20_ROM_LEN];
    uint8_t        presence_stable = 0U;

    wiring = ow_wiring_test();

    while (1) {
        bool idle_high;
        bool presence;
        bool rom_read_attempted = false;
        bool rom_ok             = false;
        bool rom_pending_show   = false;

        cycle++;
        presence  = ow_reset();
        idle_high = ow_bus_read();

        if (presence && idle_high) {
            if (presence_stable < 255U) {
                presence_stable++;
            }
        } else {
            presence_stable = 0U;
        }

        if (error_hold > 0U) {
            error_hold--;
            rom_read_attempted = true;
            rom_ok             = false;
            (void)memcpy(rom, last_rom, DS18X20_ROM_LEN);
        } else if (last_rom_ok) {
            rom_read_attempted = true;
            rom_ok             = true;
            (void)memcpy(rom, last_rom, DS18X20_ROM_LEN);
        } else if (rom_pending) {
            rom_pending = false;

            rom_ok = ds18x20_read_rom(rom);
            (void)memcpy(last_rom, rom, DS18X20_ROM_LEN);
            last_rom_ok = rom_ok;

            if (!rom_ok) {
                ow_init();
                error_hold = TEIL1_ERROR_HOLD_CYCLES;
                last_rom_ok = false;
            }

            /* Ergebnis erst im naechsten Zyklus anzeigen (LCD/SPI kann sehr lange blockieren) */
            timerUtil_sleepMs(MAIN_CYCLE_DELAY_MS);
            continue;
        } else if ((presence_stable >= TEIL1_PRESENCE_STABLE_CYCLES) && !last_rom_ok && (error_hold == 0U)) {
            rom_pending      = true;
            rom_pending_show = true;
            presence_stable  = 0U;
        }

        if (!presence) {
            last_rom_ok = false;
        }

        display_temp_show_teil1_live(&wiring, idle_high, presence, cycle, rom_pending_show,
                                     rom_read_attempted, rom_ok,
                                     rom_read_attempted ? rom : NULL);

        timerUtil_sleepMs(MAIN_CYCLE_DELAY_MS);
    }
}

#endif

#if (AUFGABE4_TEILAUFGABE == 2)

static void run_teilaufgabe2(void) {
    DisplayTempSensor_t sensors[DISPLAY_TEMP_MAX_SENSORS];

    while (1) {
        uint8_t valid_count = 0U;

        for (uint8_t i = 0U; i < S_KNOWN_SENSOR_COUNT; i++) {
            if (valid_count >= DISPLAY_TEMP_MAX_SENSORS) {
                break;
            }

            (void)memcpy(sensors[valid_count].rom, s_known_roms[i], DS18X20_ROM_LEN);
            sensors[valid_count].temp_valid = false;

            if (!ds18x20_start_conversion(s_known_roms[i])) {
                display_temp_show_no_sensor_debug(ow_bus_read());
                goto next_cycle;
            }

            if (!ds18x20_read_temperature(s_known_roms[i], &sensors[valid_count].temp_celsius)) {
                display_temp_show_crc_error();
                goto next_cycle;
            }

            sensors[valid_count].temp_valid = true;
            valid_count++;
        }

        display_temp_show_sensor_list(sensors, valid_count);

    next_cycle:
        timerUtil_sleepMs(MAIN_CYCLE_DELAY_MS);
    }
}

#endif

#if (AUFGABE4_TEILAUFGABE == 3)

static void run_teilaufgabe3(void) {
    DisplayTempSensor_t sensors[DISPLAY_TEMP_MAX_SENSORS];

    while (1) {
        uint8_t rom[DS18X20_ROM_LEN];
        uint8_t count = 0U;
        bool    found = ow_search_first(rom);

        if (!found) {
            display_temp_show_no_sensor_debug(ow_bus_read());
            timerUtil_sleepMs(MAIN_CYCLE_DELAY_MS);
            continue;
        }

        do {
            if (count >= DISPLAY_TEMP_MAX_SENSORS) {
                break;
            }

            (void)memcpy(sensors[count].rom, rom, DS18X20_ROM_LEN);
            sensors[count].temp_valid = false;

            if (!ds18x20_start_conversion(rom)) {
                display_temp_show_no_sensor_debug(ow_bus_read());
                goto next_cycle;
            }

            if (!ds18x20_read_temperature(rom, &sensors[count].temp_celsius)) {
                display_temp_show_crc_error();
                goto next_cycle;
            }

            sensors[count].temp_valid = true;
            count++;
        } while (ow_search_next(rom));

        display_temp_show_sensor_list(sensors, count);

    next_cycle:
        timerUtil_sleepMs(MAIN_CYCLE_DELAY_MS);
    }
}

#endif

/**
 * @brief  Programmstart: Hardware init, dann Endlosschleife der Teilaufgabe.
 */
int main(void) {
    hw_init();

#if (AUFGABE4_TEILAUFGABE == 1)
    run_teilaufgabe1();
#elif (AUFGABE4_TEILAUFGABE == 2)
    run_teilaufgabe2();
#else
    run_teilaufgabe3();
#endif

    return 0;
}
