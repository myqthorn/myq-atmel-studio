/*
 * mcp9801_test.cpp
 *
 * Created: 11/8/2013 5:39:18 AM
 *  Author: myQ_2
 */ 


#include <avr/io.h>
#include <stdio.h>
#include "MCP980x.h"
/////////////////////////////////////////////////////////////
//this block is necessary to include C files in a C++ project
/////////////////////////////////////////////////////////////
#ifdef __cplusplus
extern "C" {
#endif
	
	#include "23008LCD.h"
#include <stdlib.h>
#include <string.h>

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

void init(void){
	lcdInit();
	lcdClearDisplay();
	
}

// und keiner eier.  Heltal(read wholeNum) means integer.  
char *ftoa(char *a, double f, int precision)
{
	long p[] = {0,10,100,1000,10000,100000,1000000,10000000,100000000};
	
	char *ret = a;
	long wholeNum = (long)f;
	itoa(wholeNum, a, 10);
	while (*a != '\0') a++;
	*a++ = '.';
	long decimal = abs((long)((f - wholeNum) * p[precision]));
	itoa(decimal, a, 10);
	return ret;
}

int main(void)
{
	uint8_t temperature[2];
	float tempdouble;
	MCP980x temp1(0b000);
	temp1.setResolution(MCP980X_RES_12BIT);
	lcdInit();
	
    while(1)
    {
        temp1.getTemperature(temperature);
		tempdouble = temperature[0]+((temperature[1]>>4)*0.0625);
		
		//sprintf(line2, " %3d F", tempdouble *1.8 +32);
		
		ftoa(line1, tempdouble, 3);
		
		ftoa(line2, tempdouble * 1.8 + 32, 3);
		
		displayLCD(); 
		
		
    }
}


