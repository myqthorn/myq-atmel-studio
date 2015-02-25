/*
 * File:   23008LCD.c
 * Author: Thorn
 *
 * Created on December 10, 2011, 4:33 AM
 */
#define _23008LCD_C
#include	"23008LCD.h"    //this includes "HardwareProfile.h" which
                                //defines LCDDB (or not)

#include	<stdio.h>
#include	<string.h>
#include	<stdlib.h>
#include <util/delay.h>

#if defined (__C30__)
    #include <p24Fxxxx.h>
#endif

#if defined (__C18__)
    #include <p18cxxx.h>
#endif

#define MS *16000
	
//TODO: fix this
#define wait(a) for(delay =a; delay>0;delay--);
//#define waitms(a) for()
	
//clock is the EN pin on the LCD or gp1 on the 23008
#define clockLow()  toLCD &= 0b11111101; Write23008(GPIO, toLCD)
#define clockHigh() toLCD |= 0b00000010; Write23008(GPIO, toLCD)

//RS is gp0 on the 23008
#define RSHigh() toLCD |=0x01; Write23008(GPIO, toLCD)
#define RSLow() toLCD &=0xFE; Write23008(GPIO, toLCD)
#define dataRegister() RSHigh()
#define commandRegister() RSLow()
   

unsigned char toLCD = 0x00;
unsigned int delay;
unsigned char lcdX;
unsigned char lcdY;

    

//writes nybble to LCD preserving RS's current value
void lcdWriteNybble(unsigned char nybble){
    toLCD = (nybble<<4 & 0xF0) | (toLCD & 0x0F);
    Write23008(GPIO, toLCD);
}

// write a byte to the LCD in 4 bit mode
void lcdWrite(unsigned char address, unsigned char c)
{
    clockLow();
    if (address == CMD_REG){
        commandRegister();
    }
    else dataRegister();

    _delay_us(1);      //1 cycle = 62.5ns (nop)

    clockHigh();

    // Write the 4 bits non-destructively to the port
    lcdWriteNybble((c >> 4) & 0x0F);
    wait(5);    //~250ns

    clockLow();

    wait(4);    //~200ns
    clockHigh();
    // Write the 4 bits non-destructively to the port
    lcdWriteNybble(c & 0x0F);

    wait(5);    //~250ns
}

// write one character to the LCD
void lcdPutch(char c)
{
        // This routine needs updating so it monitors the cursor posiiton


    lcdWrite(DATA_REG, c);
}

void lcdClearDisplay(void)
{
    int counter;
        lcdWrite(CMD_REG, LCD_CLR);

        // Reset our internal cursor position
        lcdX = 0;
        lcdY = 0;

        // Delay
        for (counter = 0; counter < 200; counter++) wait(100);
}

// Initialize the LCD
void lcdInit(void){
        
	//int counter;
	Init23008();
		
	Write23008(IODIR, 0x00); //Set all ports to output
	Write23008(GPIO, 0x00); //Turn off all ports

    // Power up delay
    _delay_ms(15);

    clockLow();
    commandRegister();
    //LCD_RS = CMD_REG; // Set RS to command register
    _delay_us(1);	//Nop();
    clockHigh();

    //Set up the 4-bit interface.
    //The LCD is defaulted into 8-bit mode.
    //lcdWriteNybble() is set up for sending the less significant nybble of
    //the byte (bits3-0) in 4-bit mode (DB7-4).  Shifting it 4 bits to the
    //right ensures that the more significant nybble will be sent to DB7-4.
    lcdWriteNybble(LCD_4BIT>>4);
    wait(5);

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
                case 2: offset = 0x14;
                                break;
                case 3: offset = 0x54;
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
void LED1on(void){
    toLCD |=0x04;
    Write23008(GPIO, toLCD);
}//LED1on

void LED1off(void){
    toLCD &=0xFB;
    Write23008(GPIO, toLCD);
}//LED1off

void LED2on(void){
    toLCD |=0x08;
    Write23008(GPIO, toLCD);
}//LED2on

void LED2off(void){
    toLCD &=0xF7;
    Write23008(GPIO, toLCD);
}//LED2off

void toggleLED1(void){
    if (toLCD & 0x04)  //test GP2
        LED1off();
    else LED1on();
}

void toggleLED2(void){
    if (toLCD & 0x08)    //test GP3
        LED2off();
    else LED2on();
}

