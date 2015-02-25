/*
 * nunchuk_servo.c
 *
 * Created: 10/18/2013 8:50:24 AM
 *  Author: myQ
 */ 

#include "HardwareProfile.h"
#include <avr/io.h>

#include "nunchuk.h"
#include "myqI2C.h"
#include <stdio.h>
#include <util/delay.h>
#include <stdlib.h>


//global variables
unsigned char  nunchuk[20];                  // Nunchuk data array
unsigned int  Z, C;							// Buttons on nunchuk
char line1[40], line2[40];         // LCD line arrays

void ConfigurePorts(void){
	
	DDRB = 0x20;		//all inputs except B5 output
	
	//configure PORTD for output
	//PD3, 5 & 6 will be used for PWM
	DDRD = 0xFF;
	
}

void InitPWM(void){
	
	
	//PD6 = OC0A
	//PD5 = OC0B
	
	
	//TCCRxA - Timer/counter control register A (x = timer)
	//	7:6 COMxA1, COMxA0:
	//	5:4 COMxB1, COMxB0:
	//		00	Normal port operation, OC0A disconnected
	//		01	Toggle OC0A on compare match (when WGM02 = 1)
	//		10	Clear OC0A/B on compare match, set OC0A/B at BOTTOM (non-inverting mode)
	//		11	Set OC0A/B on compare match, clear OC0A/B at BOTTOM (inverting mode)
	//	3:2	READ ONLY
	//	1:0	WGMx1, WGMx0
	//		These bits combine with WGM02 from TCCRxB
	//			MODE OF OPERATION		TOP		UPDATE of OCRx at		TOV flag set on
	//		000	Normal					0xFF		Immediate				MAX
	//		001	PWM, Phase correct		0xFF		TOP						BOTTOM
	//		010	CTC						OCRA		Immediate				MAX
	//		011 Fast PWM				0xFF		BOTTOM					MAX
	//		100	Reserved	
	//		101	PWM, Phase correct		OCRA		TOP						BOTTOM
	//		110	Reserved
	//		111	Fast PWM				OCRA		BOTTOM					TOP
		
	//	Set Timer0 A&B to non-inverting, Fast PWM
	//	pins are set at BOTTOM, cleared at OCR0A/B  --> OCR0A/B are the on times, 0xFF is TOP/period
	TCCR0A = (1<<COM0A1) | (1<<COM0B1) | (1<<WGM01) | (1<<WGM00);//0b1010 0011    0xA3
	
	//TCCRxB - Timer/counter control register B (x = timer)
	//	7:6	FOCxA, FOCxB
	//	5:4	Reserved
	//	3	WGM02 (See Above)
	//	2:0	CS02:00 - Clock Select
	//		000	No clock source (timer/counter stopped)
	//		001	clk(IO)
	//		010 clk(IO)/8
	//		011 clk(IO)/64
	//		100	clk(IO)/256
	//		101 clk(IO)/1024
	//		110	External clock source on T0 pin.  Clock on falling edge.
	//		111 External clock source on T0 pin.  Clock on rising edge.
	
	//	Set Timer0 to IO clock, prescaler = 64
	TCCR0B = (0<<CS02)|(1<<CS01)|(1<<CS00);
	
	//in Fast PWM mode, the PWM frequency can be calculated:
	//FREQ_OCnx = FREQ_CLK_IO / (N * 256)  -  n = prescale factor (1, 8, 64, 256 or 1024)
	
	//if F_CPU = 8000000, 
	//FREQ_PWM = 8000000/ (N * 256)
	//Prescaler = 1:	FREQ_PWM = 31250 Hz,	32uS per period,	125nS/division 
	//Prescaler = 8:	FREQ_PWM = 3096.25Hz,	256uS per period,	1uS/division
	//Prescaler = 256:	FREQ_PWM = 122.07 Hz,	8.192mS per period,	32uS/div
	//Prescaler = 1024:	FREQ_PWM = 30.52 Hz,	32mS per period,	125uS/division
	
	
}


void init(void){
	InitNunchuk();	
	ConfigurePorts();
	InitPWM();	
}

void writeRGB(unsigned char red, unsigned char green, unsigned char blue){
	i2c_start(RGB_I2C_ADDR<<1); //7-bit address plus r/w bit 0=write
	
	i2c_write(red);
	i2c_write(green);
	i2c_write(blue);
	i2c_stop();
}

int main(void)
{
	//unsigned char oldX, oldY;
	init();
	
	while(1)
	{
		//oldX = nunchuk[0];
		//oldY = nunchuk[1];
		
		ReadNunchuk(nunchuk);                        // Read data from nunchuk
		Z = (nunchuk[5] & 0b1) ^ 0b1;              // Extract Z button bit
		C = (nunchuk[5]>>1 & 0b1) ^ 0b1;           // Extract C button bit
		if (C) PORTD |= 0x10;//turn PD4 on       (1<<PD4)
		else PORTD &= 0xEF;//turn PD4 off
		
		//displayNunchuk();
		
		//OCR0n represents the on-time of the PWM
		//it is 8-bit resolution - from 0x00 to 0xFF
		//8.192ms for one period, 32uS/ per division
		//a servo is complete left? at 1ms on time, complete right? at 2ms
		//nunchuk joystick ranges
		//	0x18(left) to 0xDF(right) - C7 range
		//	0x1F(back) to 0xDF(forward)
		// 1ms = 32uS*31.25 - let's say 31 is close enough.
		
		// wait 31*32uS + zero out nunchuk - scale nunchuk by 1/0xC7 and 31 (0.15)
		//if (abs(oldX-nunchuk[0])>0x02)
			OCR0A = 10 + (0xC7-(nunchuk[0]-0x18))*0.15; //X-axis (joystick)
		//if (abs(oldY-nunchuk[1])>0x02)	
			OCR0B = 10 + (0xC7-(nunchuk[1]-0x18))*0.15; //X-axis (joystick)
		//OCR0B = 31 + (nunchuk[1]-0x18)*0.15; //Y-axis (joystick)
		writeRGB(255-nunchuk[2],255-nunchuk[3],255-nunchuk[4]);
	}
}