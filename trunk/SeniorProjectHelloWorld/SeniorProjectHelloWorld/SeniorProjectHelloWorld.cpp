/*
 * SeniorProjectHelloWorld.cpp
 *
 * Created: 9/27/2014 11:46:10 AM
 *  Author: myQ
 */ 


#define F_CPU (8000000L)
#include <avr/io.h>
#include <util/delay.h>

#define IR_ON PORTD = (PORTD & ~0xC0)|(1<<PD6)
#define RED_ON PORTD = (PORTD & ~0xC0)|(1<<PD7)



void init(void);

int main(void)
{
	init();
				
    while(1)
    {
        _delay_ms(1000/30);
		IR_ON;
		
		_delay_ms(1000/30);
		RED_ON;
		
    }
}


#pragma region initialization

void init(void){
	
	//MCUCR = (1<<PUD);	//disable all pull ups
	
	//d6,d7 - LEDs
	DDRD = 0xC0;			//set PD7:6 to outputs for LEDs
							//PD2 as an input for switch
	RED_ON;					//Set PD7 high and PD6 low	
	
	
}

