/**
 * @file format.c
 * @author Daniel Biedermann, HAW Hamburg
 * @date Oktober 2026
 * @brief Dezimalausgabe ohne sprintf.
 */

#include "format.h"

#define DECIMAL_BASE 10

void int_to_string(int value, char *buf) {
   unsigned int magnitude;
   int idx = 0;
   int left;
   int right;

   if (value < 0) {
      /* Betrag von INT_MIN ohne signed overflow bilden. */
      magnitude = (unsigned int)(-(value + 1)) + 1U;
   } else {
      magnitude = (unsigned int)value;
   }

   do {
      unsigned int digit = magnitude % DECIMAL_BASE;
      buf[idx] = (char)('0' + digit);
      idx++;
      magnitude /= DECIMAL_BASE;
   } while (magnitude > 0U);

   if (value < 0) {
      buf[idx] = '-';
      idx++;
   }

   buf[idx] = '\0';

   for (left = 0, right = idx - 1; left < right; left++, right--) {
      char tmp = buf[left];
      buf[left] = buf[right];
      buf[right] = tmp;
   }
}
