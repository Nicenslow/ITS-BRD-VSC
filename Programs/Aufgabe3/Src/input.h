#ifndef _INPUT_H
#define _INPUT_H

/**
* @brief This function setups communication protocol to Python send file program
* @param None
* @retval None
*/
extern void initInput(void);

/**
* @brief This function asks the Python program to open a new file. 
*        If all files has been read, it blocks until a new file has been
*        selected in Python program.
* @param None
* @retval None
*/
extern void openNextFile(void);

/** Liest den Rest der aktuellen Datei bis EOF (leeres Paket). */
extern void discardRestOfFile(void);

/**
* @brief  This function reads the next byte from the current file that will be 
*         transmitted by Python.
* @param  None
* @retval Next byte of the current file or EOF (32 bit value for -1)
*/
extern int nextChar(void);

/**
* @brief  This function reads count elements of data, each size bytes long, 
*         from from the current file that will be transmitted by Python.
* @param  buf   Storage buffer for data
* @param  size  Size of each data element
* @param  count Number of data elements
* @retval Number of items read. If not enough data are available, EOF will be returned.
*/
extern int COMread(char*, unsigned int size, unsigned int count);

#endif
// EOF