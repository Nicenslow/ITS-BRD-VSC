#ifndef AUFGABE3_BMP_CHECK_H
#define AUFGABE3_BMP_CHECK_H

#include "BMP_types.h"

/**
 * @brief  Prueft BMP-Header auf gueltiges Format (8/24 Bit, Kompression, Offset).
 * @param  fileHeader Dateikopf aus der BMP
 * @param  infoHeader Infokopf aus der BMP
 * @retval EOK Header gueltig
 * @retval NOK Header ungueltig (Fehler auf LCD)
 */
int basicChecks(const BITMAPFILEHEADER *fileHeader, const BITMAPINFOHEADER *infoHeader);

#endif /* AUFGABE3_BMP_CHECK_H */
