/*
 * File:   1602LCD4bit.c
 * Author: Mike Litster
 *
 * Created on December 10, 2011, 4:33 AM
 *				see header file for revisions
 */
#define _1602LCD4BIT_C
#include	"1602LCD4bit.h"    //this includes "HardwareProfile.h" 
                               

#include	<stdio.h>
#include	<string.h>
#include	<stdlib.h>
#include <util/delay.h>
#include <avr/io.h>
//
//#if defined (__C30__)
    //#include <p24Fxxxx.h>
//#endif
//
//#if defined (__C18__)
    //#include <p18cxxx.h>
//#endif
//
//#define MS *16000
	
//TODO: fix this
//#define wait(a) for(delay =a; delay>0;delay--);
//#define waitms(a) for()
	
//clock is the EN pin on the LCD
#define clockLow()  PORTB &=  ~(1<<PB0)		//toLCD &= 0b11111101; Write23008(GPIO, toLCD)

#define clockHigh() PORTB |=  (1<<PB0)		//toLCD |= 0b00000010; Write23008(GPIO, toLCD)

//RS 
#define RSLow()		PORTB &=  ~(1<<PB1)		//toLCD |=0x01; Write23008(GPIO, toLCD)
#define RSHigh()	PORTB |= (1<<PB1)		//toLCD &=0xFE; Write23008(GPIO, toLCD)

#define dataRegister() RSHigh()
#define commandRegister() RSLow()
   
#define CLOCKDELAY 50
//unsigned char toLCD = 0x00;
//unsigned int delay;
unsigned char lcdX;
unsigned char lcdY;

    

//writes nybble to LCD preserving RS's current value
void lcdWriteNybble(unsigned char nybble){
	unsigned char temp;
    
	temp = (PIND & 0x0F);
    PORTD = (temp | (nybble<<4 & 0xF0));
	//PORTD = nybble<<4 & 0xF0;
}

// write a byte to the LCD in 4 bit mode
void lcdWrite(unsigned char address, unsigned char c)
{
    clockLow();
    if (address == CMD_REG){
        commandRegister();
    }
    else dataRegister();

    _delay_us(50);      //1 cycle = 62.5ns (nop)

    

    // Write the 4 bits non-destructively to the port
    lcdWriteNybble((c >> 4) & 0x0F);
    
	clockHigh();
	_delay_us(CLOCKDELAY);   
    clockLow();

	//_delay_us(CLOCKDELAY);
    
    // Write the 4 bits non-destructively to the port
    lcdWriteNybble(c & 0x0F);
	
	clockHigh();
	_delay_us(CLOCKDELAY);    //~250ns
	clockLow();
}

// write one character to the LCD
void lcdPutch(char c)
{
        // This routine needs updating so it monitors the cursor position


    lcdWrite(DATA_REG, c);
}

void lcdClearDisplay(void)
{
   
	//Clear display and home cursor (0,0)
    lcdWrite(CMD_REG, LCD_CLR);

    // Reset our internal cursor position
    lcdX = 0;
    lcdY = 0;

    // Delay
    //for (counter = 0; counter < 200; counter++) wait(100);
	_delay_ms(CLOCKDELAY);
}

// Initialize the LCD
void lcdInit(void){   	
	
	//initialize PORTD for 4-bit interface
	//DDRD = DDRD | 0xF0;		//Set PD7:4 to output
	PORTD &= 0x0F;		//Clear PD7:4 to low (preserving PD3:0)

	//initialize PB1 for RS, PB0 for E
	DDRB |= 0x03;		//Set PB1:0 to output
	PORTB &= 0xFC;		//Clear pins PB1:0 (preserving PB7:2)
	
    // Power up delay
    _delay_ms(15);


    clockLow();
    commandRegister();
    //LCD_RS = CMD_REG; // Set RS to command register
    
    
	//_delay_us(1);	//Nop();
    //Set up the 4-bit interface.
    //The LCD is defaulted into 8-bit mode.
    //lcdWriteNybble() is set up for sending the less significant nybble of
    //the byte (bits3-0) in 4-bit mode (DB7-4).  Shifting it 4 bits to the
    //right ensures that the more significant nybble will be sent to DB7-4.
    lcdWriteNybble(LCD_4BIT>>4);
    
	clockHigh();
	_delay_us(CLOCKDELAY);
	clockLow();

    //set it to 4 bit mode, 2 lines, 5x8 pixels
    lcdWrite(CMD_REG, LCD_4BIT | LCD_2LINES |LCD_5X8);

    //display on, cursor off, blinking off
    lcdWrite(CMD_REG, LCD_DISPLAY_ON | LCD_CURSOR_OFF | LCD_BLINK_OFF);

    //Increments ddram address, shift cursor
    lcdWrite(CMD_REG, LCD_INCREMENT | LCD_CURSOR_SHIFT);

    // Reset our internal cursor position
    lcdX = 0;
    lcdY = 0;
}

void lcdDisplayState(int state)
{
        if (state == 1) lcdWrite(CMD_REG, LCD_DISPLAY_ON);
                else lcdWrite(CMD_REG, LCD_DISPLAY_OFF);
}

// Move the cursor to the specified X,Y
void lcdGoto(int x, int y)
{
        int offset=0;

        // Select the correct line of the display
        switch (y)
        {
                case 0: offset = 0x00;
                                break;
                case 1: offset = 0x40;
                                break;
                case 2: offset = 0x14;  //for a 16 digit display, this should be 0x10
                                break;
                case 3: offset = 0x54;	//0x50
                                break;
        }

        // Select the correct character of the line
        offset += x;

        // Send the command to the LCD
        lcdWrite(CMD_REG, LCD_SET_DDRAM | offset);

        // Reset our internal cursor position
        lcdX = x;
        lcdY = y;
}

// Output a string of characters to the display
void lcdPuts(const char *string)
{
        // Since the 4 line display is in the order line 1, line 3, line 2, line 4
        // we need to read the current cursor position and control how the text is
        // output, adjusting the cursor position as we go
        int loop;

        for (loop = 0; loop < strlen(string); loop++)
        {
                // Write the character to the LCD
                lcdWrite(DATA_REG, string[loop]);
                lcdX++;

                // Have we reached the end of the line?
                if (lcdX == 40)
                {
                        // Move to the start of the next line
                        lcdX = 0;
                        lcdY++;

                        // If we are off the bottom of the screen
                        // go back to the top line
                        if (lcdY == 2) lcdY = 0;

                        // Adjust the cursor position on the LCD
                        lcdGoto(lcdX, lcdY);
                }
        }
}
//------functions for custom characters-----------------------------------------
void makeCheckerboard(unsigned char address){
    if(address<0x10){
        lcdWrite(CMD_REG, LCD_SET_CGRAM |address<<3);
        lcdWrite(DATA_REG, 0b10101);
        lcdWrite(DATA_REG, 0b01010);
        lcdWrite(DATA_REG, 0b10101);
        lcdWrite(DATA_REG, 0b01010);
        lcdWrite(DATA_REG, 0b10101);
        lcdWrite(DATA_REG, 0b01010);
        lcdWrite(DATA_REG, 0b10101);
        lcdWrite(DATA_REG, 0b00000);
    }
}
void makeMY(unsigned char address){
    if(address<0x10){
        lcdWrite(CMD_REG, LCD_SET_CGRAM | address<<3);
        lcdWrite(DATA_REG, 0b10001);
        lcdWrite(DATA_REG, 0b11011);
        lcdWrite(DATA_REG, 0b10101);
        lcdWrite(DATA_REG, 0b10001);
        lcdWrite(DATA_REG, 0b11011);
        lcdWrite(DATA_REG, 0b10101);
        lcdWrite(DATA_REG, 0b10101);
        lcdWrite(DATA_REG, 0b10101);
    }
}

void makeHeart(unsigned char address){
    if(address<0x10){
        lcdWrite(CMD_REG, LCD_SET_CGRAM | address<<3);
        lcdWrite(DATA_REG, 0b00000);
        lcdWrite(DATA_REG, 0b01010);
        lcdWrite(DATA_REG, 0b11111);
        lcdWrite(DATA_REG, 0b11111);
        lcdWrite(DATA_REG, 0b11111);
        lcdWrite(DATA_REG, 0b01110);
        lcdWrite(DATA_REG, 0b00100);
        lcdWrite(DATA_REG, 0b00000);
    }
}

void makeUpArrow(unsigned char address){
    if(address<0x10){
        lcdWrite(CMD_REG, LCD_SET_CGRAM | address<<3);
        lcdWrite(DATA_REG, 0b00100);
        lcdWrite(DATA_REG, 0b01110);
        lcdWrite(DATA_REG, 0b10101);
        lcdWrite(DATA_REG, 0b00100);
        lcdWrite(DATA_REG, 0b00100);
        lcdWrite(DATA_REG, 0b00100);
        lcdWrite(DATA_REG, 0b00100);
        lcdWrite(DATA_REG, 0b00000);
    }
}

void makeDownArrow(unsigned char address){
    if(address<0x10){
        lcdWrite(CMD_REG, LCD_SET_CGRAM | address<<3);
        lcdWrite(DATA_REG, 0b00100);
        lcdWrite(DATA_REG, 0b00100);
        lcdWrite(DATA_REG, 0b00100);
        lcdWrite(DATA_REG, 0b00100);
        lcdWrite(DATA_REG, 0b10101);
        lcdWrite(DATA_REG, 0b01110);
        lcdWrite(DATA_REG, 0b00100);
        lcdWrite(DATA_REG, 0b00000);
    }
}

void AssignCustomCharacters(void){
        makeCheckerboard(0x00);

        makeMY(0x01);

//     makeHeart(0x02);
//
//     makeUpArrow(0x03);
//
//     makeDownArrow(0x04);
}
//------end functions for custom characters-------------------------------------


void lcdTest(){
	while(1)
	{
		lcdWriteNybble(0b1010);
		_delay_ms(500);
		lcdWriteNybble(0b0101);
		_delay_ms(500);
	}
}

/*
Write operation:
1. Set RS line high or low to designate which register you are accessing.
2. Set R/W line low to indicate a write
3. Write data to DBport
4. Set E line high to begin write cycle.  this is the clock
5. Pause
6. Set E line low to finish write cycle.

*/