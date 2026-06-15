/**
 ******************************************************************************
 * @file    1wire.h
 * @brief   Grundlegende 1-Wire-Bus-Operationen (PD0 Daten, PD1 Versorgung).
 ******************************************************************************
 */

#ifndef AUFGABE4_1WIRE_H
#define AUFGABE4_1WIRE_H

#include <stdbool.h>
#include <stdint.h>

/** @brief 1-Wire-Datenleitung (Bus) */
#define OW_PIN_DATA GPIO_PIN_0
/** @brief Parasitaere Stromversorgung */
#define OW_PIN_POWER GPIO_PIN_1
/** @brief GPIO-Port fuer 1-Wire-Pins */
#define OW_GPIO_PORT GPIOD

/** @brief Reset-Puls: mindestens 480 us */
#define OW_RESET_LOW_US 500U
/** @brief Presence-Fenster nach Bus-Freigabe (us) */
#define OW_PRESENCE_WINDOW_US 500U
/** @brief Wartezeit vor erster Presence-Abfrage (us) */
#define OW_PRESENCE_FIRST_US 70U
/** @brief Schrittweite beim Abfragen des Presence-Pulses */
#define OW_PRESENCE_STEP_US 10U

/** @brief Restzeit nach Presence bis Ende Reset-Zyklus */
#define OW_RESET_RECOVERY_US 480U

/** @brief Write-1: Low-Zeit 1–15 us */
#define OW_WRITE_1_LOW_US 6U
/** @brief Write-0: Low-Zeit 60–120 us */
#define OW_WRITE_0_LOW_US 65U
/** @brief Mindestlaenge eines Time-Slots */
#define OW_SLOT_US 60U
/** @brief Recovery zwischen Time-Slots */
#define OW_RECOVERY_US 1U

/** @brief Read: Master zieht 1 us Low */
#define OW_READ_INIT_LOW_US 1U
/** @brief Read: Sample innerhalb 15 us nach Flanke */
#define OW_READ_SAMPLE_US 14U

/** @brief ROM-Kommando Search ROM */
#define OW_CMD_SEARCH_ROM 0xF0U
/** @brief ROM-Kommando Match ROM */
#define OW_CMD_MATCH_ROM 0x55U
/** @brief ROM-Kommando Skip ROM */
#define OW_CMD_SKIP_ROM 0xCCU
/** @brief ROM-Kommando Read ROM */
#define OW_CMD_READ_ROM 0x33U

typedef struct {
    bool pd0_low_ok;
    bool pd0_high_ok;
} OwWiringTest_t;

void           ow_init(void);
OwWiringTest_t ow_wiring_test(void);
bool           ow_bus_read(void);
bool    ow_reset(void);
void    ow_write_bit(uint8_t bit);
uint8_t ow_read_bit(void);
void    ow_write_byte(uint8_t data);
uint8_t ow_read_byte(void);
void    ow_strong_pullup_enable(void);
void    ow_strong_pullup_disable(void);

#endif /* AUFGABE4_1WIRE_H */
