/**
 * @file calc_error.h
 * @author Daniel Biedermann, HAW Hamburg
 * @date Oktober 2026
 * @brief Ausgabe von Fehlermeldungen auf dem oberen Terminal.
 *
 * Der Name ist absichtlich nicht error.h: diese Datei gibt es bereits in
 * ITS_BRD_LIB fuer den LED-Error-Handler.
 */

#ifndef CALC_ERROR_H
#define CALC_ERROR_H

#include "result.h"

/*
 ****************************************************************************************
 * @brief Fehlermeldung rot auf dem Ausgabeterminal anzeigen.
 *
 * RESULT_OK gibt nichts aus.
 *
 * @param err Statuscode der fehlgeschlagenen Operation.
 ****************************************************************************************
 */
void calc_error_print(Result err);

#endif /* CALC_ERROR_H */
