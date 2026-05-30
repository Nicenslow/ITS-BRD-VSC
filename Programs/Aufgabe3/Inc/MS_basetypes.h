/*
 * MS_basetypes.h
 *
 *  Created on: 21.10.2016
 *      Author: Franz Korf
 */

#ifndef _MS_BASETYPES_H
#define _MS_BASETYPES_H

#include <stdint.h>
/*
 * Original data, must be adapted for 64 Bit machines
 *
   typedef char                CHAR;
   typedef short               SHORT;
   typedef long                LONG;
   typedef unsigned long       DWORD;
   typedef int                 BOOL;
   typedef unsigned char       BYTE;
   typedef unsigned short      WORD;
*/

typedef int8_t 				CHAR;
typedef int16_t 			SHORT;
typedef int32_t 			LONG;
typedef uint32_t 			DWORD;
typedef int32_t 			BOOL;
typedef uint8_t 			BYTE;
typedef uint16_t 			WORD;

#endif /* _MS_BASETYPES_H */
