/**
 ******************************************************************************
 * @file    display.h
 * @brief   LCD-Ausgabe (nur Werte, statische Labels einmalig).
 ******************************************************************************
 */

#ifndef AUFGABE2_DISPLAY_H
#define AUFGABE2_DISPLAY_H

void display_init(void);
void display_update(double angleDeg, double velDegPerSec);

#endif /* AUFGABE2_DISPLAY_H */
