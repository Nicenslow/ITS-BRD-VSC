/**
 ******************************************************************************
 * @file    display.h
 * @brief   LCD-Ausgabe (nur Werte, statische Labels einmalig).
 ******************************************************************************
 */

#ifndef AUFGABE5_DISPLAY_H
#define AUFGABE5_DISPLAY_H

void display_init(void);
void display_update(double angleDeg, double velDegPerSec);

#endif /* AUFGABE5_DISPLAY_H */
