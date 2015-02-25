/*MCP908X.h
Mike Litster
23 Oct 2013

for interfacing with the Microchip MCP9800/1/2/3 I2C Temperature sensors.








*/
#include <avr/io.h>
//The 4 MSB's in the address of all MSP980x's are 1001
//The following 3 are set by jumpers
#define MCP980X_ADDRESS_PREFIX 		0x90

#define MCP9801X_WRITE	0b0
#define MCP9801X_READ	0b1

//register defines
#define MCP980X_REG_TEMPERATURE			0x00
#define MCP980X_REG_CONFIG				0x01
#define MCP980X_REG_TEMP_HYST			0x02
#define MCP980X_REG_TEMP_LIMIT			0x03


//CONFIG register defines:
/*
Pick one from each group and OR them together:
(MCP980X_CONFIG_ONE_SHOT_EN | MCP980X_CONFIG_RES_10BIT | MCP980X_CONFIG_FAULT_QUEUE_2 | 
MCP980X_CONFIG_ALERT_POL_HIGH | MCP980X_CONFIG_MODE_INT | MCP980X_CONFIG_SHUTDOWN_EN)
This would be equivalent to:
(0x80 | 0x20 | 0x08	| 0x04 |0x02| 0x01)  = 1010 1111 = 0xAF
*/
#define MCP980X_ONE_SHOT_EN		0x80
#define MCP980X_ONE_SHOT_DIS	0x00

#define MCP980X_RES_9BIT		0x00
#define MCP980X_RES_10BIT		0x20
#define MCP980X_RES_11BIT		0x40
#define MCP980X_RES_12BIT		0x60

#define MCP980X_FAULT_QUEUE_1	0x00
#define MCP980X_FAULT_QUEUE_2	0x08
#define MCP980X_FAULT_QUEUE_4	0x10
#define MCP980X_FAULT_QUEUE_6	0x18

#define MCP980X_ALERT_POL_HIGH	0x04
#define MCP980X_ALERT_POL_LOW	0x00

#define MCP980X_MODE_INT		0x02
#define MCP980X_MODE_COMP		0x00

#define MCP980X_SHUTDOWN_EN		0x01
#define MCP980X_SHUTDOWN_DIS	0x00


class MCP980x {

	public:
		MCP980x();
		MCP980x(uint8_t jumpers);
		void	init();
		void getTemperature(uint8_t *temp);
		uint8_t	getConfig();
		void	setConfig();
		void	setConfig(uint8_t config);
		void	beginOneShot();
		void	setResolution(uint8_t res);
		void	setFaultQueue(uint8_t queue);
		void	setAlertPolarity(uint8_t alertPolarity);
		void	setComparatorMode();
		void	setInterruptMode();
		void	enableShutdown();
		void	disableShutdown();
		void	setHighLimit(uint8_t * tempHigh);
		void	setLowLimit(uint8_t * tempLow);
		void getHighLimit(uint8_t *limit);
		void getLowLimit(uint8_t *limit);
		
	private:
		void read(uint8_t reg, uint8_t *data);
		uint8_t read(uint8_t reg);
		void write(uint8_t reg, uint8_t data);
		void write(uint8_t reg, uint8_t * data);
		uint8_t	address;
		uint8_t 	config;
		
}; //end class MCP980x
