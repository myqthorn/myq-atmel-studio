//portexpander.h
//Created: 6 Oct 2013
//Mike Litster

#ifndef PORTEXPANDER_H
#define PORTEXPANDER_H

//JUMPERS_23008 must be defined in HardwareProfile.h
#include "HardwareProfile.h"
#include "myqI2C.h"

//CTRL_BYTE is set up as such: x0100JJJ
//with the OPCODE being 0100, and JJJ are user-defined hardware address pins on the MCP23008,
//pins A2, A1 & A0
//these are intended to be shifted left one and the r/w bit appended to the LSB

#define OPCODE_23008 (0X04)
#define CTRL_BYTE_23008 (OPCODE_23008<<3|JUMPERS_23008<<0)


//MCP23008 register address defines:
#define IODIR   0x00
#define IPOL    0x01
#define GPINTEN 0x02
#define DEFVAL  0x03
#define INTCON  0x04
#define IOCON   0x05
#define GPPU    0x06
#define INTF    0x07
#define INCAP   0x08
#define GPIO    0x09
#define OLAT    0x0A

//TODO: redefine these-
#define SetTris23008(data) Write23008(IODIR, data)

void Write23008(unsigned char reg, unsigned char data);
void Init23008(void);
unsigned char   Read23008(unsigned char reg);

#endif
