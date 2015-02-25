/*
 * main.c
 *
 * Created: 10/21/2013 9:50:17 PM
 *  Author: myQ
 */ 




#define F_CPU (8000000L)
#include <avr/io.h>
#include <util/delay.h>

#include"myqI2C.h"

void init(void){
	DDRB = 0xFF;		//all outputs port B
	i2c_init();
	
	
}

void writeRGB(unsigned char red, unsigned char green, unsigned char blue){
	i2c_start(RGB_I2C_ADDR<<1); //7-bit address plus r/w bit 0=write
	
	i2c_write(red);
	i2c_write(green);
	i2c_write(blue);
	i2c_stop();
}

int main(void)
{
	unsigned char red=255, blue=255, green=255;
	init();
	
	while(1)
	{
		PORTB ^= (1<<PB5);		//toggle LED
		_delay_ms(10);
		red -= 1;
		green -=5;
		blue -=10;
		writeRGB(red,green,blue);
			
		
	}//while
}//main

	