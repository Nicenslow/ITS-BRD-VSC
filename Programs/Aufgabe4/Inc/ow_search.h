/**
 ******************************************************************************
 * @file    ow_search.h
 * @brief   1-Wire Search Algorithm gemaess Maxim Application Note 187.
 ******************************************************************************
 */

#ifndef AUFGABE4_OW_SEARCH_H
#define AUFGABE4_OW_SEARCH_H

#include <stdbool.h>
#include <stdint.h>

/** @brief Anzahl Bits im 64-Bit-ROM-Code */
#define OW_SEARCH_ROM_BITS 64U

/** @brief Globaler Search-State gemaess AN187 (nur dieses Modul) */
extern uint8_t ROM_NO[8];
extern int     LastDiscrepancy;
extern int     LastFamilyDiscrepancy;
extern bool    LastDeviceFlag;

bool ow_search_first(uint8_t rom[8]);
bool ow_search_next(uint8_t rom[8]);

#endif /* AUFGABE4_OW_SEARCH_H */
