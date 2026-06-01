#ifndef AUFGABE3_BMP_H
#define AUFGABE3_BMP_H

#include <stdint.h>

#include "BMP_types.h"

/** 1: zeilenweise mit GUI_WriteLine (Teilaufgabe b), 0: pixelweise mit GUI_drawPoint (a) */
#define BMP_USE_WRITE_LINE 1

/** Maximale Zeilenlaenge in Bytes (Puffergroesse) */
#define BMP_MAX_ROW_BYTES 4096u

/** Gelesene BMP-Kopfdaten und abgeleitete Werte fuer die Dekodierung */
typedef struct {
    BITMAPFILEHEADER fileHeader;
    BITMAPINFOHEADER infoHeader;
    RGBQUAD palette[MAX_COLOR_TABLE_SIZE];
    uint32_t numColors;
    uint32_t rowSize;
} BmpImageInfo;

/**
 * @brief  Empfaengt und zeigt die naechste BMP-Datei vom PC.
 * @param  Keine
 * @retval EOK Erfolg
 * @retval NOK Fehler beim Lesen oder Anzeigen
 */
int bmp_readAndDisplay(void);

#endif /* AUFGABE3_BMP_H */
