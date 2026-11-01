/**
 * @file token.h
 * @author Franz Korf, HAW Hamburg
 * @date Mar 2016
 * @brief Token, die der Scanner an den Rechner liefert.
 */

#ifndef TOKEN_H
#define TOKEN_H

/* Vorgegebene Tokentexte bleiben in dieser Schreibweise. */
/* NOLINTBEGIN(readability-identifier-naming) */

/* Token fuer Eingaben und Stackbefehle. */
#define NUMBER 'n'     //!< eingegebene int-Zahl
#define PLUS '+'       //!< Operator +
#define MINUS '-'      //!< Operator -
#define MULT '*'       //!< Operator *
#define DIV '/'        //!< Operator /
#define PRT 'p'        //!< obersten Wert ausgeben
#define SWAP 'r'       //!< die beiden obersten Werte tauschen
#define PRT_ALL 'P'    //!< die drei obersten Werte ausgeben
#define CLEAR 'C'      //!< Stack loeschen
#define DOUBLE 'd'     //!< obersten Wert duplizieren
#define ENTER ' '      //!< Enter-Taste, wird vom Scanner verbraucht
#define UNEXPECTED 'U' //!< unerwartete Taste
#define OVERFLOW 'O'   //!< die eingegebene Zahl passt nicht in einen int

/* Ein Token. val gilt nur fuer NUMBER. */
typedef struct {
   char tok;
   int val;
} T_token;

/* NOLINTEND(readability-identifier-naming) */

#endif /* TOKEN_H */
