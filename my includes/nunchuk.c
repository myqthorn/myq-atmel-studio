/*
 * File:   nunchuk.c
 * Author: myQ
 *
 * 17 Aug 2011  created
 * 07 Oct 2013 modified for Atmega168
 */


#include "HardwareProfile.h"

#ifdef __cplusplus
extern "C" {
#endif
	#include "nunchuk.h"
	#include "myqI2C.h"
#ifdef __cplusplus
}
#endif

#include <util/delay.h>

#define READ 0x01
#define WRITE 0x00

void InitNunchuk(void){
	
	i2c_init();
    i2c_start(NUNCHUK_ADDR);       
    i2c_write(0xF0);
    i2c_write(0x55);
    i2c_stop();
    i2c_start(NUNCHUK_ADDR);
    i2c_write(0xFB);
    i2c_write(0x00);
    i2c_stop();
}

// Nunchuk Read: Read the six bytes of data from the nunchuk. The read must be
// preceded by sending a zero to the nunchuk.
void ReadNunchuk(unsigned char* array) {
    
    i2c_start(NUNCHUK_ADDR);
    i2c_write(0x00);
    i2c_stop();                  
    _delay_ms(1);
	    
    i2c_start(NUNCHUK_ADDR | 0x01);
    array[0] = i2c_readAck();
    array[1] = i2c_readAck();
    array[2] = i2c_readAck();
    array[3] = i2c_readAck();
    array[4] = i2c_readAck();
    array[5] = i2c_readNak();
    i2c_stop();                  
}//ReadNunchuk


