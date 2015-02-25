/*
 * DCIMultimeter.c
 *
 * Created: 1/1/2014 9:01:48 AM
 *  Author: Mike Litster
 */ 

#pragma region Includes and Defines

#include "HardwareProfile.h"
#include <avr/io.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>		//for lcd msg
#include <util/delay.h>
#include <math.h>
#include <avr/interrupt.h>

#include "1602LCD4bit.h"
#include "si5351.h"

#define VOLTAGE_SENSE	0
#define CURRENT_SENSE	1
#define FREQ_SET		6

#define Vref			3.3

#define FREQ_MODE1		1	//220k to 2MHz
#define FREQ_MODE2		2	//750K to 2Mhz


//////////////////////////////////////////////////////////////////////////
//defines for the Si5351
//////////////////////////////////////////////////////////////////////////

	//we set the Fvco (PLL) to 600MHz
	//this gets divided by R0 by a factor of 2^5 = 32
	//which gives us a high end frequency of 18.75MHz
	//M0 needs to be between 6 and 1800
	//giving us a range from 3.125MHz down to 10.416kHz
	//we will only use from M0=9: 2.083MHz
	//to M0 = 104: 180.288kHz
#define R0	(0b101)
#define Fvco (600000000)
#define FREQUENCY (Fvco/ (M0*R0))

#pragma endregion Includes and Defines

#pragma region global variables
	//M0 is bound by 9 and 104, giving it a range of 95
	//it's value is determined by the ADC which is 10-bit
	//hence the 2^10 denominator
	//M0 will need to be 9 + ADC*deltaM0
float deltaM0 = 95.0/1024;
double M0;
uint32_t p1,p2;
//p3 is the denominator for the multisynth value
uint32_t p3 = 1024;



char		line1[40], line2[40], line3[40], line4[40];		// LCD line arrays
uint8_t		isCountingFreq = 0;		//boolean representing if we are counting for frequency or not
uint8_t		freqdone = 0;			//interrupt sets this to 1
uint16_t	freqCount = 0;			//represents the count for the frequency sense
double		frequency = 0.0;

#pragma endregion global variables

#pragma region Function Prototypes
//////////////////////////////////////////////////////////////////////////
///Function Prototypes
//////////////////////////////////////////////////////////////////////////

void led1on(); 
void led2on();
void ledoff();

char *ftoa(char *a, double f, int precision);
void displayLCD(void);
void splashScreen(void);
void initADC();
void initTimer();
uint8_t setFrequency(uint16_t f);
void customSetup(void);
uint8_t initXTAL();
uint8_t init();
void readADC(uint8_t channel);
void incrementScalar(void);
void decrementScalar(void);
void prepareDisplay(float voltage, float current, uint16_t frequencySetting, double f);
void readFrequency(void);
float calculateFrequency(uint16_t freq);

#pragma endregion Function Prototypes

int main(void)
{
	
	float		current=0.0;
	float		voltage=0.0;
	uint16_t	temp = 0,
				frequencySetting = 0,
				oldFreq = 0;
	uint8_t		displaycount = 16;
	uint8_t		mode = FREQ_MODE2;
	//startup delay for LCD, etc.
	_delay_ms(100);
	
	init();
	
	splashScreen();
	
    while(1)
    {		
		
		//Voltage Sense--------------------------------------------
		//read voltage(10-bit) and prepare for LCD
		readADC(VOLTAGE_SENSE);		
		voltage = (float)ADC * Vref / 1024L;
		
		
		//voltage *= 16.19;		
		voltage *= 24.86;
			
		//current sense--------------------------------------------
		//read current(10-bit) 
		readADC(CURRENT_SENSE);
		current = (float)ADC * Vref / 1024L;
		
		//convert from Voltage to Current through 0.1 ohm resistor
		current *= 225.0;	
				
		//Frequency pot--------------------------------------------
		//read frequency setting from pot(10-bit) and send value to Si5351
		
		readADC(FREQ_SET);
		frequencySetting = ADC;
		
		//only change Frequency of Si5351 when the pot changes
		if (abs(frequencySetting - oldFreq)>1){
			
			
			//Set up different scales for resolution
			if (frequencySetting<500)
				temp = ((long)((double)frequencySetting*100.0/500.0));	
			else if (frequencySetting<800)
				temp = (100 + (long)( (double)(frequencySetting-500.0)*500.0/300.0));
			else 
				temp = (600 + (long)( (double)(frequencySetting-800.0)*400.0/200.0));
				
			if (mode == FREQ_MODE1){}
			//from about 750kHz to 2 MHz
			else if (mode == FREQ_MODE2){
				//dividing by	6:		792k to 2M
				//dividing by	5:		704k
				//				5.5:	748k
				temp /= 5.5;
			}else temp = frequencySetting;
			//frequencySetting = temp;
			setFrequency(temp);	
			oldFreq = frequencySetting;
		}
		
		
		//----------------------------------------------------------				
		
		
		
			
		//////////////////////////////////////////////////////////////////////////				
		//Frequency Sense
		//////////////////////////////////////////////////////////////////////////
		
		readFrequency();
		
		//////////////////////////////////////////////////////////////////////////
		frequency = calculateFrequency(freqCount);
		prepareDisplay(voltage, current, M0, frequency );
		
		//sprintf(line3, "FreqRead: %04d %03X ", freqCount, frequencySetting );
		
		displayLCD();
		
	}
	//should never get to this point
	return -1;
}

#pragma region initialize functions

///@brief
///@returns 1 if failure to address i2c device
uint8_t init(){
	
	
	//LCD (outputs) on PD7:4
	//make pins outputs for bicolor LED (on pins PD1:0)
	//Frequency sense (input) on PD2
	PORTD = 0x00;
	DDRD = 0xF3;//F0, F1, F2, F3, F4 WORKS! F8 fails
	
	
	//turn LED on
	led1on();
	
	//LCD init///////////////////////////////////////////////
	lcdInit();
	lcdClearDisplay();
	
	initADC();
	initTimer();
	
	return (initXTAL());
}

//@brief initialize AD converter on Atmega168 for
void initADC(){
	
	//Power reduction ADC bit must be zero to enable ADC
	PRR &= ~(1<<PRADC);
	
	//ENABLE, ADC PRESCALER = 64
	//ADC clock needs to be between 50kHz and 200kHz
	//8MHz/64 = 125kHz
	//ADCSRA = (1<<ADEN | 1<<ADPS2 | 1<<ADPS1 | 0<<ADPS0);
	ADCSRA = 0x86;
	
	//we are using the following pins:
	//Voltage sense:	PC0 / ADC0
	//Current sense:	PC1 / ADC1
	//Frequency set:	ADC6
	
	//Disable the digital input for the ADC channels we will be using
	//DIDR0 |= (1<<ADC0D | 1<<ADC1D);
}

///@brief initialize timer on Atmega168
void initTimer(){
	//INT0 is on PD2 pin, enable as input, turn on pullup(0=input, 1=output)
	//DDRD &= ~(1<<PD2);		//make PD2 an input
	
	//Set up Interrupts--------------------------------------------------------------------------------------
	//External Interrupt Mask Register
	EIMSK |= (1<<INT0);	//turn on interrupt 0 (PD2)
	
	////External Interrupt Control Register A
	//EICRA = (0b10<<ISC00);		//Interrupt on falling edge of INT0
	EICRA = (0b11<<ISC00);		//Interrupt on rising edge of INT0
	
	//Setup Timer1 for frequency counter--------------------------------------------------------
	//disconnect OC1A, OC1B, count up, no prescalar, clock stopped
	TCCR1A = 0x00;
	TCCR1B = 0x00;
}

///Custom initialization written by my interpretation of the datasheet
void customSetup(void){
	//double a,b;
	
	//Input source select for PLLA, PLLB (0=XTAL, 1 = CLKIN)
	//SI5351_write(SI5351_REG_PLL_INPUT_SOURCE,(0<<SI5351_PLLA_SRC | 0<<SI5351_PLLB_SRC);
	
	//configure PLL Feedback Multisynth Dividers
	//Fvco needs to be between 600MHz and 900MHz
	SI5351_configureMultisynthNA(2560,0,1); //600MHz
	//SI5351_configureMultisynthNB(2560,0,1);	//600MHz
	
	//set FBA_INT and FBB_INT to improve jitter since the divider is an integer
	//CLK6 and CLK7 control registers
	
	//read frequency setting from pot(10-bit) and send value to Si5351
	readADC(FREQ_SET);
	setFrequency(ADC);
	
	
	////configure Multisynth Dividers
	////initially sets Fout to 200KHz
	////Frequency = Fvco/(M0 * R0)
	////200khz = 600MHz/(2^5 * 93.75)
	//a = 93;
	//b = floor(0.75*p3);
	
	//p1 = (uint32_t)(128 * a + floor(128*b/p3)-512);
	//p2 = (uint32_t)(128 * b + floor(128*b));
	
	//SI5351_configureMultisynth(0, p1, p2, p3, R0); //
	//SI5351_configureMultisynth(1, p1, p2, p3, R0);
	
	//set the internal capacitance for the crystal 6,8, or 10pF
	SI4341_setXTALcapacitance(SI5351_XTAL_10PF);
	
	
	
	
	//Additional setup code would go above here before we turn on the clocks
	
	//Select MS0 as source for CLK0
	//0x0F = MS0, 8mA
	SI5351_write(SI5351_REG_CLK0_CONTROL, 0x0F);
	
	//Select MS0 as source for CLK1, Invert the output
	//0001 1011 0x1B - MS0
	//0001 1111 0x1F - MS1
	SI5351_write(SI5351_REG_CLK1_CONTROL, 0x1F);
	
}

///@brief initialize Si5351
uint8_t initXTAL(){
	
	uint8_t result;
	SI5351_init();
	
	//disable outputs
	result = SI5351_write(SI5351_REG_OUTPUT_ENABLE_CONTROL, 0xFF);
	
	//powerdown all output drivers
	SI5351_write(SI5351_REG_CLK0_CONTROL, 0x80);
	SI5351_write(SI5351_REG_CLK1_CONTROL, 0x80);
	SI5351_write(SI5351_REG_CLK2_CONTROL, 0x80);
	SI5351_write(SI5351_REG_CLK3_CONTROL, 0x80);
	SI5351_write(SI5351_REG_CLK4_CONTROL, 0x80);
	SI5351_write(SI5351_REG_CLK5_CONTROL, 0x80);
	SI5351_write(SI5351_REG_CLK6_CONTROL, 0x80);
	SI5351_write(SI5351_REG_CLK7_CONTROL, 0x80);
	
	//Set interrupt masks (register 2)
	SI5351_write(SI5351_REG_INTERRUPT_STATUS_MASK, 0x00);
	
	//Write new configuration
	customSetup();
	
	
	//Apply PLLA & PLLB soft reset
	SI5351_write(SI5351_REG_PLL_RESET, 0xA0);
	
	//Enable outputs CLK0 & CLK1
	//1=disable
	SI5351_write(SI5351_REG_OUTPUT_ENABLE_CONTROL, 0x00);
	
	return result;
}


#pragma endregion initialize functions

#pragma region display functions

///@brief writes to LCD
void displayLCD(void){
	lcdGoto(0,0);
	lcdPuts(line1);		// Display line 1
	lcdGoto(0,1);		// Skip to next line
	lcdPuts(line2);
	lcdGoto(0,2);
	lcdPuts(line3);		// Display line 1
	lcdGoto(0,3);		// Skip to next line
	lcdPuts(line4);
}


void prepareDisplay(float voltage, float current, uint16_t frequencySetting, double f){
	char someString[40] = "";
	double temp =0.0;
	//voltage
	ftoa(someString, voltage, 2);
	sprintf(line1, "Voltage:  %s V ",someString);
	
	//current
	ftoa(someString, current, 2);
	sprintf(line2, "Current:   %s mA ",someString);
	
	
	//frequencyRead
	sprintf(line3, "Reading: %4ld kHz ", (long)f );
	
	//frequencySetting
	
			//sprintf(line3, " Freq:      %03X ", frequencySetting );
	//sprintf(line4, "Setting: %4ld kHz ", (long)FREQUENCY );
	
	//ftoa(someString, f, 0);
	//sprintf(line4, " %4ld", someString);
	
}

void splashScreen(void){
	//display Name and number and pause
	sprintf(line1, "Design Criteria Inc");
	sprintf(line2, "(801) 393-1414");
	displayLCD();
	_delay_ms(500);
	lcdClearDisplay();
}

///@brief converts a double to a string
char *ftoa(char *a, double f, int precision){
	long p[] = {0,10,100,1000,10000,100000,1000000,10000000,100000000};
	char *ret = a;
	long wholeNum = (long)f;
	
	if (wholeNum<10) *a++ = ' ';
	itoa(wholeNum, a, 10);
	while (*a != '\0') a++;
	*a++ = '.';
	long decimal = abs((long)((f - wholeNum) * p[precision]));
	itoa(decimal, a, 10);
	return ret;
}

#pragma endregion display functions

#pragma region LED functions

void led1on(void){
	//set one high and the other low
	PORTD |= (1<<PD1); 
	PORTD &= ~(1<<PD0);
}

void led2on(void){
	//set one high and the other low
	PORTD |= (1<<PD0);
	PORTD &= ~(1<<PD1);
}

void ledoff(void){
	//set both pins low
	PORTD &= ~(1<<PD1 | 1<<PD0);
}

#pragma endregion LED functions

#pragma region Frequency and ADC functions

///@description:	sets Si5351 frequency to the
///@returns 0 if i2c device is accessible, 1 if not
///@param f is a 10-bit number read from the ADC to set the frequency
uint8_t setFrequency(uint16_t f){
	M0 = 9 + (f*deltaM0);
	double a = floor(M0);
	double b = floor((M0-a)*p3);
	uint8_t status = 1;
	
	p1 = (uint32_t)(128 * a + floor(128*b/p3)-512);
	p2 = (uint32_t)(128 * b + floor(128*b));
	
	
	//Disable outputs 1=disable
	SI5351_write(SI5351_REG_OUTPUT_ENABLE_CONTROL, 0xFF);
	
	
	status |= (SI5351_configureMultisynth(1, p1, p2, p3, R0));
	status |= (SI5351_configureMultisynth(0, p1, p2, p3, R0));
	
	//Apply PLLA & PLLB soft reset
	SI5351_write(SI5351_REG_PLL_RESET, 0xA0);
	
	//Enable outputs 1=disable 0=enable
	SI5351_write(SI5351_REG_OUTPUT_ENABLE_CONTROL, 0x00);
	
	return (status);
	
	//////////////////////////////////////////////////////////////////////////
	///explanation of frequency:
	//Fcvo is set in customSetup to be 600MHz
	//deltaM0 = 95/1024 = .0927734
	//R0 = 0b101 = 5
	//Output Frequency = Fvco / (M0*R0) = 600MHz / (M0 * 5)
	//
	
}

void readFrequency(void){
	
	//initialize
	TCCR1B = 0x00;		//stop counter
	TCNT1  = 0x0000;	//reset counter
	EIFR = (1<<INTF0);		//Reset flag
	
	//wait for edge detect
	//EIFR is the External Interrupt Flag Register
	//INTF0 is the interrupt flag for INT0
	while ((EIFR & 0x01) != (0x01));
	
	//start timer
	TCCR1B = 0x01;		//start counter
	EIFR = (1<<INTF0);		//Reset flag
	
	//wait for edge detect
	for(int i=0;i<200;i++){
		while ((EIFR & 0x01) != (0x01));
		EIFR = (1<<INTF0);
	}
	
	//stop timer and record value into freqCount
	TCCR1B = 0x00;		//stop counter
	freqCount = TCNT1;
	
}


///@brief calculates frequency from timer readings
float calculateFrequency(uint16_t freq){
	//calculate frequency
	//  1/8MHz = 125E-9 seconds/ cycle
	// 125E-9/2 = 62.5E-9 s
	
	// count per measured cycle = freqCount/100 because we are timing 100 edge detects
	// there is a .978 error factor from the measured to the actual
	// we are also dividing by 1000 and appending a k to the output
	
	// frequency = 1/period
	// frequency = 1/(62.5E-9 * (freqCount/100) * 0.978 * 1000)
	
	// 100/(62.5E-9 * 0.978 * 1000) = 1635991.82
	return (1635992/(double)freq);
	
	
}

///@description	Performs an ADC conversion on specified ADC channel
///@param		channel		The channel to perform the conversion on.
///@returns		Nothing, but ADCH and ADCL should contain the 10-bit value of the conversion upon exit.
void readADC(uint8_t channel){
	
	//set ADC channel
	ADMUX = (0x40) | channel;
	
	//start conversion
	ADCSRA |= (1<<ADSC);
	
	//wait for conversion to finish (interrupt flag is set)
	while( !(ADCSRA & (1<<ADIF)) );
	//while (ADCSRA & (1<<ADSC));
	
	//clear interrupt by setting ADIF to 1
	ADCSRA |= (1<<ADIF);
}

#pragma endregion Frequency and ADC functions

#pragma region Interrupt

//ISR(INT0_vect){
	//TCCR1B = 0x00;		//stop counter
	//ledoff();
	////ch
	//if(isCountingFreq==0){
		//
		//isCountingFreq = 1;
		//TCNT1  = 0x0000;	//reset counter
		//TCCR1B = 0x01;		//start counter
		//
		////EIFR = 0x01;		//Reset flag - this should be done automatically
	//}else{
		////read count
		//freqCount = TCNT1;		
		//
		//isCountingFreq = 0;
		//freqdone = 1;
		//
		////EIFR = 0x01;		//Reset flag - this should be done automatically
		////led2on();
	//}//if else
//}//ISR
//
//
#pragma endregion Interrupt
