/*
 * SerialPractice.cpp
 *
 * Created: 5/17/2014 
 *  Author: myQ
 */ 

/********************************************************************************
Includes
********************************************************************************/
#include <avr/io.h>
#include <stdio.h>
#include <stdbool.h>

/********************************************************************************
Macros and Defines
********************************************************************************/
#define BAUD (19200)
#define MYUBRR (F_CPU/16/BAUD-1)
#define F_CPU (8000000)
/********************************************************************************
Function Prototypes
********************************************************************************/
void usart_init(uint16_t ubrr);
char usart_getchar( void );
void usart_putchar( char data );
void usart_pstr (char *s);
unsigned char usart_kbhit(void);

/********************************************************************************
Main
********************************************************************************/
int main( void ) {
	
	// setup PORTB data direction as an input
	DDRB = 0;
	// make sure it is high impedance and will not source
	PORTB = 0;
	// fire up the usart
	usart_init ( MYUBRR );
	// dump some strings to the screen at power on
	usart_pstr("Ready to rock and roll!\n\r");
	usart_pstr("Type in a character, and I will transpose it up by 1:\n\r");
	// main loop
	while(true) {
		// if a key has been pressed, then process it
		if(usart_kbhit()) {
			usart_putchar(usart_getchar() + 1);
		}

		
			usart_pstr("OUCH! Stop poking me!\n\r");
	
	}
}

/********************************************************************************
usart Related
********************************************************************************/
void usart_init( uint16_t ubrr) {
	// Set baud rate
	UBRR0H = (uint8_t)(ubrr>>8);
	UBRR0L = (uint8_t)ubrr;
	// Enable receiver and transmitter
	UCSR0B = (1<<RXEN0)|(1<<TXEN0);
	// Set frame format: 8data, 1stop bit
	UCSR0C = (1<<UMSEL00)|(3<<UCSZ00);
}
void usart_putchar(char data) {
	// Wait for empty transmit buffer
	while ( !(UCSR0A & (_BV(UDRE0))) );
	// Start transmission
	UDR0 = data;
}
char usart_getchar(void) {
	// Wait for incoming data
	while ( !(UCSR0A & (_BV(RXC0))) );
	// Return the data
	return UDR0;
}
unsigned char usart_kbhit(void) {
	//return nonzero if char waiting polled version
	unsigned char b;
	b=0;
	if(UCSR0A & (1<<RXC0)) b=1;
	return b;
}
void usart_pstr(char *s) {
	// loop through entire string
	while (*s) {
		usart_putchar(*s);
		s++;
	}
}