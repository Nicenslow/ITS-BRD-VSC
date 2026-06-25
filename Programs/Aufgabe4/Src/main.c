/**
 ******************************************************************************
 * @file    main.c
 * @brief   Aufgabe 4: 1-Wire-Temperatursensoren DS18B20 / DS18S20.
 *
 * Ablauf allgemein:
 *   1. hw_init()  – Board, LCD, 1-Wire-Pins, 1 s Aufladezeit fuer parasitaere Sensoren
 *   2. run_teilaufgabeX() – je nach AUFGABE4_TEILAUFGABE in config.h
 *
 * Modul-Schichten:
 *   main -> display_temp / ds18x20 / ow_search -> 1wire -> GPIO (PD0/PD1)
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
 * Feste ROM-Codes fuer Teilaufgabe 2.
 * Werte aus Teilaufgabe 1 uebernehmen (Family-Byte beginnt mit 0x28).
 */
static const uint8_t s_known_roms[][DS18X20_ROM_LEN] = {
    {0x28, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
    {0x28, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01},
};

#define S_KNOWN_SENSOR_COUNT ((uint8_t)(sizeof(s_known_roms) / sizeof(s_known_roms[0])))

#endif

#define MAIN_CYCLE_DELAY_MS            500U
#define TEIL1_ERROR_HOLD_CYCLES        6U   /* CRC-Fehler mehrere Zyklen anzeigen */
#define TEIL1_PRESENCE_STABLE_CYCLES   2U   /* Presence 2x stabil vor ROM-Lesen */
#define OW_STARTUP_DELAY_MS           1000U /* Parasit-Kondensator im Sensor aufladen */
#define OW_RECHARGE_AFTER_PULLUP_MS    500U /* Nach PD1-Low-Test Sensor wieder aufladen */

/**
 * @brief  Einmalige Initialisierung vor der Teilaufgaben-Schleife.
 */
static void hw_init(void) {
    initITSboard();
    initTimer();
    initLCDTouch();
    timerUtil_init();
    ow_init();                              /* PD0=Open-Drain, PD1=High */
    timerUtil_sleepMs(OW_STARTUP_DELAY_MS); /* Sensor braucht Spannung auf DQ */
    display_temp_init();
}

#if (AUFGABE4_TEILAUFGABE == 1)

/**
 * @brief  Teilaufgabe 1: Diagnose, Presence-Erkennung, ROM eines einzelnen Sensors.
 *
 * Pro Zyklus (~500 ms):
 *   ow_reset        -> antwortet ein Geraet? (Presence-Puls)
 *   ow_bus_read     -> freier Bus-Pegel (DQ idle)
 *   bei 2x stabiler Presence -> ds18x20_read_rom() im Folgezyklus
 *
 * ow_pullup_diag / ow_wiring_test nur einmal beim Start (PD1-Low wuerde
 * sonst jeden Zyklus den parasitischen Sensor entladen).
 */
static void run_teilaufgabe1(void) {
    uint8_t        rom[DS18X20_ROM_LEN];
    OwWiringTest_t wiring;
    OwPullupDiag_t pullup;
    uint32_t       cycle = 0U;
    bool           rom_pending = false;
    uint8_t        error_hold = 0U;
    bool           last_rom_ok = false;
    uint8_t        last_rom[DS18X20_ROM_LEN];
    uint8_t        presence_stable = 0U;

    /* Einmalig beim Start: Verdrahtung pruefen (nicht in der Schleife wiederholen) */
    wiring = ow_wiring_test();
    pullup = ow_pullup_diag();
    timerUtil_sleepMs(OW_RECHARGE_AFTER_PULLUP_MS);

    while (1) {
        bool idle_high;
        bool presence;
        bool rom_read_attempted = false;
        bool rom_ok             = false;
        bool rom_pending_show   = false;

        cycle++;

        /* --- Sensor-Erkennung (ohne vorherigen PD1-Low-Test) --- */
        presence  = ow_reset();
        idle_high = ow_bus_read();

        /* Presence nur zaehlen wenn Bus danach auch idle high ist */
        if (presence && idle_high) {
            if (presence_stable < 255U) {
                presence_stable++;
            }
        } else {
            presence_stable = 0U;
        }

        /* --- Zustandsmaschine fuer ROM-Lesen und Fehleranzeige --- */
        if (error_hold > 0U) {
            /* CRC-Fehler noch einige Zyklen auf dem Display halten */
            error_hold--;
            rom_read_attempted = true;
            rom_ok             = false;
            (void)memcpy(rom, last_rom, DS18X20_ROM_LEN);
        } else if (last_rom_ok) {
            /* ROM bereits gelesen – weiter anzeigen */
            rom_read_attempted = true;
            rom_ok             = true;
            (void)memcpy(rom, last_rom, DS18X20_ROM_LEN);
        } else if (rom_pending) {
            /* Presence war stabil -> jetzt ROM lesen (Read ROM 0x33, nur 1 Sensor) */
            rom_pending = false;

            rom_ok = ds18x20_read_rom(rom);
            (void)memcpy(last_rom, rom, DS18X20_ROM_LEN);
            last_rom_ok = rom_ok;

            if (!rom_ok) {
                ow_init();
                error_hold = TEIL1_ERROR_HOLD_CYCLES;
                last_rom_ok = false;
            }

            timerUtil_sleepMs(MAIN_CYCLE_DELAY_MS);
            continue;
        } else if ((presence_stable >= TEIL1_PRESENCE_STABLE_CYCLES) && !last_rom_ok && (error_hold == 0U)) {
            /* Naechster Zyklus startet ROM-Lesen */
            rom_pending      = true;
            rom_pending_show = true;
            presence_stable  = 0U;
        }

        if (!presence) {
            last_rom_ok = false;
        }

        display_temp_show_teil1_live(&wiring, &pullup, ow_last_reset_result(), idle_high, presence,
                                     cycle, rom_pending_show, rom_read_attempted, rom_ok,
                                     rom_read_attempted ? rom : NULL);

        timerUtil_sleepMs(MAIN_CYCLE_DELAY_MS);
    }
}

#endif

#if (AUFGABE4_TEILAUFGABE == 2)

/**
 * @brief  Teilaufgabe 2: Temperatur fuer fest hinterlegte ROM-Codes messen.
 */
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

            /* Match ROM -> Convert T -> 750 ms starker Pull-up -> Scratchpad lesen */
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

/**
 * @brief  Teilaufgabe 3: Search (AN187) findet alle Sensoren, dann Temperatur messen.
 */
static void run_teilaufgabe3(void) {
    DisplayTempSensor_t sensors[DISPLAY_TEMP_MAX_SENSORS];

    while (1) {
        uint8_t rom[DS18X20_ROM_LEN];
        uint8_t count = 0U;

        /* Ersten Sensor auf dem Bus suchen */
        bool found = ow_search_first(rom);

        if (!found) {
            display_temp_show_no_sensor_debug(ow_bus_read());
            timerUtil_sleepMs(MAIN_CYCLE_DELAY_MS);
            continue;
        }

        /* Alle gefundenen Sensoren nacheinander messen */
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
