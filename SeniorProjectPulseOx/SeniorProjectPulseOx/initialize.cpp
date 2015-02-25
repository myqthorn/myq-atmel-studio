//initialize.cpp




#include "initialize.h"


#include <avr/io.h>
#include <avr/interrupt.h>

#ifdef __cplusplus
extern "C" {
	#endif
	
	#include "timer1.h"
	#include "myqADC.h"

	#ifdef __cplusplus
}
#endif


void init(void){
	
	//MCUCR = (1<<PUD);	//disable all pull ups
	
	//d6,d7 - LEDs
	DDRD = 0xC0;			//set PD7:6 to outputs for LEDs
	//PD2 as an input for switch
	RED_ON;					//Set PD7 high and PD6 low
	
	//Switch
	PORTD |= (1<<PD2);		//set PD2 to high(pullup resistor) for switch
	//EIMSK |= (1<<INT0);		//turn on interrupt 0 (PD2)
	//PCICR |= (1<<PCIE2);	//Enable PCINT2
	//PCMSK2 |= (1<<PCINT18); //Trigger on change of PCINT18 (PD2)
	//sei();					//enable interrupts
	
	//DDRB = 0X20;		//all inputs except B5 output
	//PORTB |= (1<<PB0);	//pull up resistor on (PB0)
	
	//DDRD  = 0b11111011;   // set PD2 to input
	
	//ADC
	
	//ADC clock must be between 50-200kHz to get maximum resolution
	//8MHz/64 (div64)	= 125,000
	
	//pin, prescalar
	initADC(ADC_PIN, ADC_DIV64);
	
	//timer
	// 8MHz/60Hz			= 133,333.33333 cycles to count
	// 133,333.33333 / 8 (div8)	= 16666.666667 = 0x411B	
	
	//prescalar, mode, reload
	//initTimer1(TIMER_DIV8, 4, 0x411B);
	initTimer1(TIMER_DIV64, 4, 0xFFFF);
		
	//opamp
	//opamp is Powered Down when PD is low, initialize HIGH?
	//250nS delay when powering up, 50nS delay when powering down (1/8MHz = 125nS)
	WakeOpAmp();
	
	
}

