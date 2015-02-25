//HardwareProfile.h
//Created: 6 Oct 2013
//Mike Litster

#define F_CPU (8000000L)
#define SI5351_F_XTAL	(25000000L)
#define SCL_CLOCK	(200000L)

#define TRUE          1
#define FALSE         0

//we are using the following pins:
//Voltage sense:	PC0 / ADC0
//Current sense:	PC1 / ADC1
//Frequency set:    ADC6
//Frequency sense:	PD2 / INT0