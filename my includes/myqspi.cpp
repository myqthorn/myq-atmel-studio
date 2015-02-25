	#include "myqspi.h"

	#include <avr/io.h>
	#include <avr/interrupt.h>
	
	
	SPI::SPI(){
		init(0x00);
	}
	SPI::SPI(uint8_t SPCRvalue){
		init(SPCRvalue);
	}
	void SPI::init(uint8_t SPCRvalue)
	// Initialize pins for spi communication
	{
		DDR_SPI &= ~((1<<DD_MOSI)|(1<<DD_MISO)|(1<<DD_SS)|(1<<DD_SCK));
		// Define the following pins as output
		DDR_SPI |= ((1<<DD_MOSI)|(1<<DD_SS)|(1<<DD_SCK));

		if (SPCRvalue == 0x00){
			SPCR = ((1<<SPE)|               // SPI Enable
			(0<<SPIE)|              // SPI Interrupt Enable
			(0<<DORD)|              // Data Order (0:MSB first / 1:LSB first)
			(1<<MSTR)|              // Master/Slave select
			(0<<SPR1)|(1<<SPR0)|    // SPI Clock Rate
			(0<<CPOL)|              // Clock Polarity (0:SCK low / 1:SCK hi when idle)
			(0<<CPHA));             // Clock Phase (0:leading / 1:trailing edge sampling)
		}
		else SPCR = SPCRvalue;
		//SPSR = (1<<SPI2X);              // Double Clock Rate
		
	}
	
	void SPI::transfer (uint8_t * dataout, uint8_t * datain, uint8_t length)
	// Shift full array through target device
	{
		uint8_t i;
		for (i = 0; i < length; i++) {
			SPDR = dataout[i];
			while((SPSR & (1<<SPIF))==0);
			datain[i] = SPDR;
		}
	}
	
	void SPI::read (uint8_t * datain, uint8_t length)
	// sends out 0x00 while receiving full length of datain
	{
		uint8_t i;
		for (i = 0; i < length; i++) {
			SPDR = 0x00;					//dummy byte
			while((SPSR & (1<<SPIF))==0);	//wait until data is transferred
			datain[i] = SPDR;
		}
	}
	
	void SPI::write (uint8_t * dataout, uint8_t length)
	// Shift full array to target device without receiving any byte
	{
		uint8_t i;
		for (i = 0; i < length; i++) {
			SPDR = dataout[i];
			while((SPSR & (1<<SPIF))==0);
		}
	}
	
	uint8_t SPI::transfer1byte (uint8_t dataout)
	// Clocks only one byte to target device and returns the received one
	{
		SPDR = dataout;
		while((SPSR & (1<<SPIF))==0);
		return SPDR;
	}
	
	void SPI::setBitOrder(uint8_t order){
		SPCR = (SPCR & ~(1<<DORD)) | ((order & 0x01)<<DORD);
	}
	void SPI::setClockDivider(uint8_t divider){
		uint8_t clockRate = divider & 0b11;
		uint8_t doubleClock = divider>>2;
		SPCR = ((SPCR & ~(0b11<<SPR0))| (clockRate<<SPR0));
		SPSR = ((SPSR & ~(1<<SPI2X)) | (doubleClock<<SPI2X));
	}
	void SPI::setDataMode(uint8_t mode){
		SPCR = (SPCR & ~(11<<CPHA)) | ((mode & 0b11)<<CPHA);
		
	}