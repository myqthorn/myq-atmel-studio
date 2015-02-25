/*
 * myqADC.h
 *
 * Created: 2/14/2015 2:09:10 PM
 *  Author: myQ_2
 */ 



#ifndef MYQADC_H_
#define MYQADC_H_

#include "HardwareProfile.h"

#define ADC_DIV2	0
#define ADC_DIV4	2
#define ADC_DIV8	3
#define ADC_DIV16	4	
#define ADC_DIV32	5
#define ADC_DIV64	6
#define ADC_DIV128	7

void	initADC(uint8_t channel, uint8_t prescalar);
void	startADC(uint8_t channel);
void	readADC(uint8_t channel);
uint8_t	isADCfinished();
void	clearADCinterrupt();
void	ADCoff();

#endif /* MYQADC_H_ */