/**
 ******************************************************************************
 * @file    fsm.c
 * @brief   Finite-State-Machine fuer Quadratur-Drehgeber (ISR-gesteuert).
 ******************************************************************************
 */

#include "fsm.h"

static volatile FsmState_t  s_state        = FSM_PHASE_A;
static volatile int32_t     s_pulseCount   = 0;
static volatile uint32_t    s_lastChangeTs = 0;
static volatile Direction_t s_direction    = DIR_UNKNOWN;
static volatile bool        s_error        = false;
static volatile bool        s_synced       = false;

/**
 * @brief  Vorwaertsuebergang: Zaehler und Zeitstempel aktualisieren.
 * @param  timestamp Zeitstempel des gueltigen Phasenwechsels
 * @retval None
 */
static void applyForward(uint32_t timestamp) {
    s_pulseCount++;
    s_direction    = DIR_FORWARD;
    s_lastChangeTs = timestamp;
}

/**
 * @brief  Rueckwaertsuebergang: Zaehler und Zeitstempel aktualisieren.
 * @param  timestamp Zeitstempel des gueltigen Phasenwechsels
 * @retval None
 */
static void applyBackward(uint32_t timestamp) {
    s_pulseCount--;
    s_direction    = DIR_BACKWARD;
    s_lastChangeTs = timestamp;
}

/**
 * @brief  Verarbeitet eine neue Phasenmessung gemaess Uebergangstabelle.
 * @param  newPhase  Aus GPIO gelesene aktuelle Phase
 * @param  timestamp Zeitstempel der Messung
 * @retval None
 */
static void processPhase(FsmState_t newPhase, uint32_t timestamp) {
    if (!s_synced) {
        s_state  = newPhase;
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
 * @brief  Setzt die FSM in den Ausgangszustand zurueck (ohne Fehler).
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
 * @brief  Synchronisiert die FSM auf die aktuelle Hardware-Phase (Init).
 * @param  currentPhase  aktuell gemessene Phase am Drehgeber
 * @retval None
 */
void fsm_syncToPhase(FsmState_t currentPhase) {
    s_state  = currentPhase;
    s_synced = true;
}

/**
 * @brief  Verarbeitet einen Phasenwechsel aus der EXTI-ISR.
 * @param  newPhase  Aus GPIO gelesene aktuelle Phase
 * @param  timestamp Zeitstempel direkt nach dem Flankenwechsel
 * @retval None
 */
void fsm_isrUpdate(FsmState_t newPhase, uint32_t timestamp) {
    processPhase(newPhase, timestamp);
}

/**
 * @brief  Loescht den Fehlerzustand und synchronisiert auf die aktuelle Phase.
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
 * @brief  Liest Zaehler und Zeitstempel konsistent (Doppellesen gegen ISR-Races).
 * @param  pulseCount    Ausgabe: Impulszaehler
 * @param  lastChangeTs  Ausgabe: Zeitstempel des letzten Phasenwechsels
 * @retval true bei erfolgreichem konsistenten Lesen
 */
bool fsm_getSnapshot(int32_t *pulseCount, uint32_t *lastChangeTs) {
    volatile int32_t  pc1;
    volatile int32_t  pc2;
    volatile uint32_t ts1;
    volatile uint32_t ts2;

    do {
        ts1 = s_lastChangeTs;
        pc1 = s_pulseCount;
        ts2 = s_lastChangeTs;
        pc2 = s_pulseCount;
    } while ((pc1 != pc2) || (ts1 != ts2));

    *pulseCount   = pc1;
    *lastChangeTs = ts1;
    return true;
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
