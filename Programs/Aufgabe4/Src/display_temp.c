/**
 ******************************************************************************
 * @file    display_temp.c
 * @brief   Deutsche LCD-Ausgabe fuer 1-Wire-Temperatursensoren.
 ******************************************************************************
 */

#include "display_temp.h"

#include <stdio.h>
#include <string.h>
#include <stdint.h>

#include "1wire.h"
#include "LCD_GUI.h"
#include "lcd.h"

/** @brief Schriftgroesse fuer die Sensorliste */
#define DISPLAY_TEMP_FONT 12

/** @brief Erste Textzeile (Zeichenkoordinaten) */
#define DISPLAY_TEMP_ROW_START 1U
/** @brief Zeilenabstand */
#define DISPLAY_TEMP_ROW_STEP 2U

typedef enum {
    DISPLAY_TEMP_VIEW_NONE = 0,
    DISPLAY_TEMP_VIEW_NO_SENSOR,
    DISPLAY_TEMP_VIEW_DIAGNOSTIC,
    DISPLAY_TEMP_VIEW_TEIL1_LIVE,
    DISPLAY_TEMP_VIEW_CRC_ERROR,
    DISPLAY_TEMP_VIEW_ROM,
    DISPLAY_TEMP_VIEW_LIST
} DisplayTempView_t;

static DisplayTempView_t s_last_view = DISPLAY_TEMP_VIEW_NONE;
static char              s_last_screen[96] = "";
static bool              s_teil1_header_drawn = false;

static void display_temp_clear_screen(void) {
    GUI_clear(BLACK);
    lcdSetFont(DISPLAY_TEMP_FONT);
}

static void display_temp_goto_row(uint8_t row_index) {
    POINT y = (POINT)(DISPLAY_TEMP_ROW_START + (row_index * DISPLAY_TEMP_ROW_STEP));
    lcdGotoXY((POINT)1, y);
}

/** @brief Mindestbreite fuer lcdPrintReplS (Reste alter Texte vermeiden) */
#define DISPLAY_TEIL1_LINE_WIDTH 24U

static void display_temp_pad_line(char line[], size_t len) {
    size_t n = strlen(line);

    while ((n < DISPLAY_TEIL1_LINE_WIDTH) && ((n + 1U) < len)) {
        line[n] = ' ';
        n++;
    }
    line[n] = '\0';
}

static void display_temp_format_rom(char *buf, unsigned int buf_len, const uint8_t rom[8]) {
    (void)snprintf(buf, buf_len,
                   "%02X%02X%02X%02X%02X%02X%02X%02X",
                   rom[0], rom[1], rom[2], rom[3],
                   rom[4], rom[5], rom[6], rom[7]);
}

/**
 * @brief  Initialisiert die Display-Ausgabe.
 */
void display_temp_init(void) {
    s_last_view           = DISPLAY_TEMP_VIEW_NONE;
    s_teil1_header_drawn  = false;
    display_temp_clear_screen();
    lcdGotoXY((POINT)1, (POINT)0);
    lcdPrintS("1-Wire startet...");
}

/**
 * @brief  Zeigt die Fehlermeldung bei leerem Bus.
 */
void display_temp_show_no_sensor(void) {
    if (s_last_view == DISPLAY_TEMP_VIEW_NO_SENSOR) {
        return;
    }

    s_last_view = DISPLAY_TEMP_VIEW_NO_SENSOR;
    display_temp_clear_screen();
    lcdGotoXY((POINT)1, (POINT)3);
    lcdPrintS("Kein Sensor gefunden");
}

static void display_temp_show_diagnostic_lines(char line1[], char line2[]) {
    char screen[96];

    (void)snprintf(screen, sizeof(screen), "%s|%s", line1, line1[0] == '\0' ? "" : line2);

    if ((s_last_view == DISPLAY_TEMP_VIEW_DIAGNOSTIC) && (strcmp(s_last_screen, screen) == 0)) {
        return;
    }

    s_last_view = DISPLAY_TEMP_VIEW_DIAGNOSTIC;
    (void)strncpy(s_last_screen, screen, sizeof(s_last_screen) - 1U);
    s_last_screen[sizeof(s_last_screen) - 1U] = '\0';

    display_temp_clear_screen();
    lcdGotoXY((POINT)1, (POINT)1);
    lcdPrintS("1-Wire Diagnose");
    lcdGotoXY((POINT)1, (POINT)3);
    lcdPrintS(line1);
    if (line2[0] != '\0') {
        lcdGotoXY((POINT)1, (POINT)5);
        lcdPrintS(line2);
    }
}

void display_temp_show_no_sensor_debug(bool bus_high) {
    char line1[48];

    (void)snprintf(line1, sizeof(line1), "Kein Sensor (DQ=%c)", bus_high ? '1' : '0');
    display_temp_show_diagnostic_lines(line1, "PD0=DQ  PD1=VCC");
}

void display_temp_show_diagnostic(const OwWiringTest_t *wiring, bool bus_high) {
    char line1[48];
    char line2[48];

    if (wiring == NULL) {
        display_temp_show_no_sensor_debug(bus_high);
        return;
    }

    (void)snprintf(line1, sizeof(line1), "PD0 Low:%s High:%s",
                   wiring->pd0_low_ok ? "OK" : "FAIL",
                   wiring->pd0_high_ok ? "OK" : "FAIL");
    (void)snprintf(line2, sizeof(line2), "Kein Sensor (DQ=%c)", bus_high ? '1' : '0');
    display_temp_show_diagnostic_lines(line1, line2);
}

/**
 * @brief  Live-Anzeige fuer Teilaufgabe 1 (Idle-DQ vor Reset, aktualisiert laufend).
 */
void display_temp_show_teil1_live(const OwWiringTest_t *wiring, bool idle_high, bool presence,
                                  uint32_t cycle, bool rom_pending, bool rom_read_attempted,
                                  bool rom_ok, const uint8_t rom[8]) {
    char line1[48];
    char line2[48];
    char line3[48];
    char line4[48];
    bool wiring_ok = (wiring != NULL && wiring->pd0_low_ok && wiring->pd0_high_ok);

    (void)snprintf(line1, sizeof(line1), "PD0 Low:%s High:%s",
                   (wiring != NULL && wiring->pd0_low_ok) ? "OK" : "FAIL",
                   (wiring != NULL && wiring->pd0_high_ok) ? "OK" : "FAIL");
    (void)snprintf(line2, sizeof(line2), "Idle DQ=%c  #%lu",
                   idle_high ? '1' : '0', (unsigned long)cycle);

    if (!idle_high) {
        (void)snprintf(line3, sizeof(line3), "Bus Low (GND?)");
        (void)snprintf(line4, sizeof(line4), "Presence: %s", presence ? "ja" : "nein");
    } else if (!wiring_ok) {
        (void)snprintf(line3, sizeof(line3), "PD0 intern OK");
        (void)snprintf(line4, sizeof(line4), "Presence: %s", presence ? "ja" : "nein");
    } else if (rom_read_attempted && rom_ok && (rom != NULL)) {
        (void)snprintf(line3, sizeof(line3), "ROM gelesen:");
        (void)snprintf(line4, sizeof(line4), "%02X%02X%02X%02X%02X%02X%02X%02X",
                       rom[0], rom[1], rom[2], rom[3], rom[4], rom[5], rom[6], rom[7]);
    } else if (rom_read_attempted && !rom_ok) {
        (void)snprintf(line3, sizeof(line3), "Sensor antwortet");
        (void)snprintf(line4, sizeof(line4), "CRC-Fehler! 1 Sensor?");
    } else if (rom_pending) {
        (void)snprintf(line3, sizeof(line3), "Sensor antwortet");
        (void)snprintf(line4, sizeof(line4), "Lese ROM jetzt...");
    } else if (presence) {
        (void)snprintf(line3, sizeof(line3), "Presence erkannt");
        (void)snprintf(line4, sizeof(line4), "Warte auf ROM...");
    } else {
        (void)snprintf(line3, sizeof(line3), "Kein Sensor");
        (void)snprintf(line4, sizeof(line4), "Bus OK (Pullup)");
    }

    display_temp_pad_line(line1, sizeof(line1));
    display_temp_pad_line(line2, sizeof(line2));
    display_temp_pad_line(line3, sizeof(line3));
    display_temp_pad_line(line4, sizeof(line4));

    if (!s_teil1_header_drawn) {
        display_temp_clear_screen();
        lcdGotoXY((POINT)1, (POINT)0);
        lcdPrintS("1-Wire Diagnose");
        s_teil1_header_drawn = true;
    }

    lcdGotoXY((POINT)1, (POINT)2);
    lcdPrintReplS(line1);
    lcdGotoXY((POINT)1, (POINT)4);
    lcdPrintReplS(line2);
    lcdGotoXY((POINT)1, (POINT)6);
    lcdPrintReplS(line3);
    lcdGotoXY((POINT)1, (POINT)8);
    lcdPrintReplS(line4);

    s_last_view = DISPLAY_TEMP_VIEW_TEIL1_LIVE;
}

/**
 * @brief  Zeigt die Fehlermeldung bei CRC-Fehler.
 */
void display_temp_show_crc_error(void) {
    if (s_last_view == DISPLAY_TEMP_VIEW_CRC_ERROR) {
        return;
    }

    s_last_view = DISPLAY_TEMP_VIEW_CRC_ERROR;
    display_temp_clear_screen();
    lcdGotoXY((POINT)1, (POINT)3);
    lcdPrintS("CRC-Fehler - Messung wiederholen");
}

/**
 * @brief  Gibt den ROM-Code eines einzelnen Sensors aus (Teilaufgabe 1).
 */
void display_temp_show_rom_only(const uint8_t rom[8]) {
    char line[48];
    char screen[48];

    display_temp_format_rom(line, sizeof(line), rom);
    (void)strncpy(screen, line, sizeof(screen) - 1U);
    screen[sizeof(screen) - 1U] = '\0';

    if ((s_last_view == DISPLAY_TEMP_VIEW_ROM) && (strcmp(s_last_screen, screen) == 0)) {
        return;
    }

    s_last_view = DISPLAY_TEMP_VIEW_ROM;
    (void)strncpy(s_last_screen, screen, sizeof(s_last_screen) - 1U);
    s_last_screen[sizeof(s_last_screen) - 1U] = '\0';

    display_temp_clear_screen();
    lcdGotoXY((POINT)1, (POINT)1);
    lcdPrintS("ROM-Code:");
    lcdGotoXY((POINT)1, (POINT)3);
    lcdPrintS(line);
}

/**
 * @brief  Zeigt Index, ROM-Code und Temperatur fuer alle Sensoren.
 */
void display_temp_show_sensor_list(const DisplayTempSensor_t sensors[], uint8_t count) {
    char line[56];

    if (count == 0U) {
        display_temp_show_no_sensor();
        return;
    }

    s_last_view = DISPLAY_TEMP_VIEW_LIST;
    display_temp_clear_screen();
    lcdGotoXY((POINT)1, (POINT)0);
    lcdPrintS("Sensoren:");

    for (uint8_t i = 0U; i < count; i++) {
        char rom_hex[20];

        display_temp_format_rom(rom_hex, sizeof(rom_hex), sensors[i].rom);

        if (sensors[i].temp_valid) {
            (void)snprintf(line, sizeof(line), "%u: %s  %.2f C",
                           (unsigned int)(i + 1U), rom_hex, (double)sensors[i].temp_celsius);
        } else {
            (void)snprintf(line, sizeof(line), "%u: %s  ---",
                           (unsigned int)(i + 1U), rom_hex);
        }

        display_temp_goto_row((uint8_t)(i + 1U));
        lcdPrintS(line);
    }
}
