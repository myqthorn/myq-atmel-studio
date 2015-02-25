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
#endif
	
#include "23008LCD.h"

#ifdef __cplusplus
}
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

#ifdef __cplusplus
extern "C" {
	#endif
	
	#include "timer1.h"
	#include "myqADC.h"

	#ifdef __cplusplus
}
#endif


//#include "hal_aci_tl.h"

NRF nrf;


#define HEART_RATE_DATA_LENGTH 2

void SetSleepMode(uint8_t mode);
void init(void);

int main(void)
{	
	
	enum state_t {INIT = 0x00, RESET, CONNECT, CONNECTING, SEND, RECEIVE, IDLE};
		 
	enum lights_t {I,R,O} led_state = R;
		
	uint8_t count = 0;
		 
	state_t state = INIT;
	//uint8_t heartRateData[HEART_RATE_DATA_LENGTH];
	uint8_t oxygenSaturationData[PIPE_OXYGEN_SATURATION_O2_SET_MAX_SIZE] = {0,1,2,3,4,5,6};
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
	init();
	
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
						startTimer1();
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
						if (isTimer1FlagSet()){
							clearTimer1Flag();
						
							if(!(ADCSRA & (1<<ADIF)))
								startADC(ADC_PIN);
						
						}//if
					
						if (isADCfinished()){
						
							clearADCinterrupt();
							switch (led_state){
								case I:
									oxygenSaturationData[4] = ADCL;
									oxygenSaturationData[3] = ADCH;
									LEDS_OFF;
									led_state = O;
									break;
								case O:
									oxygenSaturationData[6] = ADCL;
									oxygenSaturationData[5] = ADCH;
									RED_ON;
									led_state = R;
									break;
								case R:
									oxygenSaturationData[2] = ADCL;
									oxygenSaturationData[1] = ADCH;
									IR_ON;
									led_state = I;
									state = SEND;
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

void init(void){
	
	//MCUCR = (1<<PUD);	//disable all pull ups
	
	//d6,d7 - LEDs
	DDRD = 0xC0;			//set PD7:6 to outputs for LEDs
	//PD2 as an input for switch
	RED_ON;					//Set PD7 high and PD6 low
	
	//Switch
	PORTD |= (1<<PD2);		//set PD2 to high(pullup resistor) for switch
	//EIMSK |= (1<<INT0);		//turn on interrupt 0 (PD2)
	//PCICR |= (1<<PCIE2);	//Enable PCINT2
	//PCMSK2 |= (1<<PCINT18); //Trigger on change of PCINT18 (PD2)
	//sei();					//enable interrupts
	
	//DDRB = 0X20;		//all inputs except B5 output
	//PORTB |= (1<<PB0);	//pull up resistor on (PB0)
	
	//DDRD  = 0b11111011;   // set PD2 to input
	
	//ADC
	
	//ADC clock must be between 50-200kHz to get maximum resolution
	//8MHz/64 (div64)	= 125,000
	
	//pin, prescalar
	initADC(ADC_PIN, ADC_DIV64);
	
	//timer
	// 8MHz/90Hz			= 88,888.8888 cycles to count
	// 88,888.888 / 8 (div8)	= 11,111 = 0x2B67
	
	//prescalar, mode, reload
	
	//initTimer1(TIMER_DIV8, 4, 0x2B67);
	initTimer1(TIMER_DIV64, 4, 0xFFFF);
	
	//opamp
	//opamp is Powered Down when PD is low, initialize HIGH?
	//250nS delay when powering up, 50nS delay when powering down (1/8MHz = 125nS)
	WakeOpAmp();
	
	
}


