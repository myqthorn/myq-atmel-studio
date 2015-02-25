/*
 * SwampCoolerController.c
 *
 * Created: 9/28/2013 6:34:41 AM
 *  Author: myQ_2
 */ 


#define F_CPU (8000000)

#include <avr/io.h>
#include <util/delay.h>
//#include <avr/interrupt.h>
//#include "Atmega168DevBoardRev0.h"

void ConfigurePorts(void){
	
	DDRB = 0x20;		//all inputs except B5 output
	
	//configure PORTD for output
	//PD3, 5 & 6 will be used for PWM
	DDRD = 0xFF;
	
}



int main(void)
{
	ConfigurePorts();
		
	
	while(1)
	{
		PORTB ^= (1<<PB5);		//toggle LED
		_delay_ms(3000);
		PORTD ^= (1<<PD5);		//toggle LED
		_delay_ms(3000);
		PORTB ^= (1<<PB5);		//toggle LED
		_delay_ms(3000);
		PORTD ^= (1<<PD5);		//toggle LED
		_delay_ms(3000);
		
		/*
		for(int i=255; i>=0; i--){ OCR0A=i; _delay_ms(5);}
		for(int i=0; i<=255; i++){ OCR0A=i; _delay_ms(5);}
		
		for(int i=255; i>=0; i--){ OCR0B=i; _delay_ms(5);}
		for(int i=0; i<=255; i++){ OCR0B=i; _delay_ms(5);}
		
		for(int i=255; i>=0; i--){ OCR2B=i; _delay_ms(5);}
		for(int i=0; i<=255; i++){ OCR2B=i; _delay_ms(5);}
		
		*/
	}//while
}//main
