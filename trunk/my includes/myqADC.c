/*
 * myqADC.c
 *
 * Created: 2/14/2015 2:09:50 PM
 *  Author: myQ
 */ 

#include "myqADC.h"


//@brief initialize AD converter on Atmega168 for
void initADC(uint8_t channel, uint8_t prescalar){
	
	//Power reduction ADC bit must be zero to enable ADC
	PRR &= ~(1<<PRADC);
	
	
	//ADC clock needs to be between 50kHz and 200kHz
	//8MHz/64 = 125kHz
	ADCSRA = (1<<ADEN | prescalar<<ADPS0);	
	
	//Disable the digital input for the ADC channels we will be using
	DIDR0 |= (1<<channel);
}

void readADC(uint8_t channel){
		
	//start conversion
	startADC(channel);
	
	//wait for conversion to finish (interrupt flag is set)
	while( !isADCfinished() );
	
	clearADCinterrupt();
}

void startADC(uint8_t channel){
	//set ADC channel
	ADMUX = (0x00) | channel;
	
	//start conversion
	ADCSRA |= (1<<ADSC);
}

uint8_t isADCfinished(){
	return ((ADCSRA & (1<<ADSC))?0:1);
	//return ((ADCSRA & (1<<ADIF)));
}

void clearADCinterrupt(){
	//clear interrupt by setting ADIF to 1
	ADCSRA |= (1<<ADIF);
}

void ADCoff(){
	ADCSRA &= ~(1<<ADEN);	
}
