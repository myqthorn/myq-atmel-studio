/*
 * atmega168_as_external_oscillator.c
 *
 * Created: 10/2/2013 9:50:17 PM
 *  Author: myQ_2
 */ 




#define F_CPU (8000000L)
#include <avr/io.h>
#include <util/delay.h>

int main()
{

	DDRB = 0xFF;
	while (1)
	{

		PORTB = ~PINB;
		asm volatile("nop");
		asm volatile("nop");
		asm volatile("nop");
		asm volatile("nop");
		//asm volatile("nop");
		//asm volatile("nop");
		//asm volatile("nop");
		//asm volatile("nop");
		// _delay_ms(100); // Uncomment this line to probe if something is live :)
	}
}
	