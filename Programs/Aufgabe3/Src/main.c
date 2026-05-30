/**
 ******************************************************************************
 * @file    main.c
 * @brief   Aufgabe 3: BMP-Empfang via USB-UART und Anzeige auf dem LCD.
 ******************************************************************************
 */

#include "init.h"
#include "LCD_GUI.h"
#include "LCD_Touch.h"
#include "lcd.h"

#include "bmp.h"
#include "button.h"
#include "errorhandler.h"
#include "input.h"

int main(void) {
    initITSboard();
    initLCDTouch();
    button_init();
    initInput();

    GUI_clear(BLACK);
    lcdGotoXY(10, 10);
    lcdPrintS("GUI: Start, dann S6");

    while (1) {
        button_waitForPress();
        GUI_clear(BLACK);

        if (NOK == bmp_readAndDisplay()) {
            /* Fehlertext steht bereits oben links (printError) */
        }
    }
}
