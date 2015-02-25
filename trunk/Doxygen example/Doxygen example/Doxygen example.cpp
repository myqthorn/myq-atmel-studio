/*
 * Doxygen_example.cpp
 *
 * Created: 11/1/2013 6:18:54 PM
 *  Author: myQ_2
 */ 

#include "HardwareProfile.h"
#include "nRF24L01.h"

#include <avr/io.h>
#include <avr/interrupt.h>


//global
nRF24L01 transmitter(1,2);			//must be global for interrupts to work

void config(void){
	//////////////////////////////////////////////////////////////////////////
	//Set up Interrupts
	//////////////////////////////////////////////////////////////////////////
	EIMSK |= (1<<INT0);			//turn on interrupt 0 (PD2)
	
	////External Interrupt Control Register A
	EICRA = 0x02;				//Interrupt on falling edge of INT0
	
	asm volatile("sei"::);		//enable interrupts
	//////////////////////////////////////////////////////////////////////////
}


int main(void)
{
	config();
	while(1)
    {
        
    }
}

ISR(INT0_vect){
	transmitter.interruptHandler();
	
}