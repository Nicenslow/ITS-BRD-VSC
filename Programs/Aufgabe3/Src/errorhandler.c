/**
 * @file    errorhandler.c
 * @brief   Fehlerbehandlung mit Anzeige auf dem LCD.
 */

#include <errorhandler.h>

#include "lcd.h"
#include <stdio.h>

#define BUF_SIZE     256
static char buf[BUF_SIZE];

/**
 * @brief  Extrahiert den Dateinamen aus einem Pfad (ohne Verzeichnis).
 * @param  path Vollstaendiger oder relativer Dateipfad
 * @retval Zeiger auf den Dateinamen innerhalb von path
 */
static const char *fileBasename(const char *path) {
    const char *base = path;

    for (const char *p = path; *p != '\0'; p++) {
        if (*p == '/' || *p == '\\') {
            base = p + 1;
        }
    }
    return base;
}

/**
 * @brief  Zeigt bei cnd==true eine Fehlermeldung auf dem LCD an.
 * @param  cnd         true loest die Fehleranzeige aus
 * @param  file        Quelldatei (__FILE__ aus Makro)
 * @param  line        Zeilennummer (__LINE__ aus Makro)
 * @param  msg         Fehlertext
 * @param  loopForEver true: Programm haengt in Endlosschleife
 * @retval EOK wenn kein Fehler, sonst NOK
 */
int printError(bool cnd, char *file, int line, char *msg, bool loopForEver) {
    if (cnd) {
        lcdGotoXY(5, 5);
        snprintf(buf, sizeof(buf) / sizeof(buf[0]), "%s:%d", fileBasename(file), line);
        lcdPrintS(buf);
        lcdPrintlnS(msg);
        while (loopForEver) {
        }
        return NOK;
    }
    return EOK;
}
