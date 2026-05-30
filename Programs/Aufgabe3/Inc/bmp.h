#ifndef AUFGABE3_BMP_H
#define AUFGABE3_BMP_H

#include <stdint.h>

#include "BMP_types.h"

/** Teilaufgabe b: zeilenweise Ausgabe ueber GUI_WriteLine(1) statt GUI_drawPoint (0) */
#define BMP_USE_WRITE_LINE 1

#define BMP_MAX_ROW_BYTES 4096u

typedef struct {
    BITMAPFILEHEADER fileHeader;
    BITMAPINFOHEADER infoHeader;
    RGBQUAD palette[MAX_COLOR_TABLE_SIZE];
    uint32_t numColors;
    uint32_t rowSize;
} BmpImageInfo;

int bmp_readAndDisplay(void);

#endif /* AUFGABE3_BMP_H */
