/*
 * si5351.h
 *
 * Created: 2/21/2014 5:28:47 PM
 *  Author: Mike Litster
 
 Description:
	Interfaces with Silicon Labs Si5351
	I2C programmable Any-Frequency CMOS Clock Generator + VCXO
	
 */ 
#include <avr/io.h>
#include <stdlib.h>
#include "HardwareProfile.h"

//this should be defined in HardwareProfile.h
#ifndef SI5351_F_XTAL
	#define SI5351_F_XTAL	(25000000L)
#endif

#ifndef SI5351_H_
#define SI5351_H_

//only need to add 1 for a read
//110 0000<<1 = 1100 0000(0xC0)
#define SI5351_ADDRESS	0xC0

//Device Status Register
#define SI5351_REG_DEVICE_STATUS	0
#define SI5351_SYS_INIT				7
#define SI5351_LOL_B				6
#define SI5351_LOL_A				5
#define SI5351_LOS					4
//bits 1:0 are the device revision number. (set at factory)

//Interrupt Status Sticky Register
#define SI5351_REG_INTERRUPT_STATUS_STICKY	1
#define SI5351_SYS_INIT_STKY	7
#define SI5351_LOL_B_STKY		6
#define SI5351_LOL_A_STKY		5
#define SI5351_LOS_STKY			4

//Interrupt Status Mask
#define SI5351_REG_INTERRUPT_STATUS_MASK	2
#define SI5351_SYS_INIT_MASK	7
#define SI5351_LOL_B_MASK		6
#define SI5351_LOL_A_MASK		5
#define SI5351_LOS_MASK			4

//Output Enable Control Register
#define SI5351_REG_OUTPUT_ENABLE_CONTROL	3
#define SI5351_CLK7_OEB			7
#define SI5351_CLK6_OEB			6
#define SI5351_CLK5_OEB			5
#define SI5351_CLK4_OEB			4
#define SI5351_CLK3_OEB			3
#define SI5351_CLK2_OEB			2
#define SI5351_CLK1_OEB			1
#define SI5351_CLK0_OEB			0

//PLL Input Source Register
#define SI5351_REG_PLL_INPUT_SOURCE			15
#define SI5351_PLLB_SRC			3
#define SI5351_PLLA_SRC			2

//CLK0 Control Register
#define SI5351_REG_CLK0_CONTROL				16
#define SI5351_CLK0_PDN			7
#define SI5351_MS0_INT			6
#define SI5351_MS0_SRC			5
#define SI5351_CLK0_INV			4
#define SI5351_CLK0_SRC			2	
#define SI5351_CLK0_IDRV		0	

//CLK1 Control Register
#define SI5351_REG_CLK1_CONTROL				17
#define SI5351_CLK1_PDN			7
#define SI5351_MS1_INT			6
#define SI5351_MS1_SRC			5
#define SI5351_CLK1_INV			4
#define SI5351_CLK1_SRC			2	
#define SI5351_CLK1_IDRV		0	

//CLK2 Control Register
#define SI5351_REG_CLK2_CONTROL				18
#define SI5351_CLK2_PDN			7
#define SI5351_MS2_INT			6
#define SI5351_MS2_SRC			5
#define SI5351_CLK2_INV			4
#define SI5351_CLK2_SRC			2	
#define SI5351_CLK2_IDRV		0	

//CLK3 Control Register
#define SI5351_REG_CLK3_CONTROL				19
#define SI5351_CLK3_PDN			7
#define SI5351_MS3_INT			6
#define SI5351_MS3_SRC			5
#define SI5351_CLK3_INV			4
#define SI5351_CLK3_SRC			2	
#define SI5351_CLK3_IDRV		0	

//CLK4 Control Register
#define SI5351_REG_CLK4_CONTROL				20
#define SI5351_CLK4_PDN			7
#define SI5351_MS4_INT			6
#define SI5351_MS4_SRC			5
#define SI5351_CLK4_INV			4
#define SI5351_CLK4_SRC			2	
#define SI5351_CLK4_IDRV		0	

//CLK5 Control Register
#define SI5351_REG_CLK5_CONTROL				21
#define SI5351_CLK5_PDN			7
#define SI5351_MS5_INT			6
#define SI5351_MS5_SRC			5
#define SI5351_CLK5_INV			4
#define SI5351_CLK5_SRC			2	
#define SI5351_CLK5_IDRV		0	

//CLK6 Control Register
#define SI5351_REG_CLK6_CONTROL				22
#define SI5351_CLK6_PDN			7
#define SI5351_MS6_INT			6
#define SI5351_MS6_SRC			5
#define SI5351_CLK6_INV			4
#define SI5351_CLK6_SRC			2	
#define SI5351_CLK6_IDRV		0	

//CLK7 Control Register
#define SI5351_REG_CLK7_CONTROL				23
#define SI5351_CLK7_PDN			7
#define SI5351_MS7_INT			6
#define SI5351_MS7_SRC			5
#define SI5351_CLK7_INV			4
#define SI5351_CLK7_SRC			2	
#define SI5351_CLK7_IDRV		0	

//CLK3_0 Disable State Register
#define SI5351_REG_CLK3_0_DISABLE_STATE		24
#define SI5351_CLK3_DIS_STATE	6	
#define SI5351_CLK2_DIS_STATE	4
#define SI5351_CLK1_DIS_STATE	2
#define SI5351_CLK0_DIS_STATE	0

//CLK7_4 Disable State Register
#define SI5351_REG_CLK7_4_DISABLE_STATE		25
#define SI5351_CLK7_DIS_STATE	6
#define SI5351_CLK6_DIS_STATE	4
#define SI5351_CLK5_DIS_STATE	2
#define SI5351_CLK4_DIS_STATE	0

//

//Multisynth0 Parameter Registers
#define SI5351_REG_MS0_P3H_1		42
#define SI5351_REG_MS0_P3L			43
#define SI5351_REG_MS0_PARAMETERS	44
#define SI5351_R0_DIV		4
#define SI5351_MS0_P1H_2	0
#define SI5351_REG_MS0_P1H_1		45
#define SI5351_REG_MS0_P1L			46
#define SI5351_REG_MS0_PARAMETERS_2	47
#define SI5351_MS0_P3H_2	4
#define SI5351_MS0_P2H_2	0
#define SI5351_REG_MS0_P2H_1		48
#define SI5351_REG_MS0_P2L			49

//Multisynth1 Parameter Registers
#define SI5351_REG_MS1_P3H_1		50
#define SI5351_REG_MS1_P3L			51
#define SI5351_REG_MS1_PARAMETERS	52
#define SI5351_R1_DIV		4
#define SI5351_MS1_P1H_2	0
#define SI5351_REG_MS1_P1H_1		53
#define SI5351_REG_MS1_P1L			54
#define SI5351_REG_MS1_PARAMETERS_2	55
#define SI5351_MS1_P3H_2	4
#define SI5351_MS1_P2H_2	0
#define SI5351_REG_MS1_P2H_1		56
#define SI5351_REG_MS1_P2L			57

//Multisynth2 Parameter Registers
#define SI5351_REG_MS2_P3H_1		58
#define SI5351_REG_MS2_P3L			59
#define SI5351_REG_MS2_PARAMETERS	60
#define SI5351_R2_DIV		4
#define SI5351_MS2_P1H_2	0
#define SI5351_REG_MS2_P1H_1		61
#define SI5351_REG_MS2_P1L			62
#define SI5351_REG_MS2_PARAMETERS_2	63
#define SI5351_MS2_P3H_2	4
#define SI5351_MS2_P2H_2	0
#define SI5351_REG_MS2_P2H_1		64
#define SI5351_REG_MS2_P2L			65

//Multisynth3 Parameter Registers
#define SI5351_REG_MS3_P3H_1		66
#define SI5351_REG_MS3_P3L			67
#define SI5351_REG_MS3_PARAMETERS	68
#define SI5351_R3_DIV		4
#define SI5351_MS3_P1H_2	0
#define SI5351_REG_MS3_P1H_1		69
#define SI5351_REG_MS3_P1L			70
#define SI5351_REG_MS3_PARAMETERS_2	71
#define SI5351_MS3_P3H_2	4
#define SI5351_MS3_P2H_2	0
#define SI5351_REG_MS3_P2H_1		72
#define SI5351_REG_MS3_P2L			73

//Multisynth4 Parameter Registers
#define SI5351_REG_MS4_P3H_1		74
#define SI5351_REG_MS4_P3L			75
#define SI5351_REG_MS4_PARAMETERS	76
#define SI5351_R4_DIV		4
#define SI5351_MS4_P1H_2	0
#define SI5351_REG_MS4_P1H_1		77
#define SI5351_REG_MS4_P1L			78
#define SI5351_REG_MS4_PARAMETERS_2	79
#define SI5351_MS4_P3H_2	4
#define SI5351_MS4_P2H_2	0
#define SI5351_REG_MS4_P2H_1		80
#define SI5351_REG_MS4_P2L			81

//Multisynth5 Parameter Registers
#define SI5351_REG_MS5_P3H_1		82
#define SI5351_REG_MS5_P3L			83
#define SI5351_REG_MS5_PARAMETERS	84
#define SI5351_R5_DIV		4
#define SI5351_MS5_P1H_2	0
#define SI5351_REG_MS5_P1H_1		85
#define SI5351_REG_MS5_P1L			86
#define SI5351_REG_MS5_PARAMETERS_2	87
#define SI5351_MS5_P3H_2	4
#define SI5351_MS5_P2H_2	0
#define SI5351_REG_MS5_P2H_1		88
#define SI5351_REG_MS5_P2L			89

//Multisynth 6 & 7 Parameter Registers
#define SI5351_REG_MS6_P1			90
#define SI5351_REG_MS7_P1			91
#define SI5351_REG_CLK_6_7_OUTPUT_DIVIDER	92
#define SI5351_R7_DIV		4
#define SI5351_R6_DIV		0

#define SI5351_REG_CLK0_PHOFF		165
#define SI5351_REG_CLK1_PHOFF		166
#define SI5351_REG_CLK2_PHOFF		167
#define SI5351_REG_CLK3_PHOFF		168
#define SI5351_REG_CLK4_PHOFF		169
#define SI5351_REG_CLK5_PHOFF		170

#define SI5351_REG_PLL_RESET		177
#define SI5351_PLLB_RST		7
#define SI5351_PLLA_RST		5

#define SI5351_REG_XTAL_INTERNAL_LOAD_CAP	183
#define SI5351_XTAL_CL		6
#define SI5351_XTAL_6PF		0b01
#define SI5351_XTAL_8PF		0b10
#define SI5351_XTAL_10PF	0b11
//Bits 5:0 should be written to 010010b.

void SI5351_init();
uint8_t SI5351_read(uint8_t address);
unsigned char SI5351_write(uint8_t address, uint8_t data);

void SI5351_configureMultisynthNA(uint32_t p1,  uint32_t p2, uint32_t p3);
void SI5351_configureMultisynthNB(uint32_t p1,  uint32_t p2, uint32_t p3);
unsigned char SI5351_configureMultisynth(uint8_t ms, uint32_t p1,  uint32_t p2, uint32_t p3, uint8_t rdiv);
void SI4341_setXTALcapacitance(uint8_t capacitance);

#endif /* SI5351_H_ */