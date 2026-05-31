/**
 ******************************************************************************
 * @file    display.c
 * @brief   LCD: einmalige Labels, danach nur Zahlen mit lcdPrintReplS.
 ******************************************************************************
 */

#include "display.h"

#include <stdint.h>
#include <stdio.h>

#include "lcd.h"

/** @brief feste Schriftgroesse fuer die Anzeige */
#define DISPLAY_FONT_POINTS 12

/** @brief Textpositionen in Zeichenkoordinaten, nicht in Pixeln. */
#define LABEL_X ((POINT)1)
#define VALUE_X ((POINT)20)
#define ROW_ANGLE_Y ((POINT)3)
#define ROW_VEL_Y ((POINT)5)

/** @brief Puffergroesse fuer Zahlenstrings */
#define DISPLAY_VALUE_BUF_LEN 24
/** @brief Breite des Winkel-Vorkommateils fuer feste Ausgabe mit einer Nachkommastelle. */
#define DISPLAY_ANGLE_WHOLE_FIELD_WIDTH 4
/** @brief Breite des Geschwindigkeitsfelds, damit alte Ziffern sicher ueberschrieben werden. */
#define DISPLAY_VELOCITY_FIELD_WIDTH 4

static int32_t s_lastAngleTenths  = INT32_MIN;
static int32_t s_lastVelDegPerSec = INT32_MIN;

static int32_t roundToInt32(double value) {
    return (value >= 0.0) ? (int32_t)(value + 0.5) : (int32_t)(value - 0.5);
}

static void formatTenths(char buf[], unsigned int bufLen, int32_t tenths) {
    int32_t whole = tenths / 10;
    int32_t frac  = tenths % 10;

    if (frac < 0) {
        frac = -frac;
    }

    (void)snprintf(buf, bufLen, "%*ld.%ld",
                   DISPLAY_ANGLE_WHOLE_FIELD_WIDTH,
                   (long)whole,
                   (long)frac);
}

/**
 * @brief  Zeichnet statische Beschriftungen genau einmal.
 * @param  None
 * @retval None
 */
void display_init(void) {
    lcdSetFont(DISPLAY_FONT_POINTS);

    lcdGotoXY(LABEL_X, ROW_ANGLE_Y);
    lcdPrintS("Winkel (Grad):");

    lcdGotoXY(LABEL_X, ROW_VEL_Y);
    lcdPrintS("Geschw. (Grad/s):");
}

/**
 * @brief  Aktualisiert nur geaenderte Zahlenfelder.
 * @param  angleDeg      aktueller Winkel
 * @param  velDegPerSec  aktuelle Winkelgeschwindigkeit
 * @retval None
 */
void display_update(double angleDeg, double velDegPerSec) {
    int32_t iAngleTenths = roundToInt32(angleDeg * 10.0);
    int32_t iVel         = roundToInt32(velDegPerSec);

    if (iAngleTenths == s_lastAngleTenths && iVel == s_lastVelDegPerSec) {
        return;
    }

    char buf[DISPLAY_VALUE_BUF_LEN];

    if (iAngleTenths != s_lastAngleTenths) {
        formatTenths(buf, sizeof(buf), iAngleTenths);
        lcdGotoXY(VALUE_X, ROW_ANGLE_Y);
        lcdPrintReplS(buf);
        s_lastAngleTenths = iAngleTenths;
    }

    if (iVel != s_lastVelDegPerSec) {
        (void)snprintf(buf, sizeof(buf), "%*ld", DISPLAY_VELOCITY_FIELD_WIDTH, (long)iVel);
        lcdGotoXY(VALUE_X, ROW_VEL_Y);
        lcdPrintReplS(buf);
        s_lastVelDegPerSec = iVel;
    }
}
