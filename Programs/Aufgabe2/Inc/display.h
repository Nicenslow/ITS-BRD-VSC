/**
 * @file display.h
 * @author Daniel Biedermann, HAW Hamburg
 * @date November 2026
 * @brief LCD-Ausgabe, pro Super-Loop hoechstens ein Zeichen.
 */

#ifndef DISPLAY_H
#define DISPLAY_H

#include <stdbool.h>

void display_init(void);
void display_submit(double angle_deg, double velocity_deg_per_sec);
bool display_is_busy(void);

/* Gibt das naechste Zeichen aus. Ein Durchlauf schreibt hoechstens eines. */
void display_pump(void);

#endif /* DISPLAY_H */
