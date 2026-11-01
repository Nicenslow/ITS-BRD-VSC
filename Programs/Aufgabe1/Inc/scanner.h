/**
 * @file scanner.h
 * @author Franz Korf, HAW Hamburg
 * @date Mar 2016
 * @brief Scanner fuer die Tastatureingabe des RPN-Rechners.
 */

#ifndef SCANNER_H
#define SCANNER_H

#include "token.h"

/* nextToken ist der Name der vorgegebenen Schnittstelle. */
/* NOLINTBEGIN(readability-identifier-naming) */

/*
 ****************************************************************************************
 * @brief Naechstes Token von der Tastatur lesen.
 *
 * Ziffern werden zu einer nichtnegativen Zahl zusammengefasst. Die Enter-Taste
 * trennt aufeinanderfolgende Zahlen und wird nicht an den Aufrufer gegeben.
 * Passt die Zahl nicht in einen signed int, ist das Token OVERFLOW.
 *
 * @return Naechstes Token. Bei einer unbekannten Taste UNEXPECTED.
 ****************************************************************************************
 */
T_token nextToken(void);

/* NOLINTEND(readability-identifier-naming) */

#endif /* SCANNER_H */
