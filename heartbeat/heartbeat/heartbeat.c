/*
 * heartbeat.c
 *
 * Created: 10/2/2013 9:50:17 PM
 *  Author: myQ_2
 */ 




#define F_CPU (8000000L)
#include <avr/io.h>
#include <util/delay.h>

//volatile int button_was_pressed = 0;

int main(void)
{
	DDRB = 0xFF;		//all outputs port B
	//PORTB |= (1<<PB0);	//pull up resistor on (PB0)
	//EIMSK |= _BV(INT0);	//turn on interrupt 0 (PD2)
	//DDRD  = 0b11111011;   // set PD2 to input
	//PORTD = 0b00000100;   // set PD2 to high(pullup resistor)
	//PCICR |= _BV(PCIE2);   //Enable PCINT2
	//PCMSK2 |= _BV(PCINT18); //Trigger on change of PCINT18 (PD2)
	//sei();						//enable global interrupts
	
	while(1)
	{
		PORTB ^= (1<<PB5);		//toggle LED
		_delay_ms(1000);
			
		
	}//while
}//main

	