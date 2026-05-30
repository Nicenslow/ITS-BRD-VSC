#include "bmp.h"

#include <stdio.h>
#include <string.h>

#include "bmp_check.h"
#include "errorhandler.h"
#include "input.h"
#include "LCD_GUI.h"

static COLOR rgbToColor(unsigned char red, unsigned char green, unsigned char blue) {
    return (COLOR)((((uint16_t)red & 0xF8u) << 8) |
                   (((uint16_t)green & 0xFCu) << 3) |
                   ((uint16_t)blue >> 3));
}

static COLOR paletteToColor(const RGBQUAD *palette, uint8_t index) {
    return rgbToColor(palette[index].rgbRed, palette[index].rgbGreen, palette[index].rgbBlue);
}

static uint32_t calcRowSize(uint32_t width, uint16_t bitCount) {
    return (((width * (uint32_t)bitCount) + 31u) / 32u) * 4u;
}

static int readBlock(void *buf, unsigned int elemSize, unsigned int count) {
    int ret = COMread((char *)buf, elemSize, count);
    return (ret == (int)count) ? EOK : NOK;
}

static int skipBytes(DWORD count) {
    for (DWORD i = 0u; i < count; i++) {
        if (EOF == nextChar()) {
            return NOK;
        }
    }
    return EOK;
}

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
static void drawPixel(LENGTH x, LENGTH y, COLOR color) {
    Coordinate crd = {x, y};
    if (VALID_COORDINATE(crd)) {
        GUI_drawPoint(crd, color, DOT_PIXEL_1X1, DOT_FILL_AROUND);
    }
}
#endif

#if BMP_USE_WRITE_LINE
static void drawLine(LENGTH x, LENGTH y, const COLOR *colors, LENGTH width) {
    Coordinate tl = {x, y};
    if (VALID_COORDINATE(tl) && width > 0u) {
        GUI_WriteLine(tl, width, colors);
    }
}
#endif

static int readUncompressedRow(uint8_t *rowBuf, const BmpImageInfo *info) {
    RETURN_NOK_ON_ERR(NOK == readBlock(rowBuf, info->rowSize, 1u),
                      "readUncompressedRow: read failed");
    return EOK;
}

static int readRle8Row(uint8_t *rowBuf, uint32_t width) {
    uint32_t x = 0u;

    memset(rowBuf, 0, width);

    while (1) {
        int c1 = nextChar();
        RETURN_NOK_ON_ERR(c1 == EOF, "readRle8Row: unexpected EOF");

        if (c1 != 0) {
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
            break;
        }
        if (c2 == 1) {
            return EOK;
        }
        if (c2 == 2) {
            int dx = nextChar();
            int dy = nextChar();
            RETURN_NOK_ON_ERR(dx == EOF || dy == EOF, "readRle8Row: unexpected EOF in delta");
            x += (uint32_t)dx;
            (void)dy;
            continue;
        }

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

static int readPixelRow(uint8_t *rowBuf, const BmpImageInfo *info) {
    if (info->infoHeader.biCompression == BI_RLE8) {
        return readRle8Row(rowBuf, (uint32_t)info->infoHeader.biWidth);
    }
    return readUncompressedRow(rowBuf, info);
}

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

static int displayBitmap(const BmpImageInfo *info) {
    static uint8_t rowBuf[BMP_MAX_ROW_BYTES];
    uint32_t height = (uint32_t)info->infoHeader.biHeight;

    for (uint32_t bmpRow = 0u; bmpRow < height; bmpRow++) {
        RETURN_NOK_ON_ERR(NOK == readPixelRow(rowBuf, info), "displayBitmap: row read failed");

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
