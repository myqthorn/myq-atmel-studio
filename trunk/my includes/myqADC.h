/*
 * myqADC.h
 *
 * Created: 2/14/2015 2:09:10 PM
 *  Author: myQ
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

#ifndef __cplusplus

void	initADC(uint8_t channel, uint8_t prescalar);
void	startADC(uint8_t channel);
void	readADC(uint8_t channel);
uint8_t	isADCfinished();
void	clearADCinterrupt();
void	ADCoff();

#else

class ANALOG{
	public:
					ANALOG();
					ANALOG(uint8_t channel, uint8_t prescalar);
		void		reinitialize();
		void		start();
		uint16_t	read();
		uint8_t		isReading();
		uint8_t		isInterruptFlagSet();
		void		clearFlag();
		void		off();
	private:
		uint8_t channel;
		uint8_t prescalar;	
			
	
}; //A2D

#endif //cplusplus
#endif /* MYQADC_H_ */