//timer1.cpp
//Mike Litster
//Will Eccles
//use with Timer1 on Atmega168/328

#include "timer1.h"

#include <avr/interrupt.h>

///@description	Default Constructor
TIMER1::TIMER1(){
	TIMER1(TIMER_DIV1, 0, 0);
}

///@brief initialize AD converter on Atmega168/328
///@param pscalar	Prescalar
///@param WGMmode	Use WGMmode values from datasheet 
///@param rload		Reload value
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

///@description Start the timer
void	TIMER1::start(void)
{
	TCCR1B = ((mode & 0x0C)<<WGM12) | (prescalar << CS10);
	
}

///@description Stop the timer
void	TIMER1::stop(void)
{
	TCCR1B = 0x00;
	TIMSK1 = 0x00;
}

///@description	Check Overflow Flag
///@returns		TRUE when Flag is set, FALSE otherwise
uint8_t TIMER1::isOVFlagSet(){
	return ((TIFR1 & 1<<TOV1)? 1:0);
	//return ((EIFR & 1<< INTF0)?1:0);
}

///@description	Check Compare A Flag
///@returns		TRUE when Flag is set, FALSE otherwise
uint8_t TIMER1::isCompareAFlagSet(){
	return ((TIFR1 & 1<<OCF1A)? 1:0);
	
}

///@description Clear Overflow flag
void	TIMER1::clearOVFlag(){
	TIFR1 |= (1<<TOV1);
	
	//EIFR = (1<<INTF0);		//Reset flag
}

///@description Clear Compare A flag
void	TIMER1::clearCompareAFlag(){
	TIFR1 |= (1<<OCF1A);
	
	//EIFR = (1<<INTF0);		//Reset flag
}

///@description	Sets timer counter
///@param		count	Value to set counter to
void	TIMER1::setCount(uint16_t count){
	TCNT1 = count;
}
