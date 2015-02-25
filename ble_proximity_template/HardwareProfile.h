#include<stdint.h>

#define TESTMODE

//the Atmega168 performs 1 instruction per clock cycle, 8MHz = 8MIPS
//at 8MHz, 1 instruction takes 125nS
#define F_OSC 	(8000000L)
#define F_CPU 	(8000000L)

//PulseOx sensor LED's
#define IR_ON 		PORTD = (PORTD & ~0xC0)|(1<<PD6)
#define RED_ON 		PORTD = (PORTD & ~0xC0)|(1<<PD7)
#define LEDS_OFF 	PORTD = (PORTD & ~0xC0)

//Power down for op-amp
#define PD 				(PORTC & (1<<PC1))
#define WakeOpAmp() 	PORTC |= (1<<PC1) 
#define SleepOpAmp()	PORTC &= ~(1<<PC1)

//Switch for interrupting from sleep
#define SW 			(PORTD & (1<<PD2))

//Communication with nRF8001 Bluetooth module
#define ACT 		(PORTB & (1<<PB0))
#define REQN		(PORTB & (1<<PB1))
#define SetREQN()	PORTB |= (1<<PB1)
#define ClearREQN()	PORTB &= ~(1<<PB1)
#define RDYN		(PORTB & (1<<PB2))

#define SCL_CLOCK (400000)

#define JUMPERS_23008 (0b000) //User-defined hardware address of MCP23008 : A2, A1 & A0