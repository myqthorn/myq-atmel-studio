/*
 * GccApplication1.c
 *
 * Created: 8/22/2013 7:49:51 PM
 *  Author: myQ
 */ 


#include <avr/io.h>
#include <util/delay.h>

int main(void)
{
	DDRB = 0XFF;
    while(1)
    {
        PORTB = 0xff;
        _delay_ms(15000);
        PORTB = 0x00;
        _delay_ms(500); 
    }
}