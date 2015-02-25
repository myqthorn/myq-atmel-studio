/*
 * File:   nunchuk.h
 * Author: myQ
 *
 *  17 Aug 2011     Created
 *  08 Mar 2012     Added PIC24 support
 *  26 Apr 2013     Functioned properly for PIC24 today
 */

#ifndef nunchuk_H
#define nunchuk_H
#define NUNCHUK_ADDR 0xA4       //10100100(0xA4)  = 01010010(0x52)<<1


// Nunchuk Init: Initialize the nunchuk by sending I2C commands to make the
// nunchuk "active".

void InitNunchuk();


// Nunchuk Read: Read the six bytes of data from the nunchuk. The read must be
// preceded by sending a zero to the nunchuk.
void ReadNunchuk(unsigned char* array);


#endif
