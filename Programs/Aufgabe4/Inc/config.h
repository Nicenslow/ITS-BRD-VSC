/**
 ******************************************************************************
 * @file    config.h
 * @brief   Schalter: welche Teilaufgabe von Aufgabe 4 gebaut wird.
 *
 * Hier wird NUR entschieden, welcher Code-Pfad in main.c aktiv ist.
 * Die gemeinsame Hardware-Schicht (1wire, ds18x20, crc8) bleibt gleich.
 ******************************************************************************
 */

#ifndef AUFGABE4_CONFIG_H
#define AUFGABE4_CONFIG_H

/**
 * Aktive Teilaufgabe (genau EINE zur Compile-Zeit):
 *
 *   1 – Basisfunktion: 1-Wire-Bits/Bytes, Presence, ROM eines Sensors lesen
 *       (Aufgaben-PDF: "Teilaufgabe 1: Basisfunktionalität")
 *
 *   2 – Temperatur messen fuer mehrere Sensoren mit fest eingetragenen ROMs
 *       (ROMs zuerst in Teilaufgabe 1 ermitteln, dann hier eintragen)
 *
 *   3 – Automatische Sensor-Erkennung per Search-Algorithmus + Temperatur
 *       (kombiniert Teilaufgabe 1 und 2 vollstaendig)
 */
/** Hier umstellen: 1, 2 oder 3 – danach neu bauen und flashen */
#define AUFGABE4_TEILAUFGABE 2

#endif /* AUFGABE4_CONFIG_H */
