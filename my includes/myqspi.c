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

#include "myqspi.h"

#include <avr/io.h>
#include <avr/interrupt.h>





void spiInit()
// Initialize pins for spi communication
{
    DDR_SPI &= ~((1<<DD_MOSI)|(1<<DD_MISO)|(1<<DD_SS)|(1<<DD_SCK));
    // Define the following pins as output
    DDR_SPI |= ((1<<DD_MOSI)|(1<<DD_SS)|(1<<DD_SCK));

    
    SPCR = ((1<<SPE)|               // SPI Enable
            (0<<SPIE)|              // SPI Interrupt Enable
            (0<<DORD)|              // Data Order (0:MSB first / 1:LSB first)
            (1<<MSTR)|              // Master/Slave select   
            (0<<SPR1)|(1<<SPR0)|    // SPI Clock Rate
            (0<<CPOL)|              // Clock Polarity (0:SCK low / 1:SCK hi when idle)
            (0<<CPHA));             // Clock Phase (0:leading / 1:trailing edge sampling)

    //SPSR = (1<<SPI2X);              // Double Clock Rate
    
}

void spiTransfer (uint8_t * dataout, uint8_t * datain, uint8_t length)
// Shift full array through target device
{
       uint8_t i;      
       for (i = 0; i < length; i++) {
             SPDR = dataout[i];
             while((SPSR & (1<<SPIF))==0);
             datain[i] = SPDR;
       }
}

void spiRead (uint8_t * datain, uint8_t length)
// sends out 0x00 while receiving full length of datain
{
	uint8_t i;
	for (i = 0; i < length; i++) {
		SPDR = 0x00;					//dummy byte
		while((SPSR & (1<<SPIF))==0);	//wait until data is transferred
		datain[i] = SPDR;
	}
}

void spiWrite (uint8_t * dataout, uint8_t length)
// Shift full array to target device without receiving any byte
{
       uint8_t i;      
       for (i = 0; i < length; i++) {
             SPDR = dataout[i];
             while((SPSR & (1<<SPIF))==0);
       }
}

uint8_t spiTransfer1byte (uint8_t dataout)
// Clocks only one byte to target device and returns the received one
{
    SPDR = dataout;
    while((SPSR & (1<<SPIF))==0);
    return SPDR;
}

