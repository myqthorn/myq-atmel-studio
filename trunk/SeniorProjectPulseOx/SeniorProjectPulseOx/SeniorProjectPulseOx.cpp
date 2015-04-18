/*
 * SeniorProjectPulseOx.cpp
 *
 * Created: 10/1/2014 6:18:59 PM
 *  Author: Mike Litster
	Description: Pulse Oximeter. Transmits 7 bytes via Bluetooth as follows:
		1: counter
		2: MSB IR value
		3: LSB IR value
		4: MSB Red value
		5: LSB Red value
		6: MSB Ambient
		7: LSB Ambient
	
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

//size of pipe in settings.h sets size of queue, which is how many times we read
//the data before we send
#define NUM_QUEUE ((PIPE_OXYGEN_SATURATION_O2_SET_MAX_SIZE-1)/6)
#define SAMPLE_ORDER 0
NRF nrf;	//bluetooth module

bool buttonWasPressed = false;

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

int main(void){	
	//timer
	// 8MHz/90Hz			= 88,888.8888 cycles to count
	// 88,888.888 / 8 (div8)	= 11,111 = 0x2B67	
	//prescalar, mode, reload
	TIMER1 timer(TIMER_DIV8, 4, 0x2B67);
	
	//pin, prescalar
	ANALOG sensor(ADC_PIN, ADC_DIV64);
	
	enum state_t {INIT = 0x00, CONNECT, CONNECTING, SEND, RECEIVE, IDLE, GO2SLEEP, WAKEUP, WAKING};
	enum lights_t {I,R,O} led_state = R;
		 
	state_t state = INIT;
	uint8_t oxygenSaturationData[PIPE_OXYGEN_SATURATION_O2_SET_MAX_SIZE];// = {0,1,2,3,4,5,6};
	uint8_t count = 0;
	uint8_t readCount = 0;	//keeps track of how many times the ADC is read before it is averaged
	uint16_t readSum = 0;	//sums up the ADC values
	uint8_t settings = 0x00;	//settings is updated from the Bluetooth, so the Android device can
								//potentially change the settings
	
//in testing, we have a 16x4 character LCD screen
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
	DDRD = 0xE0;			//set PD7:6 to outputs for LEDs
	//PD2 as an input for switch
	RED_ON;					//Set PD7 high and PD6 low
	//while (1);
	//Switch
	PORTD = (1<<PD2);		//set PD2 to high(pullup resistor) for switch
	
	//External Interrupt Mask Register
	EIMSK |= (1<<INT0);	//turn on interrupt 0 (PD2)
	
	////External Interrupt Control Register A
	//EICRA = 0x02;		//Interrupt on falling edge of INT0
	EICRA = 0x00;		//Interrupt on low level of INT0
	
	//opamp is Powered Down when PD is low, initialize HIGH
	//250nS delay when powering up, 50nS delay when powering down (1/8MHz = 125nS)
	WakeOpAmp();
	sensor.clearFlag();
		
	_delay_ms(500);
#ifdef TESTMODE	
	lcdClearDisplay();
#endif	
	RED_ON; //TESTING
	
	while(1){		
		if (nrf.hasDataToProcess()){
			if (nrf.data[0] == PIPE_MODIFY_SETTINGS_PULSEOX_SETTING_RX){
				settings = nrf.data[1];				
			}
			nrf.dataHasBeenProcessed();
			if (settings == PULSEOX_GO_TO_SLEEP)
				state = GO2SLEEP;
		}
		
		//here we change the device's state to sleep or wakeup with a button press
		//the Bluetooth module's operational state is NRF_MODE_STANDBY
		if (nrf.mode == NRF_MODE_STANDBY){			
			if(buttonWasPressed){
				buttonWasPressed = false;
				//RED_ON; //testing
				if (state == GO2SLEEP){					
					state = WAKEUP;
				}else{				
					state = GO2SLEEP;
				}
			}
			
			switch (state){
				//INIT state is entered into upon bootup,
				//this is where the Bluetooth initialization instructions are
				//sent to the module
				case INIT:	//initializing
					if (!nrf.isInitializing()){
						timer.start();
						sei();	
						RED_ON;		
						//this will put it to sleep the first time it boots up	
						//state = GO2SLEEP;		//FINAL PRODUCT	
						
						//this will go straight into connect mode upon boot			
						state = CONNECT;		//TESTING			
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
				//	CONNECT state is used to send the connect command to the BT Module
				case CONNECT:
				RED_ON;	
					if (nrf.connect(180,0x0100) == 0x00)
						state = CONNECTING;
					break;				
				// CONNECTING state is used to wait until the BT module connects
				case CONNECTING:	
					if (nrf.isConnected())
						state = IDLE;
					break;
				
				// SEND state sends the ADC values to the BT module	
				case SEND:					
					if (0x00 == nrf.setLocalData(PIPE_OXYGEN_SATURATION_O2_SET, oxygenSaturationData, PIPE_OXYGEN_SATURATION_O2_SET_MAX_SIZE)){						
						oxygenSaturationData[0]++;									
						state = IDLE;	
					}					
					break;
				
				//RECEIVE state is used to receive the settings back from the Android device
				case RECEIVE:
					nrf.requestData(PIPE_MODIFY_SETTINGS_PULSEOX_SETTING_RX);   
					state = IDLE;
					break;	
								
				//IDLE is where the ADC is read and averaged												
				case IDLE:		//idle
					if(!nrf.isConnected()){
						state = GO2SLEEP;
					}else {
						if (timer.isCompareAFlagSet()){
							timer.clearCompareAFlag();
							timer.setCount(0);
							sensor.start();
 						}//if
 					
 						if (sensor.isInterruptFlagSet()){
							sensor.clearFlag();
							readSum += ADC;
							if (++readCount == 1<<SAMPLE_ORDER){
								readSum>>=SAMPLE_ORDER;
								switch(led_state){
									case I:
										oxygenSaturationData[6*count+4] = readSum & 0xFF;
										oxygenSaturationData[6*count+3] = readSum >> 8;
										LEDS_OFF;
										led_state = O;
										break;
									case O:
										oxygenSaturationData[6*count+6] = readSum & 0xFF;
										oxygenSaturationData[6*count+5] = readSum >> 8;
										RED_ON;
										led_state = R;
										break;
									case R:
										oxygenSaturationData[6*count+2] = readSum & 0xFF;
										oxygenSaturationData[6*count+1] = readSum >> 8;
										IR_ON;
										led_state = I;
										if (++count == NUM_QUEUE){
											count = 0;
											state = SEND;
										}
										break;
								} //switch(led_state)
							readSum = 0;
							readCount = 0;
							}else{	
							sensor.start();
							}//if (readCount)
						} //if (sensor.isInterruptFlagSet())						
					}	
					break;
					
				//Nighty night, little PulseOx don't let the bed bugs bite time to sip some current 
				//and save some battery. No more gulpin' down those milliamps.
				case GO2SLEEP:
					SleepOpAmp();
					sensor.off();
					//disconnect
					if (nrf.isConnected())
						nrf.disconnect(0x01);
					
					if (!nrf.isConnected()){
						//put nrf to sleep
						if (!nrf.isSleeping()){
							nrf.sleep();
						}																
						
						if (nrf.isSleeping() && !nrf.hasDataToSend()){							
							LEDS_OFF;
							//put uC to sleep							
  							SetSleepMode(SLEEP_MODE_POWER_DOWN);
  							EnableSleep();
  							__asm__ __volatile__ ("sleep" ::);
						}
					}
					break;
					
				//good morning, Dear, did you sleep well?
				//fire up the op amp and BT module
				case WAKEUP:
					DisableSleep();					
					
					//wakeup opamp
					WakeOpAmp();
					sensor.reinitialize();
					//wakeup nrf
					//if (nrf.isSleeping() && !nrf.hasDataToSend()){
						//nrf.wakeup();
					//}
					
					if (nrf.isSleeping()){
						//clear up any remnants from when we went to sleep
						nrf.status = (1<<NRF_RX_READY | 1<<NRF_SLEEPING);
						nrf.wakeup();	
						state = WAKING;					
					}
					break;
				case WAKING:
					
					if (!nrf.isSleeping() && !nrf.hasDataToSend()){						
						state = CONNECT;
					}
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
		nrf.process();		//handle all events for BT module
	}//while 1
}//main


//interrupt on button press
ISR(INT0_vect){
	//to debounce switch, wait 5ms, then check for low condition again
	_delay_ms(5);
	if (!(PIND & (1<<PD2)))
	buttonWasPressed = true;
	
}


