/**
 * @file    bmp_check.c
 * @brief   Plausibilitaetspruefung der BMP-Header vor dem Einlesen der Pixeldaten.
 */

#include "bmp_check.h"
#include "bmp.h"
#include "errorhandler.h"

#include <stddef.h>

/**
 * @brief  Berechnet die Zeilenlaenge in Bytes inklusive 4-Byte-Ausrichtung.
 */
static uint32_t calcRowSize(uint32_t width, uint16_t bitCount) {
    return (((width * (uint32_t)bitCount) + 31u) / 32u) * 4u;
}

int basicChecks(const BITMAPFILEHEADER *fileHeader, const BITMAPINFOHEADER *infoHeader) {
    RETURN_NOK_ON_ERR(fileHeader == NULL || infoHeader == NULL, "basicChecks: null pointer");

    RETURN_NOK_ON_ERR(fileHeader->bfType != BMP_SIGNATURE, "basicChecks: wrong BMP signature");
    RETURN_NOK_ON_ERR(fileHeader->bfReserved1 != 0u || fileHeader->bfReserved2 != 0u,
                      "basicChecks: reserved fields not zero");
    RETURN_NOK_ON_ERR(infoHeader->biSize != 40u, "basicChecks: wrong info header size");
    RETURN_NOK_ON_ERR(infoHeader->biPlanes != 1u, "basicChecks: wrong color planes");
    RETURN_NOK_ON_ERR(infoHeader->biWidth <= 0L || infoHeader->biHeight <= 0L,
                      "basicChecks: invalid image size");
    RETURN_NOK_ON_ERR(infoHeader->biBitCount != 8u && infoHeader->biBitCount != 24u,
                      "basicChecks: unsupported bits per pixel");

    if (infoHeader->biBitCount == 24u) {
        RETURN_NOK_ON_ERR(infoHeader->biCompression != BI_RGB,
                          "basicChecks: 24-bit BMP must be uncompressed");
    } else {
        RETURN_NOK_ON_ERR(infoHeader->biCompression != BI_RGB &&
                              infoHeader->biCompression != BI_RLE8,
                          "basicChecks: unsupported 8-bit compression");
    }

    DWORD paletteBytes = 0u;
    if (infoHeader->biBitCount == 8u) {
        DWORD colors = infoHeader->biClrUsed;
        if (colors == 0u) {
            colors = 256u;
        }
        RETURN_NOK_ON_ERR(colors > MAX_COLOR_TABLE_SIZE, "basicChecks: palette too large");
        paletteBytes = colors * (DWORD)sizeof(RGBQUAD);
    }

    DWORD minOffset = (DWORD)sizeof(BITMAPFILEHEADER) + (DWORD)sizeof(BITMAPINFOHEADER) + paletteBytes;
    RETURN_NOK_ON_ERR(fileHeader->bfOffBits < minOffset, "basicChecks: invalid data offset");

    RETURN_NOK_ON_ERR(calcRowSize((uint32_t)infoHeader->biWidth, infoHeader->biBitCount) > BMP_MAX_ROW_BYTES,
                      "basicChecks: row too large");

    return EOK;
}
