/**
 ******************************************************************************
 * @file    1wire.h
 * @brief   1-Wire-Bus (ow = One-Wire): PD0 = Daten, PD1 = parasitaere Versorgung.
 *
 * Hardware laut Aufgabenstellung:
 *   OUT0 / PD0  -> DQ des DS18B20 (Open-Drain)
 *   OUT1 / PD1  -> 3,3 V ueber Widerstand an den gemeinsamen DQ-Bus
 *   Widerstand  -> zwischen PD1 und demselben Knoten wie PD0/DQ
 ******************************************************************************
 */

#ifndef AUFGABE4_1WIRE_H
#define AUFGABE4_1WIRE_H

#include <stdbool.h>
#include <stdint.h>

/** PD0 am ITS-Board (OUT0) – 1-Wire-Datenleitung DQ */
#define OW_PIN_DATA GPIO_PIN_0
/** PD1 am ITS-Board (OUT1) – dauerhaft High fuer schwachen Pull-up */
#define OW_PIN_POWER GPIO_PIN_1
#define OW_GPIO_PORT GPIOD

/* --- Timing laut DS18B20-Datenblatt (Figure 2 / Figure 15) --- */

/** Reset: Master haelt Bus mindestens 480 us auf Low (tRSTL) */
#define OW_RESET_LOW_US 500U
/** Kurz nach Freigabe: Pull-up muss High zeigen (vor Presence-Puls ab ~15 us) */
#define OW_PRESENCE_PULLUP_SETTLE_US 10U
/** Bis wann nach Freigabe auf den Presence-Puls gewartet wird */
#define OW_PRESENCE_DETECT_END_US 300U
#define OW_PRESENCE_STEP_US 5U
/** Mindest-Recovery nach Reset bevor der naechste Zugriff kommt (tRSTH) */
#define OW_RESET_RECOVERY_US 480U
/** Presence-Sample typisch 60–75 us nach Freigabe (Dallas-Referenz) */
#define OW_PRESENCE_SAMPLE_US 70U

/** Detailliertes Ergebnis von ow_reset() – fuer Display-Diagnose */
typedef enum {
    OW_RESET_NO_PULLUP = 0,  /**< Bus wird nach Reset nicht High (kein Pull-up?) */
    OW_RESET_NO_PRESENCE,    /**< Pull-up OK, aber kein Presence-Puls vom Sensor */
    OW_RESET_PRESENCE        /**< Sensor hat geantwortet */
} OwResetResult_t;

/** Schreib-Timing: Write-1 kurz Low, Write-0 lang Low (je ein Time-Slot) */
#define OW_WRITE_1_LOW_US 6U
#define OW_WRITE_0_LOW_US 65U
#define OW_SLOT_US 70U
#define OW_RECOVERY_US 1U

/** Lese-Timing: Master impulst 1 us Low, samplet innerhalb 15 us */
#define OW_READ_INIT_LOW_US 1U
#define OW_READ_SAMPLE_US 14U

/* --- Standard 1-Wire-ROM-Befehle --- */
#define OW_CMD_SEARCH_ROM 0xF0U
#define OW_CMD_MATCH_ROM  0x55U
#define OW_CMD_SKIP_ROM   0xCCU
#define OW_CMD_READ_ROM   0x33U

/** Ergebnis von ow_wiring_test(): kann PD0 den Bus direkt treiben? (nur Start-Test) */
typedef struct {
    bool pd0_low_ok;
    bool pd0_high_ok;
} OwWiringTest_t;

/** Ergebnis von ow_pullup_diag(): funktioniert der externe Pfad PD1 -> R -> DQ? */
typedef struct {
    bool bus_high_pd1_on;  /**< PD1=High, PD0 frei -> Bus muss High sein */
    bool bus_low_pd1_off;  /**< PD1=Low,  PD0 frei -> Bus muss Low sein */
} OwPullupDiag_t;

void           ow_init(void);
OwWiringTest_t ow_wiring_test(void);
OwPullupDiag_t ow_pullup_diag(void);
bool           ow_bus_read(void);
bool           ow_reset(void);
OwResetResult_t ow_last_reset_result(void);
void           ow_write_bit(uint8_t bit);
uint8_t        ow_read_bit(void);
void           ow_write_byte(uint8_t data);
uint8_t        ow_read_byte(void);
void           ow_strong_pullup_enable(void);
void           ow_strong_pullup_disable(void);

#endif /* AUFGABE4_1WIRE_H */
