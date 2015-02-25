/*
    Copyright (c) 2013 Mike Litster <myq.thorn@gmail.com>

    Permission is hereby granted, free of charge, to any person 
    obtaining a copy of this software and associated documentation 
    files (the "Software"), to deal in the Software without 
    restriction, including without limitation the rights to use, copy, 
    modify, merge, publish, distribute, sublicense, and/or sell copies 
    of the Software, and to permit persons to whom the Software is 
    furnished to do so, subject to the following conditions:

    The above copyright notice and this permission notice shall be 
    included in all copies or substantial portions of the Software.

    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, 
    EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF 
    MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND 
    NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT 
    HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, 
    WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, 
    OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER 
    DEALINGS IN THE SOFTWARE.

 */

#ifndef _MYQSPI_H_
#define _MYQSPI_H_

#include <avr/io.h>
#define PORT_SPI    PORTB
#define SCK			PB5
#define MISO		PB4
#define MOSI		PB3
#define SS			PB2

#define DDR_SPI     DDRB
#define DD_SCK      DDB5
#define DD_MISO     DDB4
#define DD_MOSI     DDB3
#define DD_SS       DDB2

#define SPI_MSBFIRST 0b0
#define SPI_LSBFIRST 0b1

#define SPI_MODE0	0b00
#define SPI_MODE1	0b01
#define SPI_MODE2	0b10
#define SPI_MODE3	0b11


#define SPI_CLOCK_DIV4		0b000
#define SPI_CLOCK_DIV16		0b001
#define SPI_CLOCK_DIV64		0b010
#define SPI_CLOCK_DIV128	0b011
#define SPI_CLOCK_DIV2		0b100
#define SPI_CLOCK_DIV8		0b101
#define SPI_CLOCK_DIV32		0b110
//#define SPI_CLOCK_DIV64		0b111

#ifndef __cplusplus

extern void spiInit();
extern void spiTransfer (uint8_t * dataout, uint8_t * datain, uint8_t length);
extern void spiRead (uint8_t * datain, uint8_t length);
extern void spiWrite (uint8_t * dataout, uint8_t length);
extern uint8_t spiTransfer1byte (uint8_t dataout);

#else
class SPI
{
	public:
		SPI();
		SPI(uint8_t SPCRvalue);
		void init(uint8_t SPCRvalue);
		void transfer(uint8_t * dataout, uint8_t * datain, uint8_t length);
		void read(uint8_t * datain, uint8_t length);
		void write(uint8_t * dataout, uint8_t length);
		uint8_t transfer1byte(uint8_t dataout);
		void setBitOrder(uint8_t order);
		void setClockDivider(uint8_t divider);
		void setDataMode(uint8_t mode);
	private:
	
};

#endif

#endif /* _SPI_H_ */
