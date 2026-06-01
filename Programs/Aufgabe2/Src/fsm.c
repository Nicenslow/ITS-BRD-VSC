/**
 ******************************************************************************
 * @file    fsm.c
 * @brief   Finite-State-Machine fuer Quadratur-Drehgeber.
 ******************************************************************************
 */

#include "fsm.h"

static FsmState_t s_state          = FSM_PHASE_A;
static int32_t    s_pulseCount     = 0;
static uint32_t   s_lastChangeTs   = 0;
static Direction_t s_direction     = DIR_UNKNOWN;
static bool       s_error          = false;
static bool       s_synced         = false;

/**
 * @brief  Vorwaertsuebergang: Zaehler und Zeitstempel aktualisieren.
 * @param  timestamp Zeitstempel des Gueltigen Phasenwechsels
 * @retval None
 */
static void applyForward(uint32_t timestamp) {
    s_pulseCount++;
    s_direction         = DIR_FORWARD;
    s_lastChangeTs      = timestamp;
}

/**
 * @brief  Rueckwaertsuebergang: Zaehler und Zeitstempel aktualisieren.
 * @param  timestamp Zeitstempel des Gueltigen Phasenwechsels
 * @retval None
 */
static void applyBackward(uint32_t timestamp) {
    s_pulseCount--;
    s_direction         = DIR_BACKWARD;
    s_lastChangeTs      = timestamp;
}

/**
 * @brief  Setzt die FSM in den Ausgangszustand A zurueck (ohne Fehler).
 * @param  None
 * @retval None
 */
void fsm_init(void) {
    s_state        = FSM_PHASE_A;
    s_pulseCount   = 0;
    s_lastChangeTs = 0;
    s_direction    = DIR_UNKNOWN;
    s_error        = false;
    s_synced       = false;
}

/**
 * @brief  Verarbeitet eine neue Phasenmessung gemaess Uebergangstabelle.
 * @param  newPhase  Aus GPIO gelesene aktuelle Phase
 * @param  timestamp Zeitstempel der Messung
 * @retval None
 */
void fsm_update(FsmState_t newPhase, uint32_t timestamp) {
    if (!s_synced) {
        s_state = newPhase;
        s_synced = true;
        return;
    }

    uint32_t delta = ((uint32_t)newPhase + 4u - (uint32_t)s_state) & 3u;

    if (delta == 0u) {
        return;
    }

    if (delta == 1u) {
        applyForward(timestamp);
    } else if (delta == 3u) {
        applyBackward(timestamp);
    } else if (s_direction == DIR_FORWARD) {
        s_error = true;
        applyForward(timestamp);
        applyForward(timestamp);
    } else if (s_direction == DIR_BACKWARD) {
        s_error = true;
        applyBackward(timestamp);
        applyBackward(timestamp);
    } else {
        s_state = newPhase;
        return;
    }

    s_state = newPhase;
}

/**
 * @brief  Loescht den Fehlerzustand und synchronisiert auf die aktuelle Phase
 *         (z. B. nach Fortsetzen per Reset-Taste).
 * @param  currentPhase  aktuell gemessene Phase am Drehgeber
 * @retval None
 */
void fsm_resetError(FsmState_t currentPhase) {
    s_error  = false;
    s_state  = currentPhase;
    s_synced = true;
}

/**
 * @brief  Liefert den Phasenimpulszaehler (signed).
 * @param  None
 * @retval Aktueller Impulszaehlerstand
 */
int32_t fsm_getPulseCount(void) {
    return s_pulseCount;
}

/**
 * @brief  Zeitstempel des letzten gueltigen Vor-/Rueckwaerts-Schritts.
 * @param  None
 * @retval Zeitstempel (TIM2-Ticks)
 */
uint32_t fsm_getLastChangeTimestamp(void) {
    return s_lastChangeTs;
}

/**
 * @brief  Zuletzt erkannte Drehrichtung.
 * @param  None
 * @retval Vorwaerts, Rueckwaerts oder unbekannt
 */
Direction_t fsm_getDirection(void) {
    return s_direction;
}

/**
 * @brief  Meldet ob ein Phasensprung-Fehler aktiv ist.
 * @param  None
 * @retval true wenn Fehler, sonst false
 */
bool fsm_hasError(void) {
    return s_error;
}
