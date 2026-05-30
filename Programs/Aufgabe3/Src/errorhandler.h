#ifndef _ERRORHANDLER_H
#define _ERRORHANDLER_H

#include <stdbool.h>

#define NOK				-1
#define EOK				0

// Do not call this function directly. Use the wrapper macro ERROR_HANDLER.
extern int printError(bool cnd, char *file, int line, char *msg, bool loopForEver);


/**
* @brief Wrapper of the simple error handler waitOnError that
*        writes an error message von LCD display and loops forever.
* @param cnd  If this condition is true, an error will be reported.
* @param msg  Error message
* @retval None
*/

#define LOOP_ON_ERR(cnd,msg) printError((cnd),__FILE__,__LINE__,(msg), true)
	
#define ERR_HANDLER(cnd,msg) printError((cnd),__FILE__,__LINE__,(msg), false)

// Die beiden folgenden Markros sind fehleranfälliger C Code 
#define RETURN_NOK_ON_ERR(cnd,msg) {if (NOK == ERR_HANDLER(cnd,msg)){return NOK;}}
#define RAISE_NOK(fcall) {if (NOK == (fcall)){return NOK;}}

#endif
// EOF

