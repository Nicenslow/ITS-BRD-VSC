/**
 * @file format.h
 * @author Daniel Biedermann, HAW Hamburg
 * @date Oktober 2026
 * @brief Umwandlung von int-Werten in Dezimaltext.
 */

#ifndef FORMAT_H
#define FORMAT_H

/* Platz fuer den laengsten int-Text inklusive Abschlussnull, z. B.
 * "-2147483648". */
#define INT_STR_BUF_SIZE 16

/*
 ****************************************************************************************
 * @brief Einen int-Wert in einen dezimalen, nullterminierten String schreiben.
 *
 * Auch negative Werte und INT_MIN werden ausgegeben.
 *
 * @param value Umzuwandelnder Wert.
 * @param buf Zielpuffer mit mindestens INT_STR_BUF_SIZE Bytes.
 ****************************************************************************************
 */
void int_to_string(int value, char *buf);

#endif /* FORMAT_H */
