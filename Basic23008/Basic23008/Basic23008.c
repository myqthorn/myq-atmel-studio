/*
 * Basic23008.c
 *
 * Created: 10/6/2013 7:19:13 AM
 *  Author: myQ
 */ 


#include "HardwareProfile.h"
#include "portexpander.h"
#include "23008LCD.h"

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>

int main(void)
{
	DDRB = 0xFF;		//all outputs port B
	lcdInit();	
	lcdClearDisplay();
	
	lcdGoto(0,0);
	lcdPuts("Atmega168");
	lcdGoto(0,1);
	lcdPuts("Dev Board rev0");
		
    while(1)
    {
		PORTB ^= (1<<PB5);		//toggle LED
		toggleLED1();
		toggleLED2();
		lcdWrite(CMD_REG, LCD_SHIFT | LCD_SHIFT_DISPLAY | LCD_SHIFT_LEFT);
		_delay_ms(50);
		
		
    }//while
}//main