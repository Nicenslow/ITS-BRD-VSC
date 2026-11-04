/**
 * @file display.h
 * @author Franz Korf, HAW Hamburg
 * @date Mar 2016
 * @brief Zwei Terminals rechts neben der Tastatur.
 */

#ifndef DISPLAY_H
#define DISPLAY_H

#include <stdbool.h>

/* Funktionsnamen der vorgegebenen Anzeige bleiben unveraendert. */
/* NOLINTBEGIN(readability-identifier-naming) */

/*
 ****************************************************************************************
 * @brief Display, Touch und die beiden Terminals initialisieren.
 ****************************************************************************************
 */
void initDisplay(void);

/*
 ****************************************************************************************
 * @brief Ausgabeterminal auf Fehlerfarbe setzen und loeschen.
 ****************************************************************************************
 */
void setErrMode(void);

/*
 ****************************************************************************************
 * @brief Ausgabeterminal auf Normalfarbe setzen und loeschen.
 ****************************************************************************************
 */
void setNormalMode(void);

/*
 ****************************************************************************************
 * @brief Text auf dem oberen Terminal ausgeben.
 *
 * @param msg Auszugebender Text. Ein \\n beginnt eine neue Zeile.
 ****************************************************************************************
 */
void printStdout(char *msg);

/*
 ****************************************************************************************
 * @brief Eine Ziffer auf dem unteren Echo-Terminal ausgeben.
 *
 * @param echo_char Auszugebendes Zeichen.
 ****************************************************************************************
 */
void printToEchoLine(char echo_char);

/*
 ****************************************************************************************
 * @brief Echo-Terminal loeschen.
 ****************************************************************************************
 */
void clearEchoTerm(void);

/*
 ****************************************************************************************
 * @brief Ausgabeterminal loeschen.
 ****************************************************************************************
 */
void clearStdout(void);

/*
 ****************************************************************************************
 * @brief Pruefen, ob die aktuelle Ausgabezeile die letzte Zeile ist.
 *
 * @return true, wenn die aktuelle Zeile die unterste Zeile ist.
 ****************************************************************************************
 */
bool isBottomLine(void);

/* NOLINTEND(readability-identifier-naming) */

#endif /* DISPLAY_H */
