/*
 * PWMpractice.c
 *
 * Created: 9/2/2013 7:06:02 AM
 *  Author: myQ
 */ 

#define F_CPU (8000000)

#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>


void ConfigurePorts(void){
	
	DDRB = 0x20;		//all inputs except B5 output
	
	//configure PORTD for output
	//PD3, 5 & 6 will be used for PWM
	DDRD = 0xFF;
	
}

void InitPWM(void){
	
	TCCR0A = (1<<COM0A1) | (1<<COM0B1) | (1<<WGM01) | (1<<WGM00);
	TCCR0B = (1<<CS00);
	TCCR2A = (1<<COM2B1) | (1<<COM0B1) | (1<<WGM21) | (1<<WGM20);
	TCCR2B = (1<<CS20);
}

int main(void)
{
	ConfigurePorts();
	InitPWM();
	
	
    while(1)
    {
		PORTB ^= (1<<PB5);		//toggle LED
		//_delay_ms(50);
        
		for(int i=255; i>=0; i--){ OCR0A=i; _delay_ms(5);}
		for(int i=0; i<=255; i++){ OCR0A=i; _delay_ms(5);}
		
		for(int i=255; i>=0; i--){ OCR0B=i; _delay_ms(5);}
		for(int i=0; i<=255; i++){ OCR0B=i; _delay_ms(5);}
		
		for(int i=255; i>=0; i--){ OCR2B=i; _delay_ms(5);}	
		for(int i=0; i<=255; i++){ OCR2B=i; _delay_ms(5);}
			
    }//while
}//main



