/*
 * nunchuk_transmitter.cpp
 *
 * Created: 11/13/2013 7:25:19 PM
 *  Author: Mike Litster
 */ 

#include "HardwareProfile.h"
#include "NRF24L01.h"
/////////////////////////////////////////////////////////////
//this block is necessary to include C files in a C++ project
/////////////////////////////////////////////////////////////
#ifdef __cplusplus
extern "C" {
#endif
	
#include "nunchuk.h"

#ifdef __cplusplus
}
#endif
/////////////////////////////////////////////////////////////

#include <avr/io.h>
#include <util/delay.h>

#define heartbeatInit()		DDRD |= (1<<PD7)
#define heartbeatOn()		PORTD |= (1<<PD7)
#define heartbeatOff()		PORTD &= ~(1<<PD7)

//declare transmitter as global for interrupts
//channel 1, payload length 6
NRF transmitter(1,6);
uint8_t payload[6];

unsigned char nunchuk[6]={0,1,2,3,4,5};
	
void init(void){
	uint8_t address[5] = {0x42,0x42,0x42,0x42,0x42};
		
	//http://gizmosnack.blogspot.com/2013/04/tutorial-NRF-and-avr.html
	//http://www.insidegadgets.com/2012/08/22/using-the-NRF-wireless-module/
	
	//////////////////////////////////////////////////////////////////////////
	//nRF inits
	//////////////////////////////////////////////////////////////////////////
	
	//setup channel
	transmitter.setChannel(1);
	
	//set payload length of data pipe 0 (6 bytes per package)
	transmitter.setPayloadLength(0,6);
	
	//set receiver address (set RX_ADDR_P0 = TX_ADDR if EN_AA is enabled!!)
	transmitter.setRXAddress(0,address);
	
	//RX_ADDR_P1-05
	//Only one RX address is being used for this application.
	//transmitter.setRXAddress(1,address1);   //etc, etc
	
	//TX_ADDR this needs to be the same address as is assigned to the receiving device's RX_ADDR
	transmitter.setTXAddress(address);
	
	//EN_RXADDR choose number of enabled data pipes (1-5)
	// datapipes 0 and 1 are enabled by default
	//transmitter.enableRXDatapipes(1<<NRF_ERX_P0);
	
		//EN_AA - enable auto-acknowledgments on data pipe 0
		//	default is all on (0x3F)
		//transmitter.enableAutoAcknowledge(1<<NRF_ENAA_P0);	
		
		//SETUP_AW sets address width for all data pipes
		transmitter.setAddressWidth(NRF_AW_5BYTES);
		
		//choose power mode and data speed (1 vs 2 MHz or 250KHz)
		transmitter.setupRF(NRF_RF_DR_2MHZ | NRF_RF_PWR_0DB);
		
		//disable retransmit, nunchuk updates continually, lost data is trivial
		transmitter.setupRetransmit(0x00);		
	
		//mask interrupts - default values.  Commented out as a placeholder.
		//transmitter.maskIRQ(0<<NRF_MASK_RX_DR | 0<<NRF_MASK_TX_DS | 0<<NRF_MASK_MAX_RT);
		
		//Set CRC to 2-byte
		transmitter.setCRC(NRF_CRC_2BYTE);
		
		//set device to be primary transmitter and power up.
		transmitter.setMode(NRF_MODE_TX);
		_delay_ms(100);
		transmitter.powerUp();	
}

int main(void)
{
	uint8_t config = 0;
	init();
	InitNunchuk();	
	heartbeatInit();
	
	//toggle interrupt LED check
	PORTB |= (1<<PB7);
	_delay_ms(100);
	PORTB &= ~(1<<PB7);
	
	//heartbeat LED check
	heartbeatOn();
	_delay_ms(100);
	heartbeatOff();
	
	asm volatile ("sei"::);		//activate interrupts
    while(1)
    {
		//check if nRF has been reset to defaults, then reinitialize if necessary
		config = transmitter.getConfig();
		if ((config & 0x0E) != 0x0E) init();
		
        transmitter.flushRX();
		transmitter.flushTX();			
		transmitter.sendPayload(nunchuk, 6);
		//_delay_ms(10);
				
		PORTB &= ~(1<<PB7);			//turn off interrupt LED
		
		//heartbeat LED
		//heartbeatOn();
		//_delay_ms(1000);
		//heartbeatOff();
		//_delay_ms(1000);
		
		asm volatile ("cli"::);		//deactivate interrupts
		ReadNunchuk(nunchuk);
		asm volatile ("sei"::);		//activate interrupts
		//if( ( (nunchuk[5] & 0b1) ^0b1)==1)
		//heartbeatOn();
		//else heartbeatOff();
		
		//if (transmitter.status & 0x70)
			//heartbeatOn();
			//else heartbeatOff();		
	}//while
}//main
