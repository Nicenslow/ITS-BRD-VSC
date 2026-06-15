/**
 ******************************************************************************
 * @file    crc8.h
 * @brief   1-Wire CRC-8 (Polynom X^8 + X^5 + X^4 + 1, Lookup-Tabelle AN27).
 ******************************************************************************
 */

#ifndef AUFGABE4_CRC8_H
#define AUFGABE4_CRC8_H

#include <stdint.h>

/** @brief Laenge des 1-Wire ROM-Codes ohne CRC-Byte */
#define CRC8_ROM_DATA_LEN 7U

/** @brief Laenge des Scratchpads ohne CRC-Byte */
#define CRC8_SCRATCHPAD_DATA_LEN 8U

uint8_t crc8_update(uint8_t crc, uint8_t data);
uint8_t crc8_buf(const uint8_t *buf, uint8_t len);

#endif /* AUFGABE4_CRC8_H */
