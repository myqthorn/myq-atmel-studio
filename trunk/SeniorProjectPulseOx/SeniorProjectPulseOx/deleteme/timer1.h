//timer1.h
 


#ifndef _TIMER_1_H
#define _TIMER_1_H

#include "HardwareProfile.h"

#define TIMER_CLOCK_OFF		0
#define TIMER_DIV1			1
#define TIMER_DIV8			2
#define TIMER_DIV64			3
#define TIMER_DIV256		4
#define TIMER_DIV1024		5
#define TIMER_FALL_T1		6
#define TIMER_RISE_T1		7















void	initTimer1();
void	startTimer1(void);
void	stopTimer1(void);
uint8_t isTimer1FlagSet();
void	clearTimer1Flag();
#endif //_TIMER_1_H
