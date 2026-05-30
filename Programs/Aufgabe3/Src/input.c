#include <stdio.h>
#include <stdbool.h>
#include "input.h"
#include "errorhandler.h"
#include "lcd.h"
#include "perfTimer.h"
#include "timer.h"

#define USE_DMA 

/*
 * Commands and constants for transfer protocol
 */
#define HELLO_IN_CMD	      'H'   // Hello cmd send from Python program until our programs answers with READY 
#define READY_OUT_CMD      'R'   // Ready cmd send to Phyton program 
#define GO_IN_CMD          'G'   // This cmd will be send by Python program after it has received the Ready command.
                                 // After this command has been sent, Python program opens first BITMAP files and send data

#define START_OUT_CMD      'S'   // Ask Python program to transmit the first data package of a new fill.
                                 // If all BITMAP files has been transmitted, Python program blocks until new files has been selected
#define DATA_OUT_CMD       'D'   // Ask Python program to transmit the next data package of the current file 
                                 // An empty data package represents an EOF

#define BUF_SIZE   512           // Number of bytes of one burst 


#ifdef USE_DMA
/*
 * Code for USART RX DMA 
 */

#include "stm32f4xx_ll_bus.h"
#include "stm32f4xx_ll_dma.h"
#include "stm32f4xx_ll_usart.h"

// RX ring buffer for RX DMA controller
#define SIZE_OF_RING_BUFFER         (3*BUF_SIZE)
static char ringBuffer[SIZE_OF_RING_BUFFER];  // ring buffer

static int usbUartRead(char *buf, size_t len){
   // ToDo Check overflow of ring buffer
   static int nextReadPos = 0; // The next read from the DMA ring buffer reads the value from this position
   for (int i = 0; i < (int)len; i++){
      int pos = SIZE_OF_RING_BUFFER - LL_DMA_GetDataLength(DMA1, LL_DMA_STREAM_1); 
      if (nextReadPos == pos) return (int) i;
      buf[i] = ringBuffer[nextReadPos];
      nextReadPos = (nextReadPos + 1) % SIZE_OF_RING_BUFFER;
   }
   return (int)len;
}

static void usbUartWrite(const char *buf, size_t len){
   for(size_t i = 0; i < len; i++){
      LL_USART_TransmitData8(USART3, buf[i]);
      while (!LL_USART_IsActiveFlag_TXE(USART3)) {}
   }
   while (!LL_USART_IsActiveFlag_TC(USART3)) {} 
}

static void usbUartDMAInt(void) {
   /* Peripheral clock enable */
   LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_DMA1);
   /* USART3 DMA Init */
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
   /* Enable USART and DMA */
   LL_DMA_EnableStream(DMA1, LL_DMA_STREAM_1);
}
#endif

/*
 * readChar & writeChar : read / write low level access to UART or local read buffer
 */
static char readChar(void){
#ifdef USE_DMA
   char c;
   while (0 == usbUartRead(&c, 1));
   return c;
#else
   return (char) fgetc(stdin);
#endif
}

static void writeChar(int val){
#ifdef USE_DMA
   char c = (char) val;
   usbUartWrite(&c, 1);
#else 
   fputc(val, stdout);
#endif
}

#ifndef USE_DMA
/*
 * local read buffer that will be for each burst of bytes started by START_OUT_CMD or DATA_OUT_CMD command
 */
static char buf[BUF_SIZE];          // data buffer if no DMA
void fillBuf(int size){
   for(int i = 0; i < size; i++){
      buf[i] = (char) readChar();
   }
}
#endif

/*
 * Variables to manage next byte burst 
 */
static int nextCharPos = BUF_SIZE;  // next unread element in buffer (BUF_SIZE : alle elems has been consumed)
static int noElemsInBuf = 0;        // number of elems in buffer; noElemsInBuf == 0 : EOF

void initInput(void){
#ifdef USE_DMA
   usbUartDMAInt();
#endif
   char ch;
   // wait for hello
   while (HELLO_IN_CMD != (char) readChar()){};
   // got first hello
   writeChar(READY_OUT_CMD);
   // Yet some other Hello cmds might be in output buffer of Python programs (e.g. due to long wait)
   do {
      ch = (char)readChar();
      if((HELLO_IN_CMD != ch) && (GO_IN_CMD != ch)){
         lcdPrintS("Unexpected input cmd received.");
      }
   } while (GO_IN_CMD != ch);
}

/**
* @brief This function will be called when all byte of current byte burst has been consumed
*
* @param openNewFile  A new file should be opened
* @retval None
*/
static void startNextByteBurst(bool openNewFile){
   // Ask for data package
   if (openNewFile){
      writeChar(START_OUT_CMD);
   }
#ifndef USE_DMA
   if (!openNewFile){
      writeChar(DATA_OUT_CMD);
   }
#endif
   // read no of data that will be transmitted
   noElemsInBuf = (((char) readChar())& 0xff) << 8;
   noElemsInBuf = noElemsInBuf | (((char) readChar())& 0xff);
#ifdef USE_DMA
   if (0 != noElemsInBuf){
      writeChar(DATA_OUT_CMD);  // start transmission of next byte burst
   }
#endif
   LOOP_ON_ERR(noElemsInBuf > BUF_SIZE, "startNextByteBurst: To many input data.");
#ifndef USE_DMA  
   fillBuf(noElemsInBuf);
#endif
   nextCharPos = 0;
}

int nextChar(void){
   if(0 == noElemsInBuf){
      return EOF;
   }
   if((noElemsInBuf == nextCharPos) || (BUF_SIZE == nextCharPos)){
      // start next byte burst
      startNextByteBurst(false);
   }
   if(0 == noElemsInBuf){
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

void openNextFile(void){
#ifdef USE_DMA
   // clear DMA ring buffer
   char c;
   while (0 != usbUartRead(&c, 1));
#endif
   if (0 != noElemsInBuf) {
      discardRestOfFile();
   }
   noElemsInBuf = 0;
   nextCharPos = BUF_SIZE;
   startNextByteBurst(true);
}

int COMread(char* buf, unsigned int size, unsigned int count){
   for (unsigned int i = 0; i < size * count; i++){
      int c = nextChar();
      if (EOF == c) {
         return EOF;
      }
      buf[i] = (char) c;
   }
   return count;
}

//EOF
