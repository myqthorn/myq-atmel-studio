/*
 * BluetoothTest.cpp
 *
 * Created: 9/6/2014 9:10:42 AM
 *  Author: myQ
 */ 

#define F_CPU 1000000L
#define BAUDRATE 9600
#define MYUBRR F_CPU/2/BAUDRATE - 1

#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>

//#ifdef __cplusplus
//extern "C" {
	//#endif	
	//#include "uart.h"
	//#ifdef __cplusplus
//}
//#endif

void USART_Init(unsigned int ubrr){
	//set baud rate
	UBRR0H = (unsigned char)(ubrr>>8);
	UBRR0L = (unsigned char)ubrr;
	
	//enable receiver and transmitter
	UCSR0B = (1<<RXEN0)|(1<<TXEN0);
	
	//set frame format: 8 data, 1 stop bit
	UCSR0C = (0<<USBS0)|(3<<UCSZ00);
}

void USART_Transmit(unsigned int data){
	//wait for empty transmit buffer
	while(!(UCSR0A & (1<<UDRE0)));
	
	UCSR0B &= -(1<<TXB80);
	if (data & 0x0100) UCSR0B |= (1<<TXB80);
	
	//put data into buffer, send data
	UDR0 = data;
}

int main(void)
{
	uint8_t i = 0;
	
	
	DDRB = 0X20;		//all inputs except B5 output
	
	USART_Init(MYUBRR);
	
	
	
    while(1)
    {
        _delay_ms(100);
		PORTB ^= (1<<PB5);
		USART_Transmit(7);
		USART_Transmit('\n');
    }
}