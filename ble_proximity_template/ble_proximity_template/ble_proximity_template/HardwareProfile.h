#ifndef HARDWAREPROFILE_H
#define HARDWAREPROFILE_H

#include<stdint.h>

#define TESTMODE

//the Atmega168 performs 1 instruction per clock cycle, 8MHz = 8MIPS
//at 8MHz, 1 instruction takes 125nS
#define F_OSC 	(8000000L)
#ifndef F_CPU
#define F_CPU 	(8000000L)
#endif

//PulseOx sensor LED's
#define IR_ON 		PORTD = (PORTD & ~0xC0)|(1<<PD6)
#define RED_ON 		PORTD = (PORTD & ~0xC0)|(1<<PD7)
#define LEDS_OFF 	PORTD = (PORTD & ~0xC0)
#define TOGGLELEDS	PORTD = (PORTD & ~0XC0) | (PORTD ^ 0xC0);_delay_ms(500)
//Power down for op-amp
#define PD 				(PORTC & (1<<PC1))
#define WakeOpAmp() 	PORTC |= (1<<PC1) 
#define SleepOpAmp()	PORTC &= ~(1<<PC1)

//Switch for interrupting from sleep
#define SW 			(PORTD & (1<<PD2))

//Communication with nRF8001 Bluetooth module
#define ACT 		(PORTB & (1<<PB0))
#define PORT_ACT	PORTB
#define PIN_ACT		PB0
#define DDR_ACT		DDRB

#define REQN		(PORTB & (1<<PB1))
#define PORT_REQN	PORTB
#define PIN_REQN	PB0
#define DDR_REQN	DDRB
#define SetREQN()	PORTB |= (1<<PB1)
#define ClearREQN()	PORTB &= ~(1<<PB1)

#define RDYN		(PORT_RDYN & (1<<PIN_RDYN))
#define PORT_RDYN	PORTB
#define PIN_RDYN	PB2
#define DDR_RDYN	DDRB

#define NRF_RESET		(PORT_NRF_RESET & (1<<PIN_NRF_RESET))
#define PORT_NRF_RESET	PORTC
#define PIN_NRF_RESET	PC2
#define DDR_NRF_RESET	DDRC
#define SetNRFReset()	PORT_NRF_RESET |= (1<<PIN_NRF_RESET)
#define ClearNRFReset()	PORT_NRF_RESET &= ~(1<<PIN_NRF_RESET)

#define SCL_CLOCK (400000)

#define JUMPERS_23008 (0b000) //User-defined hardware address of MCP23008 : A2, A1 & A0


#endif //HARDWAREPROFILE_H