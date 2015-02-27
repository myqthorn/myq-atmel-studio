//timer1.c

#include "timer1.h"

#include <avr/interrupt.h>

TIMER1::TIMER1(){
	TIMER1(TIMER_DIV1, 0, 0);
}

///@brief initialize AD converter on Atmega168 for
TIMER1::TIMER1(uint8_t pscalar, uint8_t WGMmode, uint16_t rload){
	reload = rload;
	mode = WGMmode;
	prescalar = pscalar;
	
	// The PRTIM1 bit in the Power reduction register must be written to zero to enable Timer/Counter1 module.
	PRR &= ~(1<<PRTIM1);
	
	//disable timer1
	TCCR1B = 0x00;
	
	//clear counter	
	TCNT1 = 0;
	
	//set reload value
	if (mode == 4) OCR1A = reload;
	if (mode == 12) ICR1 = reload;
	if (mode == 0) TCNT1 = reload;
	
	//setup and enable timer
	TCCR1A = (mode & 0x03);		//normal timer operation, no PWM, etc
	//TCCR1B = ((mode & 0x0C)<<WGM12) | (prescalar << CS10);
	TCCR1C = 0x00;		//force output compare off	
	
	
	
	//TIFR1 = 1<<ICF1;			//Clear pending interrupts
	
// 	TIMSK1 |= (1 << TOIE1) |	// Enable
// 	(1<<OCIE1A);				// Enable capture event timer
	//sei();
}

void	TIMER1::start(void)
{
	TCCR1B = ((mode & 0x0C)<<WGM12) | (prescalar << CS10);
	
}

void	TIMER1::stop(void)
{
	TCCR1B = 0x00;
	TIMSK1 = 0x00;
}

uint8_t TIMER1::isOVFlagSet(){
	return ((TIFR1 & 1<<TOV1)? 1:0);
	//return ((EIFR & 1<< INTF0)?1:0);
}

uint8_t TIMER1::isCompareAFlagSet(){
	return ((TIFR1 & 1<<OCF1A)? 1:0);
	
}

void	TIMER1::clearOVFlag(){
	TIFR1 |= (1<<TOV1);
	
	//EIFR = (1<<INTF0);		//Reset flag
}

void	TIMER1::clearCompareAFlag(){
	TIFR1 |= (1<<OCF1A);
	
	//EIFR = (1<<INTF0);		//Reset flag
}

void	TIMER1::setCount(uint16_t count){
	TCNT1 = count;
}
