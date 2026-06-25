/**
 ******************************************************************************
 * @file    crc8.h
 * @brief   1-Wire CRC-8 gemaess Maxim Application Note 27.
 *
 * ROM:     CRC ueber Bytes 0..6, Ergebnis muss Byte 7 entsprechen.
 * Scratch: CRC ueber Bytes 0..7, Ergebnis muss Byte 8 entsprechen.
 ******************************************************************************
 */

#ifndef AUFGABE4_CRC8_H
#define AUFGABE4_CRC8_H

#include <stdint.h>

#define CRC8_ROM_DATA_LEN        7U
#define CRC8_SCRATCHPAD_DATA_LEN 8U

uint8_t crc8_update(uint8_t crc, uint8_t data);
uint8_t crc8_buf(const uint8_t *buf, uint8_t len);

#endif /* AUFGABE4_CRC8_H */
