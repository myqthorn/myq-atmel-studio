/*
 * InterruptsPractice.c
 *
 * Created: 8/30/2013 7:45:47 PM
 *  Author: myQ
 */ 

#define F_CPU (8000000)
#define EEADDRESS (0x0100)

#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>




volatile int button_was_pressed;

void EEPROM_write(unsigned int address, unsigned char data);
unsigned char EEPROM_read(unsigned int address);
void blinkLight(unsigned char count);


int main(void)
{
	unsigned char numRuns;		//total number of times this code has run (stored in EEPROM)
	
	//Set up IO pins-----------------------------------------------------------------------------------------
	DDRB = 0x20;		//all inputs except B5 output
	
	//INT0 is on PD2 pin, enable as input, turn on pullup
	DDRD = 0x00;		//all inputs
	PORTD = 0x04;		//pull up resistor on (PD2)
	
	//Set up Interrupts--------------------------------------------------------------------------------------
	//External Interrupt Mask Register
	EIMSK |= (1<<INT0);	//turn on interrupt 0 (PD2)
		
	////External Interrupt Control Register A
	EICRA = 0x02;		//Interrupt on falling edge of INT0
	
	//before interrupts are enabled, read numRuns from EEPROM, increment and write new value
	numRuns = EEPROM_read(EEADDRESS)+1;
	EEPROM_write(EEADDRESS,numRuns);
	
	//blink light one time for each time the program has been run
	blinkLight(numRuns);
	
	sei();						//enable global interrupts
	
	while(1)
	{
		
		if (button_was_pressed){
			PORTB ^= (1<<PB5);		//toggle LED
			button_was_pressed = 0;	//reset button press flag
			
		}
	}
}

	ISR(INT0_vect){
		//to debounce switch, wait 5ms, then check for low condition again
		_delay_ms(5);
		if (!(PIND & (1<<PD2))) 
			button_was_pressed = 1;
		
	}
	
	
	//good idea to disable interrupts before running this to avoid
	//failing to write
	void EEPROM_write(unsigned int address, unsigned char data){
		
		//wait for completion of previous write
		while (EECR & (1<<EEPE));
		
		//setup address and data registers
		EEAR = address;
		EEDR = data;
		
		//write a logical 1 to EEMPE
		EECR |= (1<<EEMPE);
		
		//start EEPROM write by setting EEPE
		//(this must be done within 4 clock cycles of setting EEMPE)
		EECR |= (1<<EEPE);
		
	}	
	
	
	unsigned char EEPROM_read(unsigned int address){
		
		//wait for completion of previous write
		while (EECR & (1<<EEPE));
		
		//setup address register
		EEAR = address;
		
		//start EEPROM read by setting EERE
		EECR |= (1<<EERE);
		
		//return data from data register
		return EEDR;
		
		
	}
	
	void blinkLight(unsigned char count){
		
		for (int i=0;i<count;i++){
			PORTB ^= (1<<PB5);		//toggle LED
			_delay_ms(50);
			PORTB ^= (1<<PB5);		//toggle LED
			_delay_ms(50);
			
		}
	}
