//portexpander.c
//Created: 6 Oct 2013
//Mike Litster

#include "portexpander.h"

void Write23008(unsigned char reg, unsigned char data){
	i2c_start((CTRL_BYTE_23008<<1) | I2C_WRITE); // Slave address.  Shift 7-bit address one bit, and clear the read bit (bit0).)
	i2c_write(reg);
	i2c_write(data);
	i2c_stop();
}

unsigned char   Read23008(unsigned char reg){
	unsigned char data;
	
	
	i2c_start(CTRL_BYTE_23008<<1 | I2C_WRITE);
	i2c_write(reg);
	i2c_rep_start(CTRL_BYTE_23008<<1 | I2C_READ);
	data = i2c_readNak();
	i2c_stop();

	return data;
	}
	
void Init23008(void){
	i2c_init();
	
}