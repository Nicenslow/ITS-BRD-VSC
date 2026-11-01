/**
 * @file scanner.c
 * @author Franz Korf, HAW Hamburg
 * @date Mar 2016
 * @brief Scanner fuer die Tastatur des ITS-Boards.
 *
 * Ziffernfolgen werden vor dem Weiterrechnen auf den Wertebereich von int
 * geprueft. Die Enter-Taste beendet eine Zahl und wird nicht nach aussen
 * gegeben.
 */

#include "scanner.h"

#include "display.h"
#include "keypad.h"

#include <limits.h>

/* nextToken ist der Name der vorgegebenen Schnittstelle. */
/* NOLINTBEGIN(readability-identifier-naming) */

#define DECIMAL_BASE 10

static char next_char(void) { return getKeyPadInput(); }

T_token nextToken(void) {
   /* Ein Zeichen Vorausschau, damit das Ende einer Zahl erkannt wird. */
   static char lookahead = ENTER;
   T_token result = {UNEXPECTED, 0};

   clearEchoTerm();
   while (lookahead == ENTER) {
      lookahead = next_char();
   }

   switch (lookahead) {
   case PLUS:
   case MINUS:
   case MULT:
   case DIV:
   case PRT:
   case SWAP:
   case PRT_ALL:
   case DOUBLE:
   case CLEAR:
      result.tok = lookahead;
      lookahead = ENTER;
      break;

   case '0':
   case '1':
   case '2':
   case '3':
   case '4':
   case '5':
   case '6':
   case '7':
   case '8':
   case '9':
      result.tok = NUMBER;
      while ((lookahead >= '0') && (lookahead <= '9')) {
         int digit = lookahead - '0';
         printToEchoLine(lookahead);
         if (result.val > (INT_MAX - digit) / DECIMAL_BASE) {
            result.tok = OVERFLOW;
            result.val = 0;
            lookahead = ENTER;
            return result;
         }
         result.val = (result.val * DECIMAL_BASE) + digit;
         lookahead = next_char();
      }
      break;

   default:
      lookahead = ENTER;
      break;
   }

   return result;
}

/* NOLINTEND(readability-identifier-naming) */
