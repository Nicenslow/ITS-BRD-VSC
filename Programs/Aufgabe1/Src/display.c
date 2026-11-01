/**
 * @file display.c
 * @author Franz Korf, HAW Hamburg
 * @date Mar 2016
 * @brief Oberes Terminal fuer Ergebnisse und Fehler, unteres fuer Ziffernecho.
 */

#include "display.h"

#include "LCD_GUI.h"
#include "LCD_Touch.h"
#include "keypad.h"
#include "terminal.h"

#include <stddef.h>

/* Funktionsnamen der vorgegebenen Anzeige bleiben unveraendert. */
/* NOLINTBEGIN(readability-identifier-naming) */

static Terminal stdout_term = INVALID_TERM;
static Terminal echo_term = INVALID_TERM;

void clearStdout(void) { clearTerm(&stdout_term); }

void clearEchoTerm(void) { clearTerm(&echo_term); }

void printStdout(char *msg) { printTerm(msg, &stdout_term); }

void initDisplay(void) {
   Terminal term;
   Coordinate split;

   GUI_init(DEFAULT_BRIGHTNESS);
   TP_Init(false);
   term = makeKeyPad();

   /* Tastaturterminal in Ausgabeflaeche und eine Echozeile teilen. */
   split = term.bottomRight;
   split.y = split.y - term.font->Height - 1;
   stdout_term =
       genTerm(term.topLeft, split, term.bgColor, term.fgColor, term.font);

   split = term.topLeft;
   split.y = term.bottomRight.y - term.font->Height;
   echo_term = genTerm(split, term.bottomRight, BLUE, YELLOW, term.font);

   clearEchoTerm();
   clearStdout();
}

void printToEchoLine(char c) {
   char buf[2] = {'\0', '\0'};
   buf[0] = c;
   printTerm(buf, &echo_term);
}

void setErrMode(void) {
   clearTerm(&stdout_term);
   setTextColorTerm(RED, &stdout_term);
}

void setNormalMode(void) {
   clearTerm(&stdout_term);
   setTextColorTerm(BLACK, &stdout_term);
}

bool isBottomLine(void) {
   return getCurRow(&stdout_term) + 1 == getNoOfRows(&stdout_term);
}

/* NOLINTEND(readability-identifier-naming) */
