#ifndef AUFGABE3_BUTTON_H
#define AUFGABE3_BUTTON_H

#include <stdbool.h>

/** Initialisiert zusaetzlich den blauen Nucleo-User-Taster (PC13). */
void button_init(void);

/** Wartet auf Taster S1..S6 (PG0..PG5) oder Nucleo-User-Taster. */
void button_waitForPress(void);

bool button_isPressed(void);

#endif /* AUFGABE3_BUTTON_H */
