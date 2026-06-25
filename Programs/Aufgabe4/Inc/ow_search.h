/**
 ******************************************************************************
 * @file    ow_search.h
 * @brief   1-Wire Search-Algorithmus (AN187) – alle Sensoren am Bus finden.
 *
 * ow_search_first() startet eine neue Suche, ow_search_next() liefert
 * weitere ROM-Codes, bis alle Geraete gefunden wurden.
 ******************************************************************************
 */

#ifndef AUFGABE4_OW_SEARCH_H
#define AUFGABE4_OW_SEARCH_H

#include <stdbool.h>
#include <stdint.h>

/** Letzter gefundener ROM-Code und Search-State (global laut AN187-Referenz) */
extern uint8_t ROM_NO[8];
extern int     LastDiscrepancy;
extern int     LastFamilyDiscrepancy;
extern bool    LastDeviceFlag;

bool ow_search_first(uint8_t rom[8]);
bool ow_search_next(uint8_t rom[8]);

#endif /* AUFGABE4_OW_SEARCH_H */
