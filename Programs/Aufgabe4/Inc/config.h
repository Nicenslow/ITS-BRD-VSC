/**
 ******************************************************************************
 * @file    config.h
 * @brief   Welche Teilaufgabe von Aufgabe 4 aktiv ist.
 *
 * Ueber AUFGABE4_TEILAUFGABE wird zur Compile-Zeit entschieden, welche
 * run_teilaufgabeX()-Funktion in main.c eingebunden wird.
 ******************************************************************************
 */

#ifndef AUFGABE4_CONFIG_H
#define AUFGABE4_CONFIG_H

/**
 * Aktive Teilaufgabe (nur eine gleichzeitig):
 *   1 = Ein Sensor, ROM lesen und auf Display anzeigen (Diagnose-Modus)
 *   2 = Mehrere Sensoren mit fest eingetragenen ROM-Codes messen
 *   3 = Alle Sensoren per Search-Algorithmus finden und messen (Standard)
 */
#ifndef AUFGABE4_TEILAUFGABE
#define AUFGABE4_TEILAUFGABE 3
#endif

#endif /* AUFGABE4_CONFIG_H */
