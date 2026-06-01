/**
 * @file    BMP_types.h
 * @brief   Datenstrukturen und Konstanten gemaess BMP-Dateiformat.
 */

#ifndef _BMP_TYPES_H
#define _BMP_TYPES_H

#include "MS_basetypes.h"

/** Unkomprimierte Pixeldaten */
#define BI_RGB  0
/** 8-Bit Run-Length-Encoding */
#define BI_RLE8 1

/** Dateisignatur 'BM' (Little-Endian: 0x4D42) */
#define BMP_SIGNATURE        0x4d42
/** Maximale Palettengroesse bei 8-Bit-BMP */
#define MAX_COLOR_TABLE_SIZE 256

/** BMP-Dateikopf (14 Byte) */
typedef struct tagBITMAPFILEHEADER {
    WORD  bfType;
    DWORD bfSize;
    WORD  bfReserved1;
    WORD  bfReserved2;
    DWORD bfOffBits; /* Byte-Offset zu den Pixeldaten */
} __attribute__((__packed__)) BITMAPFILEHEADER, *PBITMAPFILEHEADER;

/** BMP-Infokopf (40 Byte, BITMAPINFOHEADER) */
typedef struct tagBITMAPINFOHEADER {
    DWORD biSize;
    LONG  biWidth;
    LONG  biHeight; /* positive Hoehe: Bildzeilen von unten nach oben */
    WORD  biPlanes;
    WORD  biBitCount;      /* 8 oder 24 in diesem Projekt */
    DWORD biCompression;   /* BI_RGB oder BI_RLE8 */
    DWORD biSizeImage;
    LONG  biXPelsPerMeter;
    LONG  biYPelsPerMeter;
    DWORD biClrUsed;
    DWORD biClrImportant;
} __attribute__((__packed__)) BITMAPINFOHEADER, *PBITMAPINFOHEADER;

/** Paletteneintrag (BGR + reserviert) */
typedef struct tagRGBQUAD {
    unsigned char rgbBlue;
    unsigned char rgbGreen;
    unsigned char rgbRed;
    unsigned char rgbReserved;
} __attribute__((__packed__)) RGBQUAD;

/** 24-Bit-Pixel (BGR, ohne Alpha) */
typedef struct tagRGBTRIPLE {
    unsigned char rgbtBlue;
    unsigned char rgbtGreen;
    unsigned char rgbtRed;
} __attribute__((__packed__)) RGBTRIPLE, *PRGBTRIPLE;

#endif /* _BMP_TYPES_H */
