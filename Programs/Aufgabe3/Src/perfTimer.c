#include "perfTimer.h"
#include "errorhandler.h"
#include "timer.h"
#include <stdint.h>
#include <stdbool.h>

#define NO_TIMERS       5

uint64_t perfTimers[NO_TIMERS];
uint64_t noPerfTimerCalls[NO_TIMERS];
uint32_t timeStamp[NO_TIMERS];
bool firstCallPerfTimer[NO_TIMERS];


void initPerfTimers(void){
   initTimer();
   for (int i = 0; i < NO_TIMERS; i++){
      perfTimers[i] = 0;
      noPerfTimerCalls[i] = 0;
      firstCallPerfTimer[i] = true;
   }
}

void startPerfTimer(unsigned int tim){
   LOOP_ON_ERR(tim >= NO_TIMERS, "startPerfTimr: Wrong timer number.");
   firstCallPerfTimer[tim] = false;
   noPerfTimerCalls[tim] = noPerfTimerCalls[tim] + 1;
   timeStamp[tim] = getTimeStamp();
}

void stopPerfTimer(unsigned int tim){
   LOOP_ON_ERR(tim >= NO_TIMERS, "stopPerfTimr: Wrong timer number.");
   if (!firstCallPerfTimer[tim]){
      perfTimers[tim] = perfTimers[tim] + (getTimeStamp() - timeStamp[tim]);
   }
}

// EOF
