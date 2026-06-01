/**
 * @file    main.c
 * @brief   Aufgabe 3: BMP-Empfang per USB-UART und Anzeige auf dem LCD.
 */

#include "init.h"
#include "LCD_GUI.h"
#include "LCD_Touch.h"
#include "lcd.h"

#include "bmp.h"
#include "button.h"
#include "errorhandler.h"
#include "input.h"

/**
 * @brief  Programmstart: Hardware initialisieren, dann Bildschleife mit Taster.
 * @param  Keine
 * @retval 0 (wird bei Endlosschleife nicht erreicht)
 */
int main(void) {
    initITSboard();
    initLCDTouch();
    button_init();
    initInput();

    GUI_clear(BLACK);
    lcdGotoXY(10, 10);
    lcdPrintS("GUI: Start, dann ");
    lcdPrintlnS("blauen User-Button druecken");

    while (1) {
        button_waitForPress();
        GUI_clear(BLACK);

        if (NOK == bmp_readAndDisplay()) {
            /* Fehlermeldung wurde bereits auf dem LCD ausgegeben */
        }
    }
}
