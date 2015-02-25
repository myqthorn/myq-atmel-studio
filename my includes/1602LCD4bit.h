/*
 * File:   1602LCD4bit.h
 * Author: Mike Litster
 *
 * Revisions:   10 Dec 2012     created
 *              6 Feb 2012      clean up HardwareProfile dependency
 *				07 Oct 2013		modified for Atmega168
 *				01 Jan 2014		modified for 4-bit communication
 
 * Description: 
 *              with a HD44780 LCD.
 *              Currently supports 4-bit interface only.
 *
 * Pin assignments:
 *              RS:     GPIO0
*              EN:     GPIO1
*              DB0-3:  NC
*              DB4-7:  GPIO4-7
*              LED1:   GPIO2
*              LED2:   GPIO3
*
*/

#ifndef _1602LCD4BIT_H
#define _1602LCD4BIT_H

#include "HardwareProfile.h"    //this defines LCDDB




// LCD command definitions
    //the first line in each group is the category.
    //The following lines are the settings.
    //All settings in the category must be defined together.

//The clear screen group only has one setting.
#define LCD_CLR     0b00000001	// Clear display and home cursor

//The return home group only has one setting.  The LSB is x (don't care).
#define LCD_HOME    0b00000010  // Return home and reset shifting w/o changing DDRAM

//Entry Mode Set - These operations are performed during read and write
#define LCD_ENTRY_MODE_SET  0b00000100
#define LCD_INCREMENT       0b00000110  //Increments DDRAM address - writes L->R
#define LCD_DECREMENT       0b00000100  //Decrements DDRAM address - writes R->L
#define LCD_DISPLAY_SHIFT   0b00000101  //Display shifts upon entry
#define LCD_CURSOR_SHIFT    0b00000100  //Cursor shifts upon entry

//Display controls
#define LCD_DISPLAY         0b00001000
#define LCD_DISPLAY_ON      0b00001100  //Turn display on
#define LCD_DISPLAY_OFF     0b00001000  //Turn display off
#define LCD_CURSOR_ON       0b00001110  //Turn cursor on
#define LCD_CURSOR_OFF      0b00001100  //Turn cursor off
#define LCD_BLINK_ON        0b00001101  //Cursor blink on
#define LCD_BLINK_OFF       0b00001100  //Cursor blink off

//Shift the cursor or display left or right
#define LCD_SHIFT           0b00010000
#define LCD_SHIFT_DISPLAY   0b00011000  //shift the display
#define LCD_SHIFT_CURSOR    0b00010000  //shift the cursor
#define LCD_SHIFT_RIGHT     0b00010100  //shift to the right
#define LCD_SHIFT_LEFT      0b00010000  //shift to the left

//Function set
#define LCD_FUNCTION_SET    0b00100000
#define LCD_8BIT            0b00110000  //8 bit interface using DB7-0
#define LCD_4BIT            0b00100000  //4 bit interface using DB7-4
#define LCD_2LINES          0b00101000  //2 horizontal line display
#define LCD_1LINE           0b00100000  //1 horizontal line display
#define LCD_5X10            0b00100100  //uses font which is 5x10 pixels
#define LCD_5X8             0b00100000  //uses font which is 5x8 pixels

//Set CGRAM address.  This should be ORed with the 6 bit address.
//Character Generator RAM.
#define LCD_SET_CGRAM       0b01000000

//Set DDRAM address.  This should be ORed with the 7 bit address.
//Display Data RAM.  This is the address of the cursor.
#define LCD_SET_DDRAM       0b10000000


#define CMD_REG 		0	// Command register selection
#define DATA_REG 		1		// Data register selection

// custom characters
#define CHECKERBOARD    0x00
#define MY              0x01
#define HEART           0x02
#define UPARROW         0x03
#define DOWNARROW       0x04

// preset characters
#define OMEGA           0xF4
#define RIGHTARROW      0x7E
#define LEFTARROW       0x7F
#define DEGREES         0xDF
#define SIGMA           0xF6
#define THETA           0xF2
#define PI              0xF7

// Protypes...

// Write a byte to the LCD in 4 bit mode
void lcdWrite(unsigned char, unsigned char);

// Clear and home the LCD
void lcdClearDisplay(void);

// Write a string of characters to the LCD
void lcdPuts(const char * string);

// Go to the specified position
void lcdGoto(int x, int y);

// Intialize the LCD - call before anything else
void lcdInit(void);

// Output a single character
void lcdPutch(char);

// Set the display state (on or off)
void lcdDisplayState(int state);

//custom characters
void makeCheckerboard(unsigned char address);
void makeMY(unsigned char address);
void makeHeart(unsigned char address);
void makeUpArrow(unsigned char address);
void makeDownArrow(unsigned char address);

void AssignCustomCharacters(void);

void lcdTest(void);
#endif
