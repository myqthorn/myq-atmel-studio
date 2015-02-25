/*
 * TimerPractice.cpp
 *
 * Created: 2/21/2015 5:53:29 AM
 *  Author: myQ_2
 */ 

#include "HardwareProfile.h"
#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#ifdef __cplusplus
extern "C" {
	#endif
	
	#include "timer1.h"	

	#ifdef __cplusplus
}
#endif

int main(void)
{
	DDRD = 0xC0;			//set PD7:6 to outputs for LEDs
	
	
	initTimer1(TIMER_DIV64, 4, 0x00FF);
	//uint8_t prescalar = TIMER_DIV64;
	//uint8_t WGMmode = 4;
	//uint16_t reload = 0x01FF;
	//
	//
	//
	//// The PRTIM1 bit in the Power reduction register must be written to zero to enable Timer/Counter1 module.
	//PRR &= ~(1<<PRTIM1);
	////sei();
	//
	//
	//
	//
 	//TCCR1A = (WGMmode & 0x03);		//normal timer operation, no PWM, etc
 	////TCCR1B = (prescalar << CS10 || ((WGMmode & 0x0C)<<1) ) ;
	//TCCR1B = ((WGMmode & 0x0C)<<WGM12) | (prescalar << CS10);
 	////TCCR1B = 0x0D ;
 	////TCCR1C = 0x00;		//force output compare off
	//OCR1A = reload;
	//TCNT1 = 0;
	//
	//TIFR1 = 1<<ICF1;			//Clear pending interrupts
	//
	//TIMSK1 |= (1 << TOIE1) |	// Enable
		//(1<<OCIE1A);				// Enable capture event timer
	//
	//
	//sei();
	RED_ON;
    while(1)
    {
//          if (TIFR1 & 1<< TOV1){
//   		   TIFR1 = (1<<TOV1);
//   		   PORTD ^= (1<<PD7);
//  	   }
// 	   if (TIFR1 & 1<< ICF1){
// 		   TIFR1 |= (1<<ICF1);
// 		   PORTD ^= (1<<PD7);
// 	   }
// 	   	   if (TIFR1 & 1<< OCF1A){
// 		   	   TIFR1 |= (1<<OCF1A);
// 		   	   PORTD ^= (1<<PD7);
// 	   	   }
			   if (isTimer1FlagSet()){
				   clearTimer1Flag();
				   PORTD ^= (1<<PD7);
			   }
    }
}

// ISR(TIMER1_CAPT_vect){
// 	PORTD ^= (1<<PD7);
// }