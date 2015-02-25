/*
 * nunchuk_receiver.cpp
 *
 * Created: 11/21/2013 8:08:32 PM
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
	
#include "23008LCD.h"

#ifdef __cplusplus
}
#endif
/////////////////////////////////////////////////////////////

#include <avr/io.h>
#include <stdio.h>		//for lcd msg
#include <util/delay.h>

//declare transmitter as global for interrupts
//channel 1, payload length 6
NRF transmitter(1,6);
uint8_t payload[6] = {0,0,0,0,0,0};

unsigned int  Z, C;							// Buttons on nunchuk
char line1[40], line2[40];         // LCD line arrays

void test();

void displayLCD(void){
	
	lcdGoto(0,0);
	lcdPuts(line1);		// Display line 1
	lcdGoto(0,1);		// Skip to next line
	lcdPuts(line2);
	
}

void init(void){
	uint8_t address[5] = {0x42,0x42,0x42,0x42,0x42};		
	
	//////////////////////////////////////////////////////////////////////////
	//nRF Initialization
	//////////////////////////////////////////////////////////////////////////
	//http://gizmosnack.blogspot.com/2013/04/tutorial-NRF-and-avr.html
	//nRF inits
	
	//EN_AA - enable auto-acknowledgments on data pipe 0
	//	default is all on (0x3F)
	//transmitter.enableAutoAcknowledge(1<<NRF_ENAA_P0);
	
	//EN_RXADDR choose number of enabled data pipes (1-5)
	// datapipes 0 and 1 are enabled by default
	//transmitter.enableRXDatapipes(1<<NRF_ERX_P0);
	
	//SETUP_AW sets address width for all data pipes
	transmitter.setAddressWidth(NRF_AW_5BYTES);
	
	//choose power mode and data speed (1 vs 2 MHz or 250KHz)
	transmitter.setupRF(NRF_RF_DR_2MHZ | NRF_RF_PWR_0DB);
	
	//disable retransmit, nunchuk updates continually, lost data is trivial
	transmitter.setupRetransmit(0x00);
	
	//set receiver address (set RX_ADDR_P0 = TX_ADDR if EN_AA is enabled!!)
	transmitter.setRXAddress(0,address);
	
	//RX_ADDR_P1-05
	//Only one RX address is being used for this application.
	//transmitter.setRXAddress(1,address);   //etc, etc
	
	//TX_ADDR this needs to be the same address as is assigned to the receiving device's RX_ADDR
	transmitter.setTXAddress(address);
	
	//set payload length of data pipe 0 (6 bytes per package)
	transmitter.setPayloadLength(0,6);
	//transmitter.setPayloadLength(1,32);  ///etc, etc
	
	//setup channel
	transmitter.setChannel(1);
	
	//mask interrupts - default values.  Commented out as a placeholder.
	transmitter.maskIRQ(0<<NRF_MASK_RX_DR | 1<<NRF_MASK_TX_DS | 1<<NRF_MASK_MAX_RT);
	
	//Set CRC to 2-byte
	transmitter.setCRC(NRF_CRC_2BYTE);
	
	//set device to be primary transmitter and power up.
	transmitter.setMode(NRF_MODE_RX);
	_delay_ms(100);
	transmitter.powerUp();

	
	//////////////////////////////////////////////////////////////////////////
}

int main(void){
	
	char mode = 'T';
	uint8_t config = 0, cd = 0;;
	
	extern void clearCE();
	extern void setCE();	
	
	//////////////////////////////////////////////////////////////////////////
	//LCD initialization
	//////////////////////////////////////////////////////////////////////////
	//asm volatile ("cli"::);		//deactivate interrupts
	lcdInit();
	lcdClearDisplay();
	sprintf(line1, "myQ Atmega168 Dev Board rev0");
	sprintf(line2, "nRF24L01+ Receiver");
	displayLCD();
	//////////////////////////////////////////////////////////////////////////
	
	init();
	
	toggleLED1();
	PORTB |= (1<<PB7);
	_delay_ms(1000);
	PORTB &= ~(1<<PB7);
	toggleLED1();
	lcdClearDisplay();
	
	asm volatile ("sei"::);		//activate interrupts
	setCE();
	
	while(1){				
		
		//check if nRF has been reset to defaults, then reinitialize if necessary
		config = transmitter.getConfig();
		if ((config & 0x0F) != 0x0F) init();
						
		if (config & (1<<NRF_PRIM_RX))
			mode = 'R';
			else mode = 'T';
			
		//cd = transmitter.receivedPowerDetected();
		
		Z = (payload[5] & 0b1) ^ 0b1;				// Extract Z button bit
		C = (payload[5]>>1 & 0b1) ^ 0b1;			// Extract C button bit
		
		asm volatile ("cli"::);//deactivate interrupts
		toggleLED1();
		sprintf(line1, "SX%02X Y%02X C%d Z%d",		// Format joystick and
		payload[0], payload[1], C, Z);				//  button data
		sprintf(line2,"X%02X Y%02X Z%02X",			// Format accelerometer
		payload[2], payload[3], payload[4]);		//  data
		displayLCD();		
		asm volatile ("sei"::);//activate interrupts
			
		//Flush RX buffer to avoid filling up
		transmitter.flushRX();
		
		//test();
		
	}//while
}//main

void test(){

//////////////////////////////////////////////////////////////////////////
	//test variables - delete when finished
	
	//uint8_t test1 = 0,
			//test2 = 0,
			//test3 = 0,
			//address1[5];
	//////////////////////////////////////////////////////////////////////////
	

//test maskIRQ function			*****Passed**********	
		//if (transmitter.getConfig() & 1<<NRF_MASK_MAX_RT)
			//transmitter.maskIRQ(0<<NRF_MASK_MAX_RT);
			//else transmitter.maskIRQ(1<<NRF_MASK_MAX_RT);
		
		//test interrupt				
		//_delay_ms(100);
		PORTB &= ~(1<<PB7);		//Turn off LED (turned on in interrupt)
		
		//test getChannel and receivedPowerDetected
		//channel = transmitter.getChannel();
		//carrierDetect = transmitter.receivedPowerDetected();
		
		////test flushTX
			////populate TX FIFO
			//transmitter.setMode(NRF_MODE_TX);			// Set to transmitter mode
			//transmitter.flushTX();							// Flush TX FIFO
			//clearCE();	//prevent payload from being sent
			//transmitter.write(NRF_W_TX_PAYLOAD,	payload, 6);	//write payload
			////check if is empty
			//TXisEmptyPre = transmitter.isTXEmpty();
			////flush TX FIFO
			//transmitter.flushTX();	
			////check if is empty
			//TXisEmptyPost = transmitter.isTXEmpty();
			
		//test 
			//test1 = transmitter.read(NRF_SETUP_RETR);
			//test1 = transmitter.;
			
			
			//test2 = transmitter.;
			
		////test powerup/powerDown ****Passed****
			//transmitter.powerDown();
			//test1 = transmitter.getConfig();
			//test2 = transmitter.isPoweredUp();
			//transmitter.powerUp();
			//test3 = transmitter.isPoweredUp();
			//sprintf(line2, "PD%02X PU%02X PU%02X",
			//test1, test2, test3);
			
		////test isTXFIFOFUll flushTX  ****Passed****
			//asm volatile ("cli"::);		//deactivate interrupts
			//clearCE();
			//transmitter.setMode(NRF_MODE_TX);			// Set to transmitter mode
			//transmitter.flushTX();
			//transmitter.write(NRF_W_TX_PAYLOAD,	payload, 6);	//write payload
			//test1 = transmitter.isTXFIFOFull();
			//transmitter.write(NRF_W_TX_PAYLOAD,	payload, 6);	//write payload
			//transmitter.write(NRF_W_TX_PAYLOAD,	payload, 6);	//write payload
			//test2 = transmitter.isTXFIFOFull();
			//transmitter.flushTX();
			//test3 = transmitter.isTXFIFOFull();
			//sprintf(line2, "DR%02X D2%02X %02X",
				//test1, test2, test3);
			//asm volatile ("sei"::);		//activate interrupts
			
		////test getLostPacketCount()
			//test1 = transmitter.getLostPacketCount();
			//transmitter.sendPayload(payload, 6)	;
			//test2 = transmitter.getLostPacketCount();
			//transmitter.sendPayload(payload, 6)	;
			//test3 = transmitter.getLostPacketCount();
			//transmitter.sendPayload(payload, 6)	;
			//sprintf(line2, "DR%02X D2%02X %02X %cX",
			//test1, test2, test3, mode);
			//config = transmitter.getConfig();
			
		//test receivedPowerDetected
			//transmitter.setChannel(transmitter.getChannel()+1);
			//_delay_ms(10);
			//test1 = transmitter.getChannel();
			//test2 = transmitter.receivedPowerDetected();
			//
			//sprintf(line2, "CH%02X CD%02X %02X %cX",
			//test1, test2, test3, mode);
			
		////test setRXAddress set TXAddress  ****Passed****
			//transmitter.read(NRF_TX_ADDR, address1, 5);	
				//
			//sprintf(line2, "%02X %02X %02X %02X %02X",
			//address1[0], address1[1], address1[2], address1[3], address1[4]);
			
		//test setPayloadLength  ****Passed****
			//test1 = transmitter.read(NRF_RX_PW_P0);
			//test2 = transmitter.read(NRF_RX_PW_P1);
			//test3 = transmitter.read(NRF_RX_PW_P2);
			//config = transmitter.getConfig();
			//sprintf(line2, "%02X %02X %02X %cX",
			//test1, test2, test3, mode);
			
		//test isTXFUll			****Passed****
			//asm volatile ("cli"::);		//deactivate interrupts
			//clearCE();
			//transmitter.setMode(NRF_MODE_TX);			// Set to transmitter mode
			//transmitter.flushTX();
			//transmitter.write(NRF_W_TX_PAYLOAD,	payload, 6);	//write payload
			//test1 = transmitter.isRXEmpty();
			//transmitter.write(NRF_W_TX_PAYLOAD,	payload, 6);	//write payload
			//transmitter.write(NRF_W_TX_PAYLOAD,	payload, 6);	//write payload
			//test2 = transmitter.isRXFull();
			//transmitter.flushTX();
			//test3 = transmitter.isTXEmpty();
			//sprintf(line2, "%02X %02X %02X %cX",
			//test1, test2, test3, mode);
			//asm volatile ("sei"::);		//activate interrupts
			
		//test enableDynamicPayloadLength   ****Passed****
			//transmitter.enableDynamicPayloadLength(0x05)	;
			//test1 = transmitter.read(NRF_DYNPD);
			//transmitter.enableDPL();
			//transmitter.enableACKPayload();
			//transmitter.enableDynamicACK();
			//test2 = transmitter.read(NRF_FEATURE);
			//transmitter.clearFeature();
			//test3 = transmitter.read(NRF_FEATURE);
			//sprintf(line2, "%02X %02X %02X %cX",
			//test1, test2, test3, mode);
			
		//////////////////////////////////////////////////////////////////////////
		//test output
			//payload, config and status
			//sprintf(line1, "PL%02X CF%02X ST%02X",
			//payload[1], config, transmitter.status);
		
		
		
		//////////////////////////////////////////////////////////////////////////
				
		
		
		//asm volatile ("cli"::);		//disable interrupts

}

