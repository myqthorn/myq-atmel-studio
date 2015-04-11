
#include <stdint.h>
#include <avr/io.h>		//for uint8_t definition

#ifndef HARDWAREPROFILE_H
	#define HARDWAREPROFILE_H

	
	//#define TESTMODE
	//#define PROGRAMSPACE
	
	//the Atmega168 performs 1 instruction per clock cycle, 8MHz = 8MIPS
	//at 8MHz, 1 instruction takes 125nS
	#define F_OSC 	(8000000L)
	#define F_CPU 	(8000000L)

	//PulseOx sensor LED's
	#define IR_ON 		DDRD |= (1<<PD5);PORTD = ((PIND & ~0xE0)|(1<<PD6)|(1<<PD5))
	#define RED_ON 		DDRD &= ~(1<<PD5); PORTD = ((PIND & ~0xC0)|(1<<PD7))
	#define LEDS_OFF 	DDRD &= ~(1<<PD5); PORTD = (PIND & ~0xE0)
	
	//ADC
	#define ADC_PIN		0
	
	//Power down for op-amp
	#define PD 				(PORTC & (1<<PC1))
	#define WakeOpAmp() 	PORTC |= (1<<PC1) 
	#define SleepOpAmp()	PORTC &= ~(1<<PC1)

	//Switch for interrupting from sleep
	#define SW 			(PORTD & (1<<PD2))

	//Communication with nRF8001 Bluetooth module
	#define ACT 		(ACT_PORT & (1<<ACT_PIN))
	#define ACT_PIN		PB0
	#define ACT_PORT	PORTB
	#define ACT_DDR		DDRB

	#define REQN		(REQN_PORT & (1<<REQN_PIN))
	#define REQN_PIN	1
	#define REQN_PORT	PORTB
	#define REQN_DDR	DDRB
	#define SetREQN()	REQN_PORT |= (1<<REQN_PIN)
	#define ClearREQN()	REQN_PORT &= ~(1<<REQN_PIN)

	#define RDYN		(RDYN_PORT & (1<<RDYN_PIN))
	#define RDYN_PIN	2
	#define	RDYN_PORT	PORTB
	#define RDYN_DDR	DDRB


	#define SCL_CLOCK (400000)

	#define JUMPERS_23008 (0b000) //User-defined hardware address of MCP23008 : A2, A1 & A0

	//PulseOx Commands from Android Device
	#define PULSEOX_GO_TO_SLEEP		0x01

#endif