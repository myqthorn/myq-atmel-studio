//timer1.c

#include "timer1.h"

#include <avr/interrupt.h>

///@brief initialize AD converter on Atmega168 for
void	initTimer1(uint8_t prescalar, uint8_t WGMmode, uint16_t reload){
	
	// The PRTIM1 bit in the Power reduction register must be written to zero to enable Timer/Counter1 module.
	PRR &= ~(1<<PRTIM1);
	
	
	
	TCCR1A = (WGMmode & 0x03);		//normal timer operation, no PWM, etc
	TCCR1B = ((WGMmode & 0x0C)<<WGM12) | (prescalar << CS10);
	TCCR1C = 0x00;		//force output compare off	
	OCR1A = reload;
	TCNT1 = 0;
	
	TIFR1 = 1<<ICF1;			//Clear pending interrupts
	
	TIMSK1 |= (1 << TOIE1) |	// Enable
	(1<<OCIE1A);				// Enable capture event timer
	//sei();
}

uint8_t isTimer1FlagSet(){
	return ((TIFR1 & 1<<OCF1A)? 1:0);
	//return ((EIFR & 1<< INTF0)?1:0);
}

void	clearTimer1Flag(){
	TIFR1 |= (1<<OCF1A);
	
	//EIFR = (1<<INTF0);		//Reset flag
}


void startTimer1(void) 
{

   
}

void stopTimer1(void)
{
    TCCR1B = 0x00;
    TIMSK1 = 0x00;
}

