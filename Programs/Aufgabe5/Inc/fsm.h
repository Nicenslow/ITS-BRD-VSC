/**
 ******************************************************************************
 * @file    fsm.h
 * @brief   Finite-State-Machine fuer Quadratur-Drehgeber (Phasenzustaende).
 ******************************************************************************
 */

#ifndef AUFGABE5_FSM_H
#define AUFGABE5_FSM_H

#include <stdbool.h>
#include <stdint.h>

typedef enum {
    FSM_PHASE_A,
    FSM_PHASE_B,
    FSM_PHASE_C,
    FSM_PHASE_D
} FsmState_t;

typedef enum {
    DIR_UNKNOWN,
    DIR_FORWARD,
    DIR_BACKWARD
} Direction_t;

void        fsm_init(void);
void        fsm_isrUpdate(FsmState_t newPhase, uint32_t timestamp);
void        fsm_syncToPhase(FsmState_t currentPhase);
void        fsm_resetError(FsmState_t currentPhase);
int32_t     fsm_getPulseCount(void);
uint32_t    fsm_getLastChangeTimestamp(void);
bool        fsm_getSnapshot(int32_t *pulseCount, uint32_t *lastChangeTs);
Direction_t fsm_getDirection(void);
bool        fsm_hasError(void);

#endif /* AUFGABE5_FSM_H */
