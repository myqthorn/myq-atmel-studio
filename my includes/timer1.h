//timer1.h


#ifndef _TIMER_1_H
#define _TIMER_1_H

#include "HardwareProfile.h"

#define TIMER_CLOCK_OFF		0
#define TIMER_DIV1			1
#define TIMER_DIV8			2
#define TIMER_DIV64			3
#define TIMER_DIV256		4
#define TIMER_DIV1024		5
#define TIMER_FALL_T1		6
#define TIMER_RISE_T1		7

#ifndef __cplusplus
	void	initTimer1(uint8_t prescalar, uint8_t WGMmode, uint16_t reload);
	void	startTimer1(void);
	void	stopTimer1(void);
	uint8_t isTimer1OVFlagSet();
	uint8_t	isTimer1CompareAFlagSet();
	void	clearTimer1Flag();
	
	uint8_t		prescalar;
	uint8_t		mode;
	uint16_t	reload;
	
#else
	class TIMER1{
		public:
			TIMER1();
			TIMER1(uint8_t pscalar, uint8_t WGMmode, uint16_t rload);
			void	start(void);
			void	stop(void);
			uint8_t isOVFlagSet();
			uint8_t	isCompareAFlagSet();
			void	clearOVFlag();
			void	clearCompareAFlag();
			void	setCount(uint16_t count);
			
		private:
			uint8_t		prescalar;
			uint8_t		mode;
			uint16_t	reload;
			
		};
		
#endif // cplusplus
#endif //_TIMER_1_H
