/**
 ******************************************************************************
 * @file    main.c
 * @brief   Aufgabe 4: 1-Wire-Master fuer DS18B20/DS18S20-Temperatursensoren.
 *
 * === Gesamtaufbau (von oben nach unten) ===
 *   main.c          – Steuerlogik je Teilaufgabe (diese Datei)
 *   ds18x20.c       – Sensor-Befehle (Read ROM, Convert T, Read Scratchpad)
 *   ow_search.c     – Bus-Scan: alle Sensoren finden (nur Teilaufgabe 3)
 *   1wire.c         – Bit-/Byte-Zugriff, Reset, Timing auf PD0/PD1
 *   crc8.c          – Pruefsumme fuer ROM und Scratchpad (AN27)
 *   display_temp.c  – LCD-Ausgabe fuer Diagnose und Messwerte
 *   timer_util.c    – Wartezeiten (ms fuer Konversion, us fuer 1-Wire)
 *
 * === Programmstart ===
 *   1. hw_init()         – Board, Timer, LCD, 1-Wire-Pins, 1 s Sensor-Aufladung
 *   2. run_teilaufgabeX() – Endlosschleife gemaess config.h (1, 2 oder 3)
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
 * Feste ROM-Codes fuer Teilaufgabe 2 (manuell aus Teilaufgabe 1 kopiert).
 *
 * Jedes ROM hat 8 Bytes:
 *   [0] Family-Code  (0x28 = DS18B20, 0x10 = DS18S20)
 *   [1..6] Seriennummer (herstellerspezifisch)
 *   [7] CRC-8 (Pruefsumme ueber Bytes 0..6)
 */
static const uint8_t s_known_roms[][DS18X20_ROM_LEN] = {
    {0x28, 0x0B, 0x04, 0x87, 0x0D, 0x00, 0x00, 0xB6},
    {0x28, 0xFF, 0x5E, 0x89, 0x0D, 0x00, 0x00, 0x25},
    {0x28, 0xC1, 0x06, 0x89, 0x0D, 0x00, 0x00, 0x70},
    {0x28, 0xD2, 0xE2, 0x54, 0x0F, 0x00, 0x00, 0x64},
};

#define S_KNOWN_SENSOR_COUNT ((uint8_t)(sizeof(s_known_roms) / sizeof(s_known_roms[0])))

#endif

/* Pause zwischen Display-Aktualisierungen in allen Teilaufgaben */
#define MAIN_CYCLE_DELAY_MS            500U
/* CRC-Fehler bleibt N Zyklen sichtbar, damit man ihn auf dem LCD lesen kann */
#define TEIL1_ERROR_HOLD_CYCLES        6U
/* Presence muss 2x hintereinander erkannt werden, bevor ROM gelesen wird */
#define TEIL1_PRESENCE_STABLE_CYCLES   2U
/* Parasit-Sensor: interner Kondensator braucht ~1 s Spannung auf PD1 */
#define OW_STARTUP_DELAY_MS           1000U
/* Nach Pull-up-Diagnose (PD1 kurz Low) Sensor erneut aufladen */
#define OW_RECHARGE_AFTER_PULLUP_MS    500U

/**
 * @brief  Einmalige Hardware-Initialisierung (alle Teilaufgaben).
 *
 * Reihenfolge wichtig:
 *   Board/Timer/LCD zuerst, dann 1-Wire-Pins, dann Wartezeit fuer
 *   parasitaere Versorgung, zuletzt Display vorbereiten.
 */
static void hw_init(void) {
    initITSboard();
    initTimer();
    initLCDTouch();
    timerUtil_init();
    ow_init();                              /* PD0 = Open-Drain, PD1 = 3,3 V */
    timerUtil_sleepMs(OW_STARTUP_DELAY_MS); /* parasit. Sensor aufladen */
    display_temp_init();
}

#if (AUFGABE4_TEILAUFGABE == 1)

/**
 * @brief  Teilaufgabe 1: 1-Wire-Basis + ROM eines einzelnen Sensors auslesen.
 *
 * === Was die Aufgabe verlangt ===
 *   - Bits/Bytes senden und empfangen (1wire.c)
 *   - 64-Bit Registration ROM zyklisch lesen und auf Display zeigen
 *   - CRC pruefen (crc8.c)
 *   - Fehlermeldung wenn kein Sensor angeschlossen ist
 *
 * === Ablauf pro Zyklus (~500 ms) ===
 *   1. ow_reset()     – Bus resetten, Presence-Puls vom Sensor erkennen
 *   2. ow_bus_read()  – freier Pegel auf DQ (muss idle High sein)
 *   3. Zustandsmaschine – bei stabiler Presence ROM lesen (ds18x20_read_rom)
 *
 * === Warum Diagnose nur einmal beim Start? ===
 *   ow_pullup_diag() zieht PD1 kurz auf Low und entlaedt den parasitischen
 *   Sensor. Deshalb NICHT in der Hauptschleife wiederholen.
 */
static void run_teilaufgabe1(void) {
    uint8_t        rom[DS18X20_ROM_LEN];
    OwWiringTest_t wiring;       /* Snapshot: kann PD0 den Bus treiben? */
    OwPullupDiag_t pullup;       /* Snapshot: funktioniert PD1 -> R -> DQ? */
    uint32_t       cycle = 0U;   /* Zaehler fuer Display (Zyklus #) */
    bool           rom_pending = false;   /* naechster Zyklus: ROM lesen */
    uint8_t        error_hold = 0U;       /* Zyklen, die CRC-Fehler anzeigen */
    bool           last_rom_ok = false;   /* letztes ROM erfolgreich gelesen? */
    uint8_t        last_rom[DS18X20_ROM_LEN];
    uint8_t        presence_stable = 0U;  /* wie oft Presence hintereinander OK */

    /* --- Einmalige Verdrahtungs-Checks (nur Programmstart) --- */
    wiring = ow_wiring_test();   /* PD0 direkt Low/High testen */
    pullup = ow_pullup_diag();   /* externer Pull-up ueber PD1 testen */
    timerUtil_sleepMs(OW_RECHARGE_AFTER_PULLUP_MS);

    while (1) {
        bool idle_high;
        bool presence;
        bool rom_read_attempted = false;  /* fuer Display: ROM-Versuch? */
        bool rom_ok             = false;
        bool rom_pending_show   = false;  /* fuer Display: "Lese ROM jetzt..." */

        cycle++;

        /* Schritt 1+2: Reset-Puls senden und Presence auswerten */
        presence  = ow_reset();
        idle_high = ow_bus_read();

        /* Presence zaehlt nur mit stabilem High-Pegel danach (kein Kurzschluss) */
        if (presence && idle_high) {
            if (presence_stable < 255U) {
                presence_stable++;
            }
        } else {
            presence_stable = 0U;   /* Unterbrechung -> wieder von vorn zaehlen */
        }

        /*
         * Zustandsmaschine (Prioritaet von oben nach unten):
         *   A) error_hold   – CRC-Fehler noch anzeigen
         *   B) last_rom_ok  – gueltiges ROM weiter anzeigen
         *   C) rom_pending  – jetzt Read-ROM-Befehl (0x33) ausfuehren
         *   D) presence OK  – naechsten Zyklus fuer ROM-Lesen vormerken
         */
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
            /* Read ROM 0x33 funktioniert NUR bei genau einem Sensor am Bus */
            rom_pending = false;

            rom_ok = ds18x20_read_rom(rom);
            (void)memcpy(last_rom, rom, DS18X20_ROM_LEN);
            last_rom_ok = rom_ok;

            if (!rom_ok) {
                ow_init();   /* Bus nach fehlgeschlagenem Lesen zuruecksetzen */
                error_hold = TEIL1_ERROR_HOLD_CYCLES;
                last_rom_ok = false;
            }

            timerUtil_sleepMs(MAIN_CYCLE_DELAY_MS);
            continue;   /* diesen Zyklus ohne erneutes Display-Update beenden */
        } else if ((presence_stable >= TEIL1_PRESENCE_STABLE_CYCLES) && !last_rom_ok && (error_hold == 0U)) {
            rom_pending      = true;
            rom_pending_show = true;
            presence_stable  = 0U;
        }

        /* Sensor abgezogen -> gespeichertes ROM verwerfen */
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
 * @brief  Teilaufgabe 2: Temperatur fuer mehrere fest bekannte Sensoren messen.
 *
 * Pro Sensor und Zyklus:
 *   1. Match ROM (0x55 + 8 Bytes) – einen bestimmten Sensor ansprechen
 *   2. Convert T (0x44) – Messung starten, 750 ms starker Pull-up
 *   3. Read Scratchpad (0xBE) – Rohdaten lesen, CRC pruefen, in °C umrechnen
 *
 * ROM-Codes stehen oben in s_known_roms[] (aus Teilaufgabe 1 kopiert).
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
 * @brief  Teilaufgabe 3: Sensoren automatisch finden und Temperaturen anzeigen.
 *
 * Kombiniert ROM-Suche (ow_search.c, AN187) mit Temperaturmessung (ds18x20.c).
 *
 * Phase 1 – Discovery:
 *   ow_search_first/next() liefert alle 64-Bit-ROMs auf dem Bus.
 *
 * Phase 2 – Messung:
 *   Fuer jedes gefundene ROM: Convert T + Scratchpad lesen.
 *   Fehlende Sensoren werden uebersprungen (Rest bleibt sichtbar).
 *   Neu eingesteckte Sensoren erscheinen im naechsten Zyklus automatisch.
 */
static void run_teilaufgabe3(void) {
    DisplayTempSensor_t sensors[DISPLAY_TEMP_MAX_SENSORS];

    while (1) {
        uint8_t rom[DS18X20_ROM_LEN];   /* aktuell gefundenes ROM */
        uint8_t roms[DISPLAY_TEMP_MAX_SENSORS][DS18X20_ROM_LEN];
        uint8_t rom_count   = 0U;   /* wie viele Sensoren der Search fand */
        uint8_t valid_count = 0U;   /* wie viele Messungen erfolgreich waren */

        /* --- Phase 1: Search ROM (0xF0) – alle Geraete auflisten --- */
        if (!ow_search_first(rom)) {
            display_temp_show_no_sensor_debug(ow_bus_read());
            timerUtil_sleepMs(MAIN_CYCLE_DELAY_MS);
            continue;
        }

        do {
            if (rom_count >= DISPLAY_TEMP_MAX_SENSORS) {
                break;
            }

            (void)memcpy(roms[rom_count], rom, DS18X20_ROM_LEN);
            rom_count++;
        } while (ow_search_next(rom));

        /* --- Phase 2: jeden gefundenen Sensor einzeln ansprechen und messen --- */
        for (uint8_t i = 0U; i < rom_count; i++) {
            if (valid_count >= DISPLAY_TEMP_MAX_SENSORS) {
                break;
            }

            if (!ds18x20_start_conversion(roms[i])) {
                continue;   /* Sensor nicht erreichbar – naechster */
            }

            if (!ds18x20_read_temperature(roms[i], &sensors[valid_count].temp_celsius)) {
                continue;
            }

            (void)memcpy(sensors[valid_count].rom, roms[i], DS18X20_ROM_LEN);
            sensors[valid_count].temp_valid = true;
            valid_count++;
        }

        display_temp_show_sensor_list(sensors, valid_count);
        timerUtil_sleepMs(MAIN_CYCLE_DELAY_MS);
    }
}

#endif

/**
 * @brief  Einstiegspunkt: init einmal, dann Endlosschleife der aktiven Teilaufgabe.
 */
int main(void) {
    hw_init();

#if (AUFGABE4_TEILAUFGABE == 1)
    run_teilaufgabe1();
#elif (AUFGABE4_TEILAUFGABE == 2)
    run_teilaufgabe2();
#else
    run_teilaufgabe3();   /* Standard: automatische Sensor-Erkennung */
#endif

    return 0;   /* wird in der Praxis nie erreicht (Endlosschleifen) */
}
