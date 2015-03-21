/*
 * myqADC.cpp
 *
 * Created: 2/14/2015 2:09:50 PM
 *  Author: myQ
 */ 

#include "myqADC.h"

///@brief initialize ADC0 with 64 prescalar
//ADC clock needs to be between 50kHz and 200kHz
//Assume 8Mhz F_OSC
//8MHz/64 = 125kHz
ANALOG::ANALOG(){
	channel = 0;
	prescalar = ADC_DIV64;
	reinitialize();
}

//@brief initialize AD converter on Atmega168 for
ANALOG::ANALOG(uint8_t ch, uint8_t pscalar){
	channel = ch;
	prescalar = pscalar;
	reinitialize();
	
}

void ANALOG::reinitialize(){
	//Power reduction ADC bit must be zero to enable ADC
	PRR &= ~(1<<PRADC);
	
	
	//ADC clock needs to be between 50kHz and 200kHz
	//8MHz/64 = 125kHz
	ADCSRA = (1<<ADEN | prescalar<<ADPS0);
	
	//Disable the digital input for the ADC channel we will be using
	DIDR0 |= (1<<channel);
}

uint16_t ANALOG::read(){
		
	//start conversion
	start();
	
	//wait for conversion to finish (interrupt flag is set)
	while( isReading() && !isInterruptFlagSet());
	
	clearFlag();
	return (ADC);
}

void ANALOG::start(){
	//set ADC channel
	ADMUX = (1<<REFS0) | channel;
	
	//start conversion
	ADCSRA |= (1<<ADSC);
}

uint8_t ANALOG::isReading(){
	return (ADCSRA & (1<<ADSC));	
}

uint8_t	ANALOG::isInterruptFlagSet(){
	return(ADCSRA & (1<<ADIF));
}

void ANALOG::clearFlag(){
	//clear interrupt by setting ADIF to 1
	ADCSRA |= (1<<ADIF);
}

void ANALOG::off(){
	ADCSRA &= ~(1<<ADEN);	
}
