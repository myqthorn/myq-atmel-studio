// myqADC.cpp
//Mike Litster
//Will Eccles
//use with ADC on Atmega168/328
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

///@brief initialize AD converter on Atmega168 for
///@param	ch		ADC channel
///@param	pscalar	Prescalar
ANALOG::ANALOG(uint8_t ch, uint8_t pscalar){
	channel = ch;
	prescalar = pscalar;
	reinitialize();
	
}

///@description	Sets up the ADC with values already loaded in the class
void ANALOG::reinitialize(){
	//Power reduction ADC bit must be zero to enable ADC
	PRR &= ~(1<<PRADC);	
	
	//ADC clock needs to be between 50kHz and 200kHz
	//8MHz/64 = 125kHz
	ADCSRA = (1<<ADEN | prescalar<<ADPS0);
	
	//Disable the digital input for the ADC channel we will be using
	DIDR0 |= (1<<channel);
}

///@description	Perform a complete conversion. Should only be used when waiting isn't a problem.
uint16_t ANALOG::read(){
		
	//start conversion
	start();
	
	//wait for conversion to finish (interrupt flag is set)
	while( isReading() && !isInterruptFlagSet());
	
	clearFlag();
	return (ADC);
}

///@description	Begin an ADC conversion
void ANALOG::start(){
	//set ADC channel
	ADMUX = (1<<REFS0) | channel;
	
	//start conversion
	ADCSRA |= (1<<ADSC);
}

///@description	Indicates that it is currently performing an ADC conversion
///@returns		TRUE when a conversion is in progress, FALSE when complete
uint8_t ANALOG::isReading(){
	return (ADCSRA & (1<<ADSC));	
}

///@description Indicate when the ADC interrupt flag is set
///@returns		TRUE when the flag is set, FALSE otherwise
uint8_t	ANALOG::isInterruptFlagSet(){
	return(ADCSRA & (1<<ADIF));
}

///@description	Clears ADC interrupt flag
void ANALOG::clearFlag(){
	//clear interrupt by setting ADIF to 1
	ADCSRA |= (1<<ADIF);
}

//@description	Turns off ADC module
void ANALOG::off(){
	ADCSRA &= ~(1<<ADEN);	
}
