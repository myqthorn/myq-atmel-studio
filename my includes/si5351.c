/*
 * File:   si5351.c
 * Author: Mike Litster
 *
 * 21 Feb 2014  created
 * 
 */


#include "HardwareProfile.h"

#ifdef __cplusplus
extern "C" {
#endif
	#include "si5351.h"
	#include "myqI2C.h"
#ifdef __cplusplus
}
#endif

#include <util/delay.h>

#define READ 0x01
#define WRITE 0x00

void SI5351_init(){
	i2c_init();
}

uint8_t SI5351_read(uint8_t address){
	uint8_t result, data=0;
	
	
	
	//try to address the device up to 50 times
	for (int i =0;i<50;i++){
		result = i2c_start(SI5351_ADDRESS);
		
		if (result == 0){
			i2c_write(address);
			i2c_start(SI5351_ADDRESS|READ);
			data = i2c_readNak();
			i2c_stop();
			continue;
		}
		i2c_stop();
	}
	
	
	return data;
}

///@brief
///@returns 0 if device was addressed properly
unsigned char SI5351_write(uint8_t address, uint8_t data){
	unsigned char result;
	
	//try to address the device up to 50 times
	for (int i =0;i<50;i++){
		result = i2c_start(SI5351_ADDRESS);
		
		if (result == 0){
			i2c_write(address);
			i2c_write(data);
			i2c_stop();
			continue;
		}
		i2c_stop();
	}
	
	
	return (result);
}
///Configures Multisynth
///assigns 32-bit values (p1, p2, p3) to their appropriate 8-bit registers
//rdiv is the output divider
///@returns 0 is address is accessible, 1 if not

unsigned char SI5351_configureMultisynth(uint8_t ms, uint32_t p1,  uint32_t p2, uint32_t p3, uint8_t rdiv){
	if (i2c_start(SI5351_ADDRESS) != 1){
		SI5351_write(42+ms*8+0, (p3>>8 ) & 0xFF);								//MSx_P3H_1
		SI5351_write(42+ms*8+1, (p3    ) & 0xFF);								//MSx_P3L
		SI5351_write(42+ms*8+2, ((p1>>16) & 0x03) | ((rdiv & 0x07) <<4) );								//MSx_P1H_2
		SI5351_write(42+ms*8+3, (p1>> 8) & 0xFF);								//MSx_P1H_1
		SI5351_write(42+ms*8+4, (p1    ) & 0xFF);								//MSx_P1L
		SI5351_write(42+ms*8+5, (((p3>>16) & 0xF)<<4) | ((p2>>16) & 0xF) );		//MSx_P3H_2 & MSx_P2H_2
		SI5351_write(42+ms*8+6, (p2>> 8) & 0xFF);								//MSx_P2H_1
		SI5351_write(42+ms*8+7, (p1    ) & 0xFF);								//MSx_P2L
		return 0;
	}else
		return 1;	
									
}

//configure the PLLA Feedback Multisynth divider (MSNA)
void SI5351_configureMultisynthNA(uint32_t p1,  uint32_t p2, uint32_t p3){
	
	i2c_start(SI5351_ADDRESS);
	SI5351_write(26, (p3>>8 ) & 0xFF);								//MSx_P3H_1
	SI5351_write(27, (p3    ) & 0xFF);								//MSx_P3L
	SI5351_write(28, (p1>>16) & 0x03);								//MSx_P1H_2
	SI5351_write(29, (p1>> 8) & 0xFF);								//MSx_P1H_1
	SI5351_write(30, (p1    ) & 0xFF);								//MSx_P1L
	SI5351_write(31, (((p3>>16) & 0xF)<<4) | ((p2>>16) & 0xF) );		//MSx_P3H_2 & MSx_P2H_2
	SI5351_write(32, (p2>> 8) & 0xFF);								//MSx_P2H_1
	SI5351_write(33, (p1    ) & 0xFF);
}

//configure the PLLB Feedback Multisynth divider (MSNB)
void SI5351_configureMultisynthNB(uint32_t p1,  uint32_t p2, uint32_t p3){
	i2c_start(SI5351_ADDRESS);
	SI5351_write(34, (p3>>8 ) & 0xFF);								//MSx_P3H_1
	SI5351_write(35, (p3    ) & 0xFF);								//MSx_P3L
	SI5351_write(36, (p1>>16) & 0x03);								//MSx_P1H_2
	SI5351_write(37, (p1>> 8) & 0xFF);								//MSx_P1H_1
	SI5351_write(38, (p1    ) & 0xFF);								//MSx_P1L
	SI5351_write(39, (((p3>>16) & 0xF)<<4) | ((p2>>16) & 0xF) );		//MSx_P3H_2 & MSx_P2H_2
	SI5351_write(40, (p2>> 8) & 0xFF);								//MSx_P2H_1
	SI5351_write(41, (p1    ) & 0xFF);
}

//configuring frequency:
/*
Refer to AN619 section 3:
Fvco = SI5351_F_XTAL * (a + b/c)

SI5351_F_XTAL is 25MHz

Fvco needs to be between 600MHz and 900MHz
if we want a VCO frequency of 600MHz, 
600MHz = 25E6 * 24

so, a = 24, b= 0, c=don't care

p1 = 128 * a + floor(128*b/c) -512 = 2560
p2 = 128 *b - c* floor(128*b/c) = 0
p3 = c = 1
//---------------------------------------

Fout = Fvco / (Multisynthx * Rx)

Fvco = 600MHz

Multisynthx needs to be between 6 and 1800, Rx = 2^Rx_DIV
Rx_DIV is between 0b000 and 0b111

for Fout = 200KHz 
Multisynthx * Rx = 3000
if Rx=2, Multisynthx = 1500

Multisynthx = 1500
*/

///@brief	sets the internal capacitance of the crystal
///@param capacitance valid values are SI5351_XTAL_6PF, SI5351_XTAL_8PF	
///						or SI5351_XTAL_10PF
void SI4341_setXTALcapacitance(uint8_t capacitance){
	//mask capacitance to be 2 bits
	//shift capacitance to the left into correct position
	//append 0x12 to bits 5:0 as the datasheet specifies
	
	capacitance &= 0b11;
	
	SI5351_write(SI5351_REG_XTAL_INTERNAL_LOAD_CAP, (((capacitance)<<SI5351_XTAL_CL) | 0x12) );
}