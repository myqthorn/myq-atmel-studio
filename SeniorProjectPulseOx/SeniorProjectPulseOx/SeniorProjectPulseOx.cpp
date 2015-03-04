/*
 * SeniorProjectPulseOx.cpp
 *
 * Created: 10/1/2014 6:18:59 PM
 *  Author: myQ
 */ 
#include "HardwareProfile.h"

#ifdef TESTMODE
#include <stdio.h>		//for lcd msg
//only include LCD display when testing

/////////////////////////////////////////////////////////////
//this block is necessary to include C files in a C++ project
/////////////////////////////////////////////////////////////
#ifdef __cplusplus
extern "C" {	
#include "23008LCD.h"
}
#else
#include "23008LCD.h"
#endif
/////////////////////////////////////////////////////////////
char line1[40], line2[40];         // LCD line arrays
void displayLCD(void){	
	lcdGoto(0,0);
	lcdPuts(line1);		// Display line 1
	lcdGoto(0,1);		// Skip to next line
	lcdPuts(line2);	
}
#endif	//TESTMODE
 
#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include "nrf8001.h"
#include "services.h"
#include "timer1.h"
#include "myqADC.h"
//#include "hal_aci_tl.h"

//size of pipe in settings.h sets size of queue, which is how many times we read
//the data before we send
#define NUM_QUEUE ((PIPE_OXYGEN_SATURATION_O2_SET_MAX_SIZE-1)/6)

NRF nrf;
//#define HEART_RATE_DATA_LENGTH 2

void SetSleepMode(uint8_t mode);

int main(void){	
	//timer
	// 8MHz/90Hz			= 88,888.8888 cycles to count
	// 88,888.888 / 8 (div8)	= 11,111 = 0x2B67
	
	//prescalar, mode, reload
	TIMER1 timer(TIMER_DIV8, 4, 0x2B67);
	
	//pin, prescalar
	ANALOG sensor(ADC_PIN, ADC_DIV64);
	
	enum state_t {INIT = 0x00, RESET, CONNECT, CONNECTING, SEND, RECEIVE, IDLE};
	enum lights_t {I,R,O} led_state = R;
		
	uint8_t count = 0;
		 
	state_t state = INIT;
	uint8_t oxygenSaturationData[PIPE_OXYGEN_SATURATION_O2_SET_MAX_SIZE];// = {0,1,2,3,4,5,6};
	uint8_t count = 0;
	uint8_t settings = 0x00;
	
#ifdef TESTMODE
	//////////////////////////////////////////////////////////////////////////
	//LCD initialization
	//////////////////////////////////////////////////////////////////////////
	//asm volatile ("cli"::);		//deactivate interrupts
	lcdInit();
	lcdClearDisplay();
	sprintf(line1, " Senior Project");
	sprintf(line2, "BT PulseOx ");
	displayLCD();
	//////////////////////////////////////////////////////////////////////////
#endif
	
	_delay_ms(500);
	
	//d6,d7 - LEDs
	DDRD = 0xC0;			//set PD7:6 to outputs for LEDs
	//PD2 as an input for switch
	RED_ON;					//Set PD7 high and PD6 low
	
	//Switch
	PORTD |= (1<<PD2);		//set PD2 to high(pullup resistor) for switch
	
	//opamp is Powered Down when PD is low, initialize HIGH?
	//250nS delay when powering up, 50nS delay when powering down (1/8MHz = 125nS)
	WakeOpAmp();
	sensor.clearFlag();
		
	_delay_ms(500);
#ifdef TESTMODE	
	lcdClearDisplay();
#endif	
	
	while(1)
	{
		if (nrf.hasDataToProcess()){
			if (nrf.data[0] == PIPE_MODIFY_SETTINGS_PULSEOX_SETTING_RX){
				settings = nrf.data[1];				
			}
			nrf.dataHasBeenProcessed();
		}
		
		if (nrf.mode == NRF_MODE_STANDBY){
			
			
			switch (state){
				case INIT:	//initializing
					if (!nrf.isInitializing()){
						timer.start();
						state = CONNECT;						
					}
#ifdef TESTMODE
					//display
					//sprintf(line1, " %02X%02X %02X %02X%02X ", nrf.RxCount, nrf.dataCredit, state, nrf.status, nrf.mode);
					sprintf(line2, "%02X%02X %02X %02X%02X ", nrf.RxCount, nrf.dataCredit, state, nrf.status, nrf.mode);
					displayLCD();
#else
					//_delay_ms(25);
#endif
					break;
				case RESET:	//temp
					//TODO:
					nrf.radioReset();
						state = IDLE;
					break;
					
				case CONNECT:	//connect
					if (nrf.connect(180,0x0100) == 0x00)
						state = CONNECTING;
					break;				
				
				case CONNECTING:	
					if (nrf.isConnected())
						state = IDLE;
					break;
					
				case SEND:					
					if (0x00 == nrf.setLocalData(PIPE_OXYGEN_SATURATION_O2_SET, oxygenSaturationData, PIPE_OXYGEN_SATURATION_O2_SET_MAX_SIZE)){						
						oxygenSaturationData[0]++;						
						
						state = IDLE;	
					}					
					break;
				
				case RECEIVE:
					nrf.requestData(PIPE_MODIFY_SETTINGS_PULSEOX_SETTING_RX);
					state = IDLE;
					break;				
																
				case IDLE:		//idle
					if(!nrf.isConnected()){
						state = CONNECT;
					}
					
					//reset
					//if (SW == 0)//TODO: button press resets device
						//state == RESET;
						
					
					else {
						if (timer.isCompareAFlagSet()){
							timer.clearCompareAFlag();
							timer.setCount(0);
						
 							//if( (!sensor.isReading()) && (!sensor.isInterruptFlagSet()) )
 								sensor.start();
 						
 						}//if
 					
 						//if (!sensor.isReading()){
						if (sensor.isInterruptFlagSet()){
							//timer.clearCompareAFlag();
							//timer.setCount(0);
							sensor.clearFlag();
							switch (led_state){
								case I:
									oxygenSaturationData[6*count+4] = ADCL;
									oxygenSaturationData[6*count+3] = ADCH;
									LEDS_OFF;
									led_state = O;
									break;
								case O:
									oxygenSaturationData[6*count+6] = ADCL;
									oxygenSaturationData[6*count+5] = ADCH;
									RED_ON;
									led_state = R;
									break;
								case R:
									oxygenSaturationData[6*count+2] = ADCL;
									oxygenSaturationData[6*count+1] = ADCH;
									IR_ON;
									led_state = I;
									if (++count == NUM_QUEUE){
										count = 0;
										state = SEND;
									}
									break;
							
							} //switch	
						} //if (isADCfinished())	
					
					}
					//receive settings
						//TODO:
					//send		
						//TODO:
						
					break;
				default:
					//state = idle;
					break;
			}//switch(state)
		}//if (nrf.mode == NRF_MODE_STANDBY)


#ifdef TESTMODE
		//display
		//sprintf(line1, " %02X%02X %02X %02X%02X ", nrf.RxCount, nrf.dataCredit, state, nrf.status, nrf.mode);
		sprintf(line2, "%02X%02X %02X %02X%02X ", nrf.RxCount, /*nrf.dataCredit,*/ state, nrf.status, nrf.mode, nrf.lastCommand);
		displayLCD();
#else
		if (nrf.mode == NRF_MODE_SETUP)
			_delay_ms(15);
#endif			

		nrf.process();
		
	}//while 1
}//main



#pragma region sleep

#define SLEEP_MODE_IDLE			0b000
#define SLEEP_MODE_ADC			0b001
#define SLEEP_MODE_POWER_DOWN	0b010
#define SLEEP_MODE_POWER_SAVE	0b011
#define SLEEP_MODE_STANDBY		0b110
#define EnableSleep() 			SMCR |= (1<<SE)
#define DisableSleep()			SMCR &= ~(1<<SE)
void SetSleepMode(uint8_t mode){
		if((mode == SLEEP_MODE_STANDBY) | (mode<=SLEEP_MODE_POWER_SAVE))
			SMCR = (mode<<SM0);	
}


//Sleep should be done in this order: 
//	1. SetSleepMode();
//	2. EnableSleep();
//  3. SLEEP;
//	4. After waking, DisableSleep()


#pragma endregion sleep




