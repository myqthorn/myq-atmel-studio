
///Conversion time is dependent upon the ADC resolution setting.
///	Bits	Resolution	Conversion time
///	9-bit	 .5 degree		30ms  
///	10-bit	 .25 degree		60ms 
///	11-bit	 .125 degree	120ms 
///	12-bit	 .0625 degree	240ms 

#include "MCP980x.h"
/////////////////////////////////////////////////////////////
//this block is necessary to include C files in a C++ project
/////////////////////////////////////////////////////////////
#ifdef __cplusplus
extern "C" {
	#endif
	
	#include "myqI2C.h"

	#ifdef __cplusplus
}
#endif
/////////////////////////////////////////////////////////////


///@description	Assigns address assuming jumpers are set to 0b000
///@brief
MCP980x::MCP980x(){
	address = (MCP980X_ADDRESS_PREFIX);
	init();
}

///@description	Assigns address to device using specified jumper setting.
///@brief
///@param	jumpers	Refers to physical jumper settings on the hardware.
MCP980x::MCP980x(uint8_t jumpers){
	address = (MCP980X_ADDRESS_PREFIX | (jumpers & 0x07));
	init();
}

///@description
///@brief
///@param
void MCP980x::init(){
	i2c_init();
}

///@description
///@brief Reads the 2-byte value in the specified register.
///@param	reg	The register to be read from.
///@returns A pointer to the value in the two byte register.
void MCP980x::read(uint8_t reg, uint8_t *data){
		
	//Select register
	i2c_start(address | MCP9801X_WRITE );
	i2c_write(reg);
	
	i2c_start(address | MCP9801X_READ );
	data[0] = i2c_readAck();
	data[1] = i2c_readNak();
	i2c_stop();
	
}

///@description
///@brief Reads a byte from the specified register.
///@param	reg The register to be read from.
///@returns	The value in the specified 1-byte register.
uint8_t MCP980x::read(uint8_t reg){
	uint8_t data;
	
	//Select register
	i2c_start(address | MCP9801X_WRITE );
	i2c_write(reg);
	
	//read from address
	i2c_start(address | MCP9801X_READ );
	data = i2c_readNak();
	i2c_stop();
	
	return (data);
}

///@description
///@brief Writes a byte to the specified register.
///@param	reg The register to be written to.
///@param	data	The 1-byte data to be written to the register.
void MCP980x::write(uint8_t reg, uint8_t data){
		
	//Select register
	i2c_start(address | MCP9801X_WRITE );
	i2c_write(reg);
	
	//Write data
	i2c_write(data);
	i2c_stop();	
}

///@description
///@brief Writes two bytes to the specified register.
///@param	reg The register to be written to.
///@param	data	The 2-byte data to be written to the register.
void MCP980x::write(uint8_t reg, uint8_t * data){
		
	//Select register
	i2c_start(address | MCP9801X_WRITE );
	i2c_write(reg);
	
	//Write data
	i2c_write(data[0]);
	i2c_write(data[1]);
	i2c_stop();
}


///@description
///@brief
///@param
///@returns	A pointer to a 2-byte temperature
void MCP980x::getTemperature(uint8_t *temp){
	
	(read(MCP980X_REG_TEMPERATURE, temp));	
	
}

///@description 
///@brief	 Gets the value in the CONFIG register.
///@param
///@returns		1-byte value in the CONFIG register.
uint8_t	MCP980x::getConfig(){
	config = read(MCP980X_REG_CONFIG);
	return (config);	
}

///@description
///@brief 
void	MCP980x::setConfig(){
	write(MCP980X_REG_CONFIG, config);
	config &= 0x7F;						//mask one-shot bit for future use - One-shot bit automatically resets when conversion is done.
}

///@description
///@brief 
///@param	con	New value for CONFIG register
void	MCP980x::setConfig(uint8_t con){
	config = con;
	write(MCP980X_REG_CONFIG, config);
	config &= 0x7F;						//mask one-shot bit for future use - One-shot bit automatically resets when conversion is done.
}

///@description Puts device in shutdown mode and starts measurement. 
///					Temperature should be read after the conversion time has elapsed.
///@brief Performs a One-Shot temperature measurement.
///@param
void	MCP980x::beginOneShot(){
	
	//update config variable
	getConfig();
	
	//enable shutdown mode
	if(!(config & MCP980X_SHUTDOWN_EN)){
		enableShutdown();
	}
	
	//start one-shot temperature measurement
	setConfig(config | MCP980X_ONE_SHOT_EN );
}


///@description 
///@brief	Set's temperature resolution.
///@param res	Valid values are: MCP980X_RES_9BIT, MCP980X_RES_10BIT, MCP980X_RES_11BIT, or MCP980X_RES_12BIT
void	MCP980x::setResolution(uint8_t res){	
	config &= 0x9F;				// clear bits 6:5 in config 
	config |= (res & 0x60);		//Mask all bits other than 6:5
	setConfig();
}

///@description
///@brief
///@param	queue	Valid values are: MCP980X_FAULT_QUEUE_1, MCP980X_FAULT_QUEUE_2, MCP980X_FAULT_QUEUE_4, MCP980X_FAULT_QUEUE_6
void	MCP980x::setFaultQueue(uint8_t queue){
	config &= 0xF7;					// clear bits 4:3 in config 
	config |= (queue & 0x18);		//Mask all bits other than 4:3
	setConfig();
}

///@description
///@brief
///@param alertPolarity	Valid values are: MCP980X_ALERT_POL_HIGH or MCP980X_ALERT_POL_LOW.
void	MCP980x::setAlertPolarity(uint8_t alertPolarity){
	config &= 0xFB;							// clear bit 2 in config 
	config |= (alertPolarity & 0x04);		//Mask all bits other than 2
	setConfig();
}

///@description
///@brief
///@param
void	MCP980x::setComparatorMode(){
	config &= 0xFD;							//clear bit 1 in config 
	setConfig();
}

///@description
///@brief
///@param
void	MCP980x::setInterruptMode(){
	config |= MCP980X_MODE_INT;
	setConfig();
}

///@description	Updates config variable.
///@brief
///@param
void	MCP980x::enableShutdown(){
	config |= MCP980X_SHUTDOWN_EN;
	setConfig();
}

///@description
///@brief
///@param
void	MCP980x::disableShutdown(){
	config &= 0xFE;		//clear bit 0 of config 
	setConfig();
}

//TODO: finish
///@description
///@brief
///@param
void	MCP980x::setHighLimit(uint8_t * tempHigh){
	write(MCP980X_REG_TEMP_LIMIT, tempHigh);
}

//TODO: finish
///@description
///@brief
///@param
void	MCP980x::setLowLimit(uint8_t * tempLow){
	write(MCP980X_REG_TEMP_HYST, tempLow);
}

///@description
///@brief
///@returns
void MCP980x::getHighLimit(uint8_t *limit){
	read(MCP980X_REG_TEMP_LIMIT, limit);
}

///@description
///@brief
///@returns
void MCP980x::getLowLimit(uint8_t *limit){
	read(MCP980X_REG_TEMP_HYST, limit);
}
