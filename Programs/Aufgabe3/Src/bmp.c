/**
 * @file    bmp.c
 * @brief   BMP ueber UART einlesen, dekodieren und auf dem LCD anzeigen.
 */

#include "bmp.h"

#include <stdio.h>
#include <string.h>

#include "bmp_check.h"
#include "errorhandler.h"
#include "input.h"
#include "LCD_GUI.h"

/**
 * @brief  Wandelt 8-Bit-RGB in die 16-Bit-Farbe des LCD (Format RGB565).
 * @param  red   Rotanteil (0..255)
 * @param  green Gruenanteil (0..255)
 * @param  blue  Blauanteil (0..255)
 * @retval LCD-Farbwert
 */
static COLOR rgbToColor(unsigned char red, unsigned char green, unsigned char blue) {
    return (COLOR)((((uint16_t)red & 0xF8u) << 8) |
                   (((uint16_t)green & 0xFCu) << 3) |
                   ((uint16_t)blue >> 3));
}

/**
 * @brief  Holt eine Farbe aus der 8-Bit-Palette per Index.
 * @param  palette Farbtabelle aus dem BMP-Header
 * @param  index   Palettenindex eines Pixels
 * @retval LCD-Farbwert
 */
static COLOR paletteToColor(const RGBQUAD *palette, uint8_t index) {
    return rgbToColor(palette[index].rgbRed, palette[index].rgbGreen, palette[index].rgbBlue);
}

/**
 * @brief  Berechnet die Zeilenlaenge in Bytes inklusive 4-Byte-Ausrichtung.
 * @param  width    Bildbreite in Pixeln
 * @param  bitCount Bits pro Pixel (8 oder 24)
 * @retval Zeilengroesse in Bytes
 */
static uint32_t calcRowSize(uint32_t width, uint16_t bitCount) {
    return (((width * (uint32_t)bitCount) + 31u) / 32u) * 4u;
}

/**
 * @brief  Liest exakt count Elemente vom UART in den Puffer.
 * @param  buf       Zielpuffer
 * @param  elemSize  Groesse eines Elements in Bytes
 * @param  count     Anzahl der Elemente
 * @retval EOK bei Erfolg, sonst NOK
 */
static int readBlock(void *buf, unsigned int elemSize, unsigned int count) {
    int ret = COMread((char *)buf, elemSize, count);
    return (ret == (int)count) ? EOK : NOK;
}

/**
 * @brief  Verwirft count Bytes aus dem aktuellen Dateistream.
 * @param  count Anzahl der zu ueberspringenden Bytes
 * @retval EOK bei Erfolg, NOK bei vorzeitigem EOF
 */
static int skipBytes(DWORD count) {
    for (DWORD i = 0u; i < count; i++) {
        if (EOF == nextChar()) {
            return NOK;
        }
    }
    return EOK;
}

/**
 * @brief  Liest BMP-Datei- und Info-Header, Palette und springt zu den Pixeldaten.
 * @param  info Struktur fuer Header, Palette und berechnete Zeilenlaenge
 * @retval EOK bei Erfolg, NOK bei Lese- oder Validierungsfehler
 */
static int readHeaders(BmpImageInfo *info) {
    RETURN_NOK_ON_ERR(NOK == readBlock(&info->fileHeader, sizeof(BITMAPFILEHEADER), 1u),
                      "readHeaders: file header read failed");
    RETURN_NOK_ON_ERR(NOK == readBlock(&info->infoHeader, sizeof(BITMAPINFOHEADER), 1u),
                      "readHeaders: info header read failed");

    RAISE_NOK(basicChecks(&info->fileHeader, &info->infoHeader));

    info->numColors = 0u;
    if (info->infoHeader.biBitCount == 8u) {
        info->numColors = info->infoHeader.biClrUsed;
        if (info->numColors == 0u) {
            info->numColors = 256u;
        }
        RETURN_NOK_ON_ERR(NOK == readBlock(info->palette, (unsigned int)sizeof(RGBQUAD),
                                           info->numColors),
                          "readHeaders: palette read failed");
    }

    DWORD headerEnd = (DWORD)sizeof(BITMAPFILEHEADER) + (DWORD)sizeof(BITMAPINFOHEADER) +
                      info->numColors * (DWORD)sizeof(RGBQUAD);
    if (info->fileHeader.bfOffBits > headerEnd) {
        RETURN_NOK_ON_ERR(NOK == skipBytes(info->fileHeader.bfOffBits - headerEnd),
                          "readHeaders: skip to pixel data failed");
    }

    info->rowSize = calcRowSize((uint32_t)info->infoHeader.biWidth, info->infoHeader.biBitCount);
    RETURN_NOK_ON_ERR(info->rowSize > BMP_MAX_ROW_BYTES, "readHeaders: row too large");

    return EOK;
}

#if !BMP_USE_WRITE_LINE
/**
 * @brief  Zeichnet ein Pixel (Teilaufgabe a).
 * @param  x     X-Koordinate
 * @param  y     Y-Koordinate
 * @param  color LCD-Farbe
 */
static void drawPixel(LENGTH x, LENGTH y, COLOR color) {
    Coordinate crd = {x, y};
    if (VALID_COORDINATE(crd)) {
        GUI_drawPoint(crd, color, DOT_PIXEL_1X1, DOT_FILL_AROUND);
    }
}
#endif

#if BMP_USE_WRITE_LINE
/**
 * @brief  Zeichnet eine Pixelzeile auf einmal (Teilaufgabe b).
 * @param  x      X-Startkoordinate
 * @param  y      Y-Koordinate
 * @param  colors Farbpuffer der Zeile
 * @param  width  Anzahl der Pixel
 */
static void drawLine(LENGTH x, LENGTH y, const COLOR *colors, LENGTH width) {
    Coordinate tl = {x, y};
    if (VALID_COORDINATE(tl) && width > 0u) {
        GUI_WriteLine(tl, width, colors);
    }
}
#endif

/**
 * @brief  Liest eine unkomprimierte BMP-Zeile vom UART.
 * @param  rowBuf Zielpuffer fuer die Rohdaten der Zeile
 * @param  info   Bildinformationen mit rowSize
 * @retval EOK bei Erfolg, NOK bei Leseabbruch
 */
static int readUncompressedRow(uint8_t *rowBuf, const BmpImageInfo *info) {
    RETURN_NOK_ON_ERR(NOK == readBlock(rowBuf, info->rowSize, 1u),
                      "readUncompressedRow: read failed");
    return EOK;
}

/**
 * @brief  Dekodiert eine RLE8-komprimierte Zeile in rowBuf.
 * @param  rowBuf Puffer fuer Palettenindizes (Breite = width)
 * @param  width  Bildbreite in Pixeln
 * @retval EOK bei Erfolg, NOK bei EOF oder Formatfehler
 */
static int readRle8Row(uint8_t *rowBuf, uint32_t width) {
    uint32_t x = 0u;

    memset(rowBuf, 0, width);

    while (1) {
        int c1 = nextChar();
        RETURN_NOK_ON_ERR(c1 == EOF, "readRle8Row: unexpected EOF");

        if (c1 != 0) {
            /* Encoded mode: c1-mal Farbe c2 */
            int c2 = nextChar();
            RETURN_NOK_ON_ERR(c2 == EOF, "readRle8Row: unexpected EOF in encoded mode");
            for (int i = 0; i < c1; i++) {
                if (x < width) {
                    rowBuf[x++] = (uint8_t)c2;
                }
            }
            continue;
        }

        int c2 = nextChar();
        RETURN_NOK_ON_ERR(c2 == EOF, "readRle8Row: unexpected EOF in escape");

        if (c2 == 0) {
            break; /* Zeilenende */
        }
        if (c2 == 1) {
            return EOK; /* Bildende */
        }
        if (c2 == 2) {
            /* Delta: Position um (dx, dy) verschieben */
            int dx = nextChar();
            int dy = nextChar();
            RETURN_NOK_ON_ERR(dx == EOF || dy == EOF, "readRle8Row: unexpected EOF in delta");
            x += (uint32_t)dx;
            (void)dy;
            continue;
        }

        /* Absolute mode: c2 Indizes, bei ungeradem c2 ein Padding-Byte */
        int count = c2;
        for (int i = 0; i < count; i++) {
            int idx = nextChar();
            RETURN_NOK_ON_ERR(idx == EOF, "readRle8Row: unexpected EOF in absolute mode");
            if (x < width) {
                rowBuf[x++] = (uint8_t)idx;
            }
        }
        if ((count & 1) != 0) {
            RETURN_NOK_ON_ERR(EOF == nextChar(), "readRle8Row: missing absolute padding");
        }
    }

    return EOK;
}

/**
 * @brief  Liest eine Pixelzeile (unkomprimiert oder RLE8).
 * @param  rowBuf Puffer fuer Zeilendaten
 * @param  info   Bildinformationen
 * @retval EOK bei Erfolg, NOK bei Fehler
 */
static int readPixelRow(uint8_t *rowBuf, const BmpImageInfo *info) {
    if (info->infoHeader.biCompression == BI_RLE8) {
        return readRle8Row(rowBuf, (uint32_t)info->infoHeader.biWidth);
    }
    return readUncompressedRow(rowBuf, info);
}

/**
 * @brief  Zeichnet eine dekodierte 8-Bit-Zeile auf dem LCD.
 * @param  lcdY   Y-Position auf dem Display
 * @param  rowBuf Rohdaten der Zeile (Palettenindizes)
 * @param  info   Bildinformationen inkl. Palette
 */
static void displayRow8Bit(LENGTH lcdY, const uint8_t *rowBuf, const BmpImageInfo *info) {
    uint32_t width = (uint32_t)info->infoHeader.biWidth;
#if BMP_USE_WRITE_LINE
    static COLOR line[LCD_WIDTH];
    LENGTH lineWidth = 0u;

    for (uint32_t x = 0u; x < width && x < LCD_WIDTH; x++) {
        line[lineWidth++] = paletteToColor(info->palette, rowBuf[x]);
    }
    if (lineWidth > 0u) {
        drawLine(0u, lcdY, line, lineWidth);
    }
#else
    for (uint32_t x = 0u; x < width; x++) {
        drawPixel((LENGTH)x, lcdY, paletteToColor(info->palette, rowBuf[x]));
    }
#endif
}

/**
 * @brief  Zeichnet eine dekodierte 24-Bit-Zeile auf dem LCD.
 * @param  lcdY   Y-Position auf dem Display
 * @param  rowBuf Rohdaten der Zeile (BGR-Triplets)
 * @param  info   Bildinformationen
 */
static void displayRow24Bit(LENGTH lcdY, const uint8_t *rowBuf, const BmpImageInfo *info) {
    uint32_t width = (uint32_t)info->infoHeader.biWidth;
#if BMP_USE_WRITE_LINE
    static COLOR line[LCD_WIDTH];
    LENGTH lineWidth = 0u;

    for (uint32_t x = 0u; x < width && x < LCD_WIDTH; x++) {
        const RGBTRIPLE *px = (const RGBTRIPLE *)&rowBuf[x * 3u];
        line[lineWidth++] = rgbToColor(px->rgbtRed, px->rgbtGreen, px->rgbtBlue);
    }
    if (lineWidth > 0u) {
        drawLine(0u, lcdY, line, lineWidth);
    }
#else
    for (uint32_t x = 0u; x < width; x++) {
        const RGBTRIPLE *px = (const RGBTRIPLE *)&rowBuf[x * 3u];
        drawPixel((LENGTH)x, lcdY, rgbToColor(px->rgbtRed, px->rgbtGreen, px->rgbtBlue));
    }
#endif
}

/**
 * @brief  Liest und zeichnet alle Zeilen des Bildes.
 * @param  info Bildinformationen nach readHeaders
 * @retval EOK bei Erfolg, NOK bei Lese- oder Anzeigefehler
 */
static int displayBitmap(const BmpImageInfo *info) {
    static uint8_t rowBuf[BMP_MAX_ROW_BYTES];
    uint32_t height = (uint32_t)info->infoHeader.biHeight;

    for (uint32_t bmpRow = 0u; bmpRow < height; bmpRow++) {
        RETURN_NOK_ON_ERR(NOK == readPixelRow(rowBuf, info), "displayBitmap: row read failed");

        /* BMP: erste Zeile ist unten -> Y fuer LCD umdrehen */
        int32_t lcdY = (int32_t)height - 1 - (int32_t)bmpRow;
        if (lcdY < 0 || (uint32_t)lcdY >= LCD_HEIGHT) {
            continue;
        }

        if (info->infoHeader.biBitCount == 8u) {
            displayRow8Bit((LENGTH)lcdY, rowBuf, info);
        } else {
            displayRow24Bit((LENGTH)lcdY, rowBuf, info);
        }
    }

    return EOK;
}

int bmp_readAndDisplay(void) {
    static BmpImageInfo info;

    openNextFile();
    if (NOK == readHeaders(&info)) {
        discardRestOfFile();
        return NOK;
    }
    if (NOK == displayBitmap(&info)) {
        discardRestOfFile();
        return NOK;
    }
    discardRestOfFile();
    return EOK;
}
