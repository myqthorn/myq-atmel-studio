#include "nRF8001.h"

void NRF::NRF(){
	init();
}

void NRF::init(){
	//ACT and RDYN set as inputs	
	ACT_DDR &= ~(1<<ACT_PIN);
	RDYN_DDR &= ~(1<<RDYN_PIN);
	
	//REQN set as output
	REQN_DDR |= (1<<REQN_PIN);
	
	//set pullup on RDYN(PB2)
	
	//REQN is active low, so initialize it HIGH
	SetREQN();
}

bool sendSystemCommand(){
	
}

uint8_t * sendDataCommand(){
	
}