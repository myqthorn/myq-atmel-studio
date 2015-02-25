/*
 * nunchuk_practice.c
 *
 * Created: 10/7/2013 5:53:00 PM
 *  Author: myQ
 */ 


#include <avr/io.h>

#include "nunchuk.h"
#include "23008LCD.h"
#include <stdio.h>
#include <util/delay.h>


//global variables
unsigned char  nunchuk[20];                  // Nunchuk data array
unsigned int  Z, C;							// Buttons on nunchuk
char line1[40], line2[40];         // LCD line arrays

void displayLCD(void){
	lcdGoto(0,0);
	lcdPuts(line1);		// Display line 1
	lcdGoto(0,1);		// Skip to next line
	lcdPuts(line2);
}

void init(void){
	InitNunchuk();
	lcdInit();
	lcdClearDisplay();
	sprintf(line1, "myQ Atmega168 Dev Board rev0");
	sprintf(line2, "Nunchuk display basic");
	displayLCD();
	
	//scroll display to the left 30 times
	for(int i=0;i<30;i++){
		lcdWrite(CMD_REG, LCD_SHIFT | LCD_SHIFT_DISPLAY | LCD_SHIFT_LEFT);
		_delay_ms(100);
	}
	lcdClearDisplay();
}



void displayNunchuk(void){

	//ReadNunchuk(nunchuk);                        // Read data from nunchuk
	//Z = (nunchuk[5] & 0b1) ^ 0b1;              // Extract Z button bit
	//C = (nunchuk[5]>>1 & 0b1) ^ 0b1;           // Extract C button bit
	
	sprintf(line1, "SX%02X Y%02X C%d Z%d",      // Format joystick and
	nunchuk[0], nunchuk[1], C, Z);			//  button data
	sprintf(line2,"X%02X Y%02X Z%02X",       // Format accelerometer
	nunchuk[2], nunchuk[3], nunchuk[4]);   //  data
	displayLCD();	
}

int main(void)
{
	
	init();
	
    while(1)
    {
       ReadNunchuk(nunchuk);                        // Read data from nunchuk
       Z = (nunchuk[5] & 0b1) ^ 0b1;              // Extract Z button bit
       C = (nunchuk[5]>>1 & 0b1) ^ 0b1;           // Extract C button bit
	   displayNunchuk();
	   
    }
}