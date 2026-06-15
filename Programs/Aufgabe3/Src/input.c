/**
 * @file    input.c
 * @brief   UART-Protokoll zur Datenuebertragung vom Python-Programm.
 */

#include <stdbool.h>
#include <stdio.h>
#include "input.h"
#include "errorhandler.h"
#include "lcd.h"
#include "perfTimer.h"
#include "timer.h"

#define USE_DMA

/* Protokollbefehle (Einzelzeichen) */
#define HELLO_IN_CMD   'H'  /* PC: Verbindungsanfrage */
#define READY_OUT_CMD  'R'  /* Board: bereit */
#define GO_IN_CMD      'G'  /* PC: Transfer freigegeben */
#define START_OUT_CMD  'S'  /* Board: neue Datei anfordern */
#define DATA_OUT_CMD   'D'  /* Board: naechsten Datenblock anfordern */

#define BUF_SIZE       512  /* Maximale Payload-Groesse pro Paket */

#ifdef USE_DMA

#include "stm32f4xx_ll_bus.h"
#include "stm32f4xx_ll_dma.h"
#include "stm32f4xx_ll_usart.h"

#define SIZE_OF_RING_BUFFER  (3 * BUF_SIZE)
static char ringBuffer[SIZE_OF_RING_BUFFER];

/**
 * @brief  Liest bis zu len Bytes aus dem DMA-Ringpuffer.
 * @param  buf Zielpuffer
 * @param  len Anzahl der gewuenschten Bytes
 * @retval Anzahl tatsaechlich gelesener Bytes (kann kleiner als len sein)
 */
static int usbUartRead(char *buf, size_t len) {
    static int nextReadPos = 0;

    for (int i = 0; i < (int)len; i++) {
        int pos = SIZE_OF_RING_BUFFER - LL_DMA_GetDataLength(DMA1, LL_DMA_STREAM_1);
        if (nextReadPos == pos) {
            return i;
        }
        buf[i] = ringBuffer[nextReadPos];
        nextReadPos = (nextReadPos + 1) % SIZE_OF_RING_BUFFER;
    }
    return (int)len;
}

/**
 * @brief  Sendet len Bytes ueber USART3.
 * @param  buf Quellpuffer
 * @param  len Anzahl der Bytes
 */
static void usbUartWrite(const char *buf, size_t len) {
    for (size_t i = 0; i < len; i++) {
        LL_USART_TransmitData8(USART3, buf[i]);
        while (!LL_USART_IsActiveFlag_TXE(USART3)) {
        }
    }
    while (!LL_USART_IsActiveFlag_TC(USART3)) {
    }
}

/**
 * @brief  Konfiguriert DMA-Empfang fuer USART3 in den Ringpuffer.
 */
static void usbUartDMAInt(void) {
    LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_DMA1);

    LL_DMA_SetChannelSelection(DMA1, LL_DMA_STREAM_1, LL_DMA_CHANNEL_4);
    LL_DMA_SetDataTransferDirection(DMA1, LL_DMA_STREAM_1, LL_DMA_DIRECTION_PERIPH_TO_MEMORY);
    LL_DMA_SetStreamPriorityLevel(DMA1, LL_DMA_STREAM_1, LL_DMA_PRIORITY_LOW);
    LL_DMA_SetMode(DMA1, LL_DMA_STREAM_1, LL_DMA_MODE_CIRCULAR);
    LL_DMA_SetPeriphIncMode(DMA1, LL_DMA_STREAM_1, LL_DMA_PERIPH_NOINCREMENT);
    LL_DMA_SetMemoryIncMode(DMA1, LL_DMA_STREAM_1, LL_DMA_MEMORY_INCREMENT);
    LL_DMA_SetPeriphSize(DMA1, LL_DMA_STREAM_1, LL_DMA_PDATAALIGN_BYTE);
    LL_DMA_SetMemorySize(DMA1, LL_DMA_STREAM_1, LL_DMA_MDATAALIGN_BYTE);
    LL_DMA_DisableFifoMode(DMA1, LL_DMA_STREAM_1);
    LL_DMA_SetPeriphAddress(DMA1, LL_DMA_STREAM_1, LL_USART_DMA_GetRegAddr(USART3));
    LL_DMA_SetMemoryAddress(DMA1, LL_DMA_STREAM_1, (uint32_t)ringBuffer);
    LL_DMA_SetDataLength(DMA1, LL_DMA_STREAM_1, SIZE_OF_RING_BUFFER);
    LL_USART_EnableDMAReq_RX(USART3);
    LL_DMA_EnableStream(DMA1, LL_DMA_STREAM_1);
}
#endif

/**
 * @brief  Liest ein Byte vom UART (blockierend bis Daten da sind).
 */
static char readChar(void) {
#ifdef USE_DMA
    char c;
    while (0 == usbUartRead(&c, 1)) {
    }
    return c;
#else
    return (char)fgetc(stdin);
#endif
}

/**
 * @brief  Sendet ein Byte ueber den UART.
 * @param  val Zu sendendes Zeichen
 */
static void writeChar(int val) {
#ifdef USE_DMA
    char c = (char)val;
    usbUartWrite(&c, 1);
#else
    fputc(val, stdout);
#endif
}

#ifndef USE_DMA
static char buf[BUF_SIZE];

/**
 * @brief  Fuellt den lokalen Empfangspuffer mit size Bytes (ohne DMA).
 * @param  size Anzahl der Bytes
 */
void fillBuf(int size) {
    for (int i = 0; i < size; i++) {
        buf[i] = (char)readChar();
    }
}
#endif

static int nextCharPos = BUF_SIZE;
static int noElemsInBuf = 0;
static bool fileIncomplete = false;

void initInput(void) {
#ifdef USE_DMA
    usbUartDMAInt();
#endif
    char ch;

    while (HELLO_IN_CMD != (char)readChar()) {
    }
    writeChar(READY_OUT_CMD);

    /* Weitere H-Befehle aus dem PC-Puffer ueberspringen, bis G kommt */
    do {
        ch = (char)readChar();
        if ((HELLO_IN_CMD != ch) && (GO_IN_CMD != ch)) {
            lcdPrintS("Unexpected input cmd received.");
        }
    } while (GO_IN_CMD != ch);
}

/**
 * @brief  Fordert beim PC den naechsten Datenblock an und liest die Laenge ein.
 * @param  openNewFile true: Befehl 'S' (neue Datei), false: Befehl 'D' (Folgeblock)
 */
static void startNextByteBurst(bool openNewFile) {
    if (openNewFile) {
        writeChar(START_OUT_CMD);
    }
#ifndef USE_DMA
    if (!openNewFile) {
        writeChar(DATA_OUT_CMD);
    }
#endif

    /* 16-Bit-Laenge big-endian, danach Payload vom PC */
    noElemsInBuf = (((char)readChar()) & 0xff) << 8;
    noElemsInBuf = noElemsInBuf | (((char)readChar()) & 0xff);
#ifdef USE_DMA
    if (0 != noElemsInBuf) {
        writeChar(DATA_OUT_CMD);
    }
#endif
    LOOP_ON_ERR(noElemsInBuf > BUF_SIZE, "startNextByteBurst: To many input data.");
#ifndef USE_DMA
    fillBuf(noElemsInBuf);
#endif
    nextCharPos = 0;
}

int nextChar(void) {
    if (0 == noElemsInBuf) {
        return EOF;
    }
    /* Aktueller Block verbraucht -> 'D' und naechstes Paket anfordern */
    if ((noElemsInBuf == nextCharPos) || (BUF_SIZE == nextCharPos)) {
        startNextByteBurst(false);
    }
    if (0 == noElemsInBuf) {
        return EOF;
    }
#ifdef USE_DMA
    nextCharPos++;
    char c = readChar();
    return (int)c;
#else
    return buf[nextCharPos++];
#endif
}

void discardRestOfFile(void) {
    while (EOF != nextChar()) {
    }
}

void markFileIncomplete(void) {
    fileIncomplete = true;
}

void openNextFile(void) {
    if (fileIncomplete) {
        /* Fehlerfall: Rest nicht byteweise lesen (sehr langsam bei grossen BMPs).
         * Ein 'S' beim Tastendruck bricht die Datei am PC ab und holt das naechste Bild. */
        fileIncomplete = false;
#ifdef USE_DMA
        char c;
        while (0 != usbUartRead(&c, 1)) {
        }
#endif
        noElemsInBuf = 0;
        nextCharPos = BUF_SIZE;
        startNextByteBurst(true);
        return;
    }

#ifdef USE_DMA
    char c;
    while (0 != usbUartRead(&c, 1)) {
    }
#endif
    if (0 != noElemsInBuf) {
        discardRestOfFile();
    }
    noElemsInBuf = 0;
    nextCharPos = BUF_SIZE;
    startNextByteBurst(true);
}

int COMread(char *buf, unsigned int size, unsigned int count) {
    for (unsigned int i = 0; i < size * count; i++) {
        int c = nextChar();
        if (EOF == c) {
            return EOF;
        }
        buf[i] = (char)c;
    }
    return count;
}
