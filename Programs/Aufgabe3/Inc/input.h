#ifndef _INPUT_H
#define _INPUT_H

/**
 * @brief  Initialisiert UART/DMA und fuehrt Handshake mit Python durch (H, R, G).
 * @param  Keine
 * @retval Keiner (void)
 */
extern void initInput(void);

/**
 * @brief  Fordert beim PC die naechste BMP-Datei an (Befehl 'S').
 * @param  Keine
 * @retval Keiner (void)
 */
extern void openNextFile(void);

/**
 * @brief  Liest den Rest der aktuellen Datei bis EOF (leeres Paket).
 * @param  Keine
 * @retval Keiner (void)
 */
extern void discardRestOfFile(void);

/**
 * @brief  Markiert die aktuelle Datei nach einem Fehler als abgebrochen.
 *         Beim naechsten Tastendruck (openNextFile) wird einmal 'S' gesendet,
 *         ohne den Rest der Datei ueber UART einzulesen.
 * @param  Keine
 * @retval Keiner (void)
 */
extern void markFileIncomplete(void);

/**
 * @brief  Liest das naechste Byte der aktuellen Datei.
 * @param  Keine
 * @retval Byte (0..255) oder EOF (-1) am Dateiende
 */
extern int nextChar(void);

/**
 * @brief  Liest count Elemente mit je size Bytes aus der aktuellen Datei.
 * @param  buf   Zielpuffer
 * @param  size  Elementgroesse in Bytes
 * @param  count Anzahl der Elemente
 * @retval count bei Erfolg, EOF bei vorzeitigem Dateiende
 */
extern int COMread(char *buf, unsigned int size, unsigned int count);

#endif /* _INPUT_H */
