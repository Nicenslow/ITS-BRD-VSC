#ifndef _ERRORHANDLER_H
#define _ERRORHANDLER_H

#include <stdbool.h>

#define NOK  -1
#define EOK   0

/**
 * @brief  Fehlerbehandlung mit LCD-Ausgabe (nicht direkt aufrufen, Makros nutzen).
 * @param  cnd         Bedingung: true loest Fehler aus
 * @param  file        Quelldatei
 * @param  line        Zeilennummer
 * @param  msg         Meldungstext
 * @param  loopForEver true haengt das Programm
 * @retval EOK oder NOK
 */
extern int printError(bool cnd, char *file, int line, char *msg, bool loopForEver);

/** Fehler melden und in Endlosschleife bleiben */
#define LOOP_ON_ERR(cnd, msg) printError((cnd), __FILE__, __LINE__, (msg), true)

/** Fehler melden und EOK/NOK zurueckgeben */
#define ERR_HANDLER(cnd, msg) printError((cnd), __FILE__, __LINE__, (msg), false)

/** Bei Fehler NOK aus der aufrufenden Funktion zurueckgeben */
#define RETURN_NOK_ON_ERR(cnd, msg) \
    {                               \
        if (NOK == ERR_HANDLER(cnd, msg)) { \
            return NOK;             \
        }                           \
    }

/** Bei NOK-Rueckgabe einer Unterfunktion abbrechen */
#define RAISE_NOK(fcall) \
    {                    \
        if (NOK == (fcall)) { \
            return NOK;  \
        }                \
    }

#endif /* _ERRORHANDLER_H */
