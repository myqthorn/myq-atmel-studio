/*
 * blink.c
 *
 * Created: 8/22/2013 8:59:22 PM
 *  Author: myQ_2
 */ 

#define inpin (PINB & 1 << PB0)

#define F_CPU (8000000L)
#include <avr/io.h>
#include <util/delay.h>

volatile int button_was_pressed = 0;

int main(void)
{
	DDRB = 0X20;		//all inputs except B5 output
	PORTB |= (1<<PB0);	//pull up resistor on (PB0)
	EIMSK |= _BV(INT0);	//turn on interrupt 0 (PD2)
	DDRD  = 0b11111011;   // set PD2 to input
	PORTD = 0b00000100;   // set PD2 to high(pullup resistor)
	PCICR |= _BV(PCIE2);   //Enable PCINT2
	PCMSK2 |= _BV(PCINT18); //Trigger on change of PCINT18 (PD2)
	//sei();						//enable global interrupts
	
	while(1)
	{
		
		//if (button_was_pressed){
			PORTB ^= (1<<PB5);		//toggle LED
			//button_was_pressed = 0;	//reset button press flag
		_delay_ms(1000);
	}
}

//ISR(SIG_INTERRUPT0){
	//button_was_pressed = 1;	
//}