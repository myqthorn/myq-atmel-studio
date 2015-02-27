/*
 * TimerPractice.cpp
 *
 * Created: 2/21/2015 5:53:29 AM
 *  Author: myQ_2
 */ 

#include "HardwareProfile.h"
#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>

#include "timer1.h"	


#define RELOAD 0x0FFF

int main(void)
{
	DDRD = 0xC0;			//set PD7:6 to outputs for LEDs
	
	
	TIMER1 timer(TIMER_DIV8, 4, 0x2B67);
	
	RED_ON;
	timer.start();
    while(1)
    {

			   if (timer.isCompareAFlagSet()){
				   timer.clearCompareAFlag();
				   timer.setCount(0);
				   PORTD ^= (1<<PD7);
			   }
    }
}

