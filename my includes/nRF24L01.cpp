/*
@file NRF.cpp
@author Mike Litster
@version 1.0

<Interface with NRF24L01+ Single Chip 2.4GHz Transceiver>

*/
#include "HardwareProfile.h"

#include "NRF24L01.h"
/////////////////////////////////////////////////////////////
//this block is necessary to include C files in a C++ project
/////////////////////////////////////////////////////////////
#ifdef __cplusplus
extern "C" {
	#endif
	
	#include "myqspi.h"

	#ifdef __cplusplus
}
#endif
/////////////////////////////////////////////////////////////

#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
//#include <stdint.h>

extern class NRF transmitter;

///@description Basic constructor.
///			Sets default channel and payload length.
NRF::NRF(){
	
	init();
	setChannel(0);					//set channel
	setPayloadLength(1);			//set length of payloads (RX)
	//setMode(NRF_MODE_RX);		//start receiving
}

///@description	Custom constructor.
///@param ch		Custom channel.
///@param pl		Custom payload length.
NRF::NRF(uint8_t ch, uint8_t pl){
	
	init();
	setChannel(ch);					//set channel
	setPayloadLength(pl);			//set length of payloads (RX)
	//setMode(NRF_MODE_RX);		//start receiving
}

//////////////////////////////////////////////////////////////////////////
//Private functions
//////////////////////////////////////////////////////////////////////////

///@description	CE needs to be be high to take the nRF out of standby mode
///					into either TX or RX mode.
///@brief	Sets the CE pin high.
void setCE(){
	NRF_CE_PORT |= (1<<NRF_CE_PIN);
}

///@description Clearing the CE pin puts the nRF into standby mode.
///@brief	Clears the CE pin to low.
void clearCE(){
	NRF_CE_PORT &= ~(1<<NRF_CE_PIN);
}

///@description
///@brief		Sets the CSN(/Cable Select), disabling SPI communication to the nRF.
void setCSN(){
	NRF_CSN_PORT |= (1<<NRF_CSN_PIN);
}

///@description		All SPI commands need to be started by a high to low transisition on CSN.
///@brief			Clears the CSN(/Cable Select) pin to low.  
void clearCSN(){
	NRF_CSN_PORT &= ~(1<<NRF_CSN_PIN);
}

///@description	Returns the receive address of the dataPipe
///@param	dataPipe	The dataPipe index. 0-5
///@returns	The address of the referenced dataPipe.
uint8_t getDataPipeAddress(uint8_t dataPipe){
	switch (dataPipe){
		case 1:
			return (NRF_RX_ADDR_P1);
			break;
		case 2:
			return (NRF_RX_ADDR_P2);
			break;
		case 3:
			return (NRF_RX_ADDR_P3);
			break;
		case 4:
			return (NRF_RX_ADDR_P4);
			break;
		case 5:
			return (NRF_RX_ADDR_P5);
			break;
		default:
			return (NRF_RX_ADDR_P0);
			break;
	}//end switch
}//end getDataPipeAddress

///@description	Returns the address of the dataPipe width.
///@param	dataPipe	The dataPipe index. 0-5.
///@returns	The address of the referenced dataPipe's width.
uint8_t getDataPipePayloadWidthAddress(uint8_t dataPipe){
	switch (dataPipe){
		case 1:
		return (NRF_RX_PW_P1);
		break;
		case 2:
		return (NRF_RX_PW_P2);
		break;
		case 3:
		return (NRF_RX_PW_P3);
		break;
		case 4:
		return (NRF_RX_PW_P4);
		break;
		case 5:
		return (NRF_RX_PW_P5);
		break;
		default:
		return (NRF_RX_PW_P0);
		break;
	}//end switch
}//end getDataPipeAddress

///@description		Sets status = STATUS register
///@brief			Reads one byte from the specified register.
///@param reg		The register to be accessed.
///@returns			Data read from register.
uint8_t NRF::read(uint8_t reg){
	uint8_t data;
	clearCSN();
		status = spiTransfer1byte(NRF_R_REGISTER | reg);
		data = spiTransfer1byte(0x00);				//dummy byte
	setCSN();
	return data;
}

///@description		Sets status = STATUS register
///@brief			Reads multiple bytes from the specified register.
///@param reg		The register to be accessed.
///@param val		Pointer to values to be read.
///@param length	Number of bytes to be written.
void NRF::read(uint8_t reg, uint8_t * val, uint8_t length){
	clearCSN();
		status = spiTransfer1byte(NRF_R_REGISTER | reg);
		spiRead(val, length);
	setCSN();
}

///@description		Used for 1-byte commands that return no value after the STATUS byte.
///					Sets status = STATUS register
///@brief			Sends a 1-byte command into the assigned register.
///@param reg		The register to be accessed.
void NRF::write(uint8_t reg){
	clearCSN();
		status = spiTransfer1byte(NRF_W_REGISTER | reg);
	setCSN();
}

///@description		Used for registers that have 1-byte values written to them.
///					Sets status = STATUS register
///@brief			Writes 1 byte into the specified register.
///@param reg		The register to be accessed.
///@param data		The data you want written there.
void NRF::write(uint8_t reg, uint8_t data){
	clearCSN();
		status = spiTransfer1byte(NRF_W_REGISTER | reg);
		spiTransfer1byte(data);
	setCSN();
}

///@description		Used for registers that have multiple bytes written to them.
///@brief			Writes multiple bytes into the specified register.
///@param reg		The register to be accessed
///@param val		Pointer to values to be written
///@param length	Number of bytes to be written
void NRF::write(uint8_t reg, uint8_t * data, uint8_t length){
	clearCSN();
		status = spiTransfer1byte(NRF_W_REGISTER | reg);
		spiWrite(data, length);
	setCSN();
}

//////////////////////////////////////////////////////////////////////////
//Public functions
//////////////////////////////////////////////////////////////////////////

///@description		Initializes MCU pins, interrupt, and SPI module.				
///					Should be called during startup.
///@brief			Initializes MCU to communicate with nRF25L01 module.
void NRF::init(){
	//define CE and CSN as Outputs
	NRF_CE_DDR		|= (1<<NRF_CE_PIN);
	NRF_CSN_DDR	|= (1<<NRF_CSN_PIN);
	
	//default values for CE & CSN
	clearCE();
	setCSN();
		
	//////////////////////////////////////////////////////////////////////////
	//Interrupts
	//////////////////////////////////////////////////////////////////////////
	//External Interrupt Mask Register
	EIMSK |= (1<<INT0);	//turn on interrupt 0 (PD2)
	
	////External Interrupt Control Register A
	EICRA = 0x02;		//Interrupt on falling edge of INT0
	/*
	// Initialize external interrupt on port PD6 (PCINT22)
	// First we set the pin as an input, then we set the mask(PCMSK2) so that the
	// pin isn't blocked, then we enable the group.
	DDRB &= ~(1<<PD6);			//Set PD6 as input
	PCMSK2 = (1<<PCINT22);		//Enable pin change interrupt for PCINT22 (PD6)
	PCICR  = (1<<PCIE2);		//Enable pin change interrupt for PCINT23..16 (group)
	
	*/
	//////////////////////////////////////////////////////////////////////////
	
	
	//Initialize SPI module
	spiInit();
	
	_delay_ms(50);
	
	//Interrupt LED
	DDRB |= (1<<PB7);			//Set PB7 as output
		
}	//init()

///@description		
///@brief			Flushes TX FIFO
void NRF::flushTX(){
	write(NRF_FLUSH_TX);
}

///@description
///@brief			Flushes RX FIFO
void NRF::flushRX(){
	write(NRF_FLUSH_RX);
}
//////////////////////////////////////////////////////////////////////////
//CONFIG register function
//////////////////////////////////////////////////////////////////////////

///@description 
///@brief			Sets the CONFIG register to specified value.
///@param val		Value to set CONFIG register to. 
void NRF::setConfig(uint8_t val){	
	write(NRF_CONFIG, val & 0x7F);
	
}

///@description
///@brief			Gets the value of the CONFIG register.
///@returns			Value of the CONFIG register.
uint8_t NRF::getConfig(){
	uint8_t config;
	config = read(NRF_CONFIG);
	
	return(config);
	//return (read(NRF_CONFIG));	
	
}

///@description
///@brief		Disables specified interrupts.
///@param	irq	Specifies which interrupts to disable.  To disable all 3 interrupts: 
///					(1<<NRF_MASK_RX_DR | 1<<NRF_MASK_TX_DS | 1<<NRF_MASK_MAX_RT)
void NRF::maskIRQ(uint8_t irq){
	irq &= 0x70;							//Mask valid values of irq
	setConfig((getConfig() & 0x0F) |(irq));
}
///@description		Sets the PWR_UP bit of the CONFIG register to high.
///@brief			Powers up the nRF.
void NRF::powerUp(){
	setConfig (getConfig() | (1<<NRF_PWR_UP));
}

///@description		Clears the PWR_UP bit of the CONFIG register to low.
///@brief			Powers down the nRF.
void NRF::powerDown(){
	setConfig (getConfig() & ~(1<<NRF_PWR_UP));
}

///@description		Clears the PWR_UP bit of the CONFIG register to low.
///@brief			Powers down the nRF.
uint8_t NRF::isPoweredUp(){
	return ((getConfig() & (1<<NRF_PWR_UP))>>NRF_PWR_UP);
}

///@description	Cyclic redundancy check can either be off, 1-byte or 2-bytes.
///@brief		Sets up the CRC for the nRF.
///@param	crc	Valid values:	NRF_CRC_OFF, NRF_CRC_1BYTE, NRF_CRC_2BYTE
void NRF::setCRC(uint8_t crc){
	crc &= 0x03;
	setConfig((getConfig() & 0xF3) | (crc<<NRF_CRCO));
}

///@description
///@brief			Sets power up and RX/TX modes in CONFIG register.
///@returns			NRF_MODE_TX or NRF_MODE_RX
void NRF::setMode(uint8_t mode){
	uint8_t config = read(NRF_CONFIG);
	config &= 0x7E;			//mask unaltered bits
	if (mode == NRF_MODE_RX)
	config |= (1<<NRF_PRIM_RX);
	setConfig(config);
}

///@description
///@brief			Retrieve mode.
///@returns			NRF_MODE_TX or NRF_MODE_RX
uint8_t NRF::getMode(void){
	return (getConfig() & 0x01);
}

//////////////////////////////////////////////////////////////////////////
//EN_AA register function
//////////////////////////////////////////////////////////////////////////

///@description
///@brief			Enable Enhanced ShockBurst Auto Acknowledge for the specified datapipes.
///@param datapipes	Valid bit values: 1<<NRF_ENAA_P0, 1<<NRF_ENAA_P1, etc.
void	NRF::enableAutoAcknowledge(uint8_t datapipes){
	write(NRF_EN_AA, datapipes & 0x3F);
}

///@description
///@brief			Returns value of EN_AA register
///@returns			Value of EN_AA register.
uint8_t	NRF::getENAA(){
	return (read(NRF_EN_AA));
}

//////////////////////////////////////////////////////////////////////////
//EN_RXADDR register function
//////////////////////////////////////////////////////////////////////////

///@description
///@brief			Enable the specified datapipes.
///@param datapipes	Valid bit values: 1<<NRF_ERX_P0, 1<<NRF_ERX_P1, etc.
void	NRF::enableRXDatapipes(uint8_t datapipes){
	write(NRF_EN_RXADDR, datapipes & 0x3F);
}

///@description
///@brief			Returns value of EN_RX_ADDR register
///@returns			Value of EN_RX_ADDR register.
uint8_t	NRF::getENRXADDR(){
	return (read(NRF_EN_RXADDR));
}

//////////////////////////////////////////////////////////////////////////
//SETUP_AW register function
//////////////////////////////////////////////////////////////////////////

///@description
///@brief			Sets the width of addresses for all data pipes.
///@param	addressWidth	Valid values: NRF_AW_3BYTES, NRF_AW_4BYTES,
///								NRF_AW_5BYTES
void NRF::setAddressWidth(uint8_t addressWidth){
	//clearCE();
	write(NRF_SETUP_AW, addressWidth & 0x03);
	//setCE();
}
//////////////////////////////////////////////////////////////////////////
//SETUP_RETR register function
//////////////////////////////////////////////////////////////////////////

///@description
///@brief			Sets the Auto Transmit Delay and the number of retransmits.
///@param	val		Valid bit values: 
///						bits 7:4 are the Auto Retransmit Delay.  
///						0000 = 250uS, 0001 = 500uS .... 1111 = 4000uS 
///						(xxxx<<NRF_ARD) where xxxx is the 4-bit value
///						bits 3:0 are the Auto Retransmit Count.
///						0000 = disabled, 0001 = Up to 1 .... 1111 = Up to 15 Re-Transmit on fail of AA
///						(xxxx<<NRF_ARC) where xxxx is the 4-bit value
void NRF::setupRetransmit(uint8_t val){
	write(NRF_SETUP_RETR, val);
}

//////////////////////////////////////////////////////////////////////////
//RF_CH register functions
//////////////////////////////////////////////////////////////////////////

///@description	Writes to the RF_CH register.
///				1 MHz steps.  Freq = 2400 + RF_CH [MHz].
///@brief		Sets the frequency channel NRF operates on.
///@param		ch	Channel.  Must be between 0 and 127(0x7F).
void NRF::setChannel(uint8_t ch){	
	write(NRF_RF_CH, ch & 0x7F);
}

///@description	Reads from the RF_CH register.
///				1 MHz steps.  Freq = 2400 + RF_CH [MHz].
///@brief		Gets the frequency channel NRF operates on.
///@returns		Channel.  Must be between 0 and 127(0x7F).
uint8_t NRF::getChannel(){
	return (read(NRF_RF_CH));	
}

//////////////////////////////////////////////////////////////////////////
//RF_SETUP register function
//////////////////////////////////////////////////////////////////////////

///@description
///@brief			Setup Data rate & output power.
///@param	val		Valid values: NRF_RF_DR_1MHZ, NRF_RF_DR_2MHZ,
///						NRF_RF_PWR_18DB, NRF_RF_PWR_12DB,
///						NRF_RF_PWR_6DB, NRF_RF_PWR_0DB,
///						NRF_LNA_HCURR
void	NRF::setupRF(uint8_t val){
	write(NRF_RF_SETUP, val & 0xAE);
}

///@description
///@brief			Setup Data rate & output power.
///@returns		Value of RF_SETUP register
uint8_t	NRF::getRFsetup(){
	return read(NRF_RF_SETUP);
}

//////////////////////////////////////////////////////////////////////////
//STATUS register functions
//////////////////////////////////////////////////////////////////////////

///@description	Sends a NOP command to NRF, receiving STATUS back, which is
///					written to the public variable status.
///@brief	Updates status of NRF.
void NRF::updateStatus(){
	clearCSN();								// Pull down chip select
	status = spiTransfer1byte(NRF_NOP);	// Read status register
	setCSN();								// Pull up chip select
}

///@brief Checks if data is available for reading
///@returns		0 if there is no data in the RX FIFO
///				1 if there is data in the RX FIFO to be read
uint8_t NRF::dataReady(){
	updateStatus();
	return ((status & (1<<NRF_RX_DR))  );
}
///@description
///@brief		Clears specified interrupt.
///@param		interrupt	Valid values are: NRF_RX_DR, NRF_TX_DS,
///								NRF_MAX_RT
void NRF::clearInterrupt(uint8_t interrupt){
	write(NRF_STATUS, 1<<interrupt);
}

///@description
///@brief		Returns the number of the datapipe for the RX payload.
///@returns		0-5: 	Datapipe number of RX payload, 
///				7:		RX FIFO empty.
uint8_t NRF::getDatapipeOfRXPayload(){
	updateStatus();
	return((status & 0x0E) >> 1);
}

///@description	Reads the TX_FIFO flag from the STATUS register.
///@brief		Returns status of TX FIFO.	
///@returns		1 if TX FIFO is full.
uint8_t NRF::isTXFIFOFull(){
	updateStatus();
	return((status & (1<<NRF_TX_FIFO_FULL))>>NRF_TX_FIFO_FULL);
}
//////////////////////////////////////////////////////////////////////////
//OBSERVE_TX register functions
//////////////////////////////////////////////////////////////////////////

///@description	Resets when channel is set.  Maximum value of 15.
///@brief		Returns number of lost packets.
///@returns		Number of lost packets.
uint8_t NRF::getLostPacketCount(){
	return ((read(NRF_OBSERVE_TX)&0xF0)>>4);
}

///@description	Resets when transmission of a new packet starts.
///@brief		Returns number of retransmitted packets.
///@returns		Number of retransmitted packets.
uint8_t NRF::getRetransmitPacketCount(){
	return (read(NRF_OBSERVE_TX)&0x0F);
}

//////////////////////////////////////////////////////////////////////////
//CD register function
//////////////////////////////////////////////////////////////////////////

///@description Replaces Carrier Detect
///@brief		Check for carrier.
///@returns		1 if carrier detected.
uint8_t NRF::receivedPowerDetected(){
	return (read(NRF_RPD));
}

//////////////////////////////////////////////////////////////////////////
//Datapipe address functions
//////////////////////////////////////////////////////////////////////////

///@description Sets the receive address of data pipe 0.
///@param	address	The 5-byte address to assign to data pipe 0.
void NRF::setRXAddress(uint8_t * address){	
	//clearCE();
	write(NRF_RX_ADDR_P0, address, 5);
	//setCE();
}

///@description Sets the receive address of any data pipe.
///@param	dataPipe	Value 0 through 5 representing the index of the datapipe to be addressed.
///@param	address	The 5-byte address to assign to data pipe 0.
void NRF::setRXAddress(uint8_t dataPipe, uint8_t * address){
	//clearCE();
	write(getDataPipeAddress(dataPipe), address, 5);
	//setCE();
}

///@description Used for a PTX (primary transmit) device only.
///					Set the receive address of data pipe 0 to the same address to handle
///					automatic acknowledge if this is a PTX device with
///					Enhanced Shockburst enabled. (see NRF datasheet)
///@brief	Sets the transmit address.
///@param	address	The 5-byte address to assign to data pipe 0.
void NRF::setTXAddress(uint8_t * address){
	//clearCE();
	write(NRF_TX_ADDR, address, 5);
	//setCE();
}


///@description		1-32 bytes long.
///@brief			Sets the length of the payload to be in data pipe 0.
void NRF::setPayloadLength(uint8_t length){
	payloadLength = length & 0x3F;			//bits 7:6 must be 0
	write(NRF_RX_PW_P0, payloadLength);
}

///@description		1-32 bytes long.
///@brief			Sets the length of the payload to be in specified data pipe.
///@param dataPipe	Index of data pipe (0-5).
///@param length	Length of specified data pipe (1-32).
void NRF::setPayloadLength(uint8_t dataPipe, uint8_t length){
	payloadLength = length & 0x3F;			//bits 7:6 must be 0
	write(getDataPipePayloadWidthAddress(dataPipe), payloadLength);
}

//////////////////////////////////////////////////////////////////////////
//FIFO_STATUS register functions
//////////////////////////////////////////////////////////////////////////

///@description
///@brief		Returns full status of TX_FIFO buffer.
///@returns		1 if TX FIFO buffer is full, 0 if there are available locations.
uint8_t NRF::isTXFull(){
	return ((read(NRF_FIFO_STATUS) & (1<<NRF_TX_FULL))>>NRF_TX_FULL);	
}

///@description
///@brief		Returns empty status of tX_FIFO buffer.
///@returns		1 if TX FIFO buffer is empty, 0 if data in TX FIFO buffer.
uint8_t NRF::isTXEmpty(){
	return ((read(NRF_FIFO_STATUS) & (1<<NRF_TX_EMPTY))>>NRF_TX_EMPTY);
}

///@description
///@brief		Returns full status of RX_FIFO buffer.
///@returns		1 if RX FIFO buffer is full, 0 if there are available locations.
uint8_t NRF::isRXFull(){
	return ((read(NRF_FIFO_STATUS) & (1<<NRF_RX_FULL))>>NRF_RX_FULL);
}

///@description
///@brief		Returns empty status of RX FIFO buffer.
///@returns		1 if RX FIFO buffer is empty, 0 if data in RX FIFO buffer.
uint8_t NRF::isRXEmpty(){
	return ((read(NRF_FIFO_STATUS) && (1<<NRF_RX_EMPTY))>>NRF_RX_EMPTY);
}

//////////////////////////////////////////////////////////////////////////
///DYNPD register function
//////////////////////////////////////////////////////////////////////////

///@description
///@brief	Enables dynamic payload length on specified datapipes.
///@param	datapipes	Valid bit values: 1<<NRF_DPL_P0, 1<<NRF_DPL_P1, etc.
void	NRF::enableDynamicPayloadLength(uint8_t datapipes){
	write(NRF_DYNPD, datapipes & 0x3F);
}

//////////////////////////////////////////////////////////////////////////
//FEATURE register functions
//////////////////////////////////////////////////////////////////////////

///@description
///@brief		enables dynamic payload length
void NRF::enableDPL(){
	write(NRF_FEATURE, (read(NRF_FEATURE) | (1<<NRF_EN_DPL)));
}
///@description	If ACK packet payload is activated, ACK packets have dynamic payload lengths, and 
///				the Dynamic Payload	Length feature should be enabled for pipe 0 on the PTX and PRX.
///				This is to ensure that they receive the ACK packets with payloads. 
///				If the payload in ACK is more than 15 byte in 2Mbps mode, the ARD must be 500μS or more.
///				If the payload is more than 5 byte in 1Mbps mode the ARD must be 500μS or more.
///@brief		Enables payload with ACK.
void NRF::enableACKPayload(){
	write(NRF_FEATURE, (read(NRF_FEATURE) | (1<<NRF_EN_ACK_PAY)));
}

///@description
///@brief		Enables the NRF_W_TX_PAYLOAD_NACK command
void NRF::enableDynamicACK(){
	write(NRF_FEATURE, (read(NRF_FEATURE) | (1<<NRF_EN_DYN_ACK)));
}

///@description		Disables: Dynamic Payload Length, Payload with ACK and the W_TX_PAYLOAD_NACK command.
///@brief			Clears feature register.
void NRF::clearFeature(){
	write(NRF_FEATURE,0x00);
}

//////////////////////////////////////////////////////////////////////////
//Payload functions
//////////////////////////////////////////////////////////////////////////

///@description	Read RX-Payload, length is determined by function setPayloadLength().
///				Starts with byte 0.  Payload is deleted by NRF after it is read.
///				Data Ready (RX_DR) bit of STATUS register is reset.  Used in RX mode.
///@param		datain		Pointer to be filled with payload. 
void NRF::getPayload(uint8_t * datain){
	 read(NRF_R_RX_PAYLOAD, datain, payloadLength);
	 write(NRF_STATUS,(1<<NRF_RX_DR));	// Reset status register	
}

///@description	Sets mode to TX, flushes TX FIFO, writes payload to TX FIFO, 
///				begins transmission. 1–32 bytes.
///				Always starts at byte 0 used in TX payload.
///				Adds to TX FIFO if not empty.
///@brief		Sends out a multiple-byte payload.
///@param		dataout		Pointer to data to be sent.
///@param		length		Number of bytes in dataout.
void NRF::sendPayload(uint8_t * dataout, uint8_t length){
	
//is this necessary?											
	//clearCE();								// Put nRF into standby-I mode

		powerUp();
		setMode(NRF_MODE_TX);			// Set to transmitter mode
		//flushTX();							// Flush TX FIFO
		write(NRF_W_TX_PAYLOAD, 
				dataout, length & 0x1F);	//write payload
	setCE();								// Start transmission
	_delay_us(20);
	clearCE();
}




//////////////////////////////////////////////////////////////////////////
//Interrupt
//////////////////////////////////////////////////////////////////////////

//Should this whole function be in the main.cpp file?
ISR(INT0_vect){
	extern uint8_t payload[];
	uint8_t config;
	
	asm volatile ("cli"::);		//deactivate interrupts
	PORTB |= (1<<PB7);		//Turn on LED				//testing
	//_delay_ms(1000);									//testing
	
	//check for:
	//		TX_DS	Data Sent TX FIFO interrupt. Asserted when
	//				packet transmitted on TX. If AUTO_ACK is activated,
	//				this bit is set high only when ACK is received.
	//				Write 1 to clear bit.
	//		RX_DR	Data Ready RX FIFO interrupt. Asserted when
	//				new data arrives RX FIFO.
	//				Write 1 to clear bit.
	//		MAX_RT	Maximum number of TX retransmits interrupt
	//				Write 1 to clear bit. If MAX_RT is asserted it must
	//				be cleared to enable further communication.
	
	config = transmitter.getConfig();	//this will also update status
	
	//Data Sent
	//TX_DS is not disabled			and			TX_DS caused the Interrupt
	if (!(config & (1<<NRF_MASK_TX_DS)) && (transmitter.status &  (1<< NRF_TX_DS )) ) {
		
		// The payload should be removed from the TX FIFO automagically
		
		//put into standby-I mode if TX FIFO is empty
		if (transmitter.isTXEmpty())
		clearCE();
		
		//clear interrupt
		transmitter.clearInterrupt(NRF_TX_DS);
	}
	
	//Data Ready
	//RX_DR is not disabled
	if (!(config & (1<<NRF_MASK_RX_DR)) && (transmitter.status &  (1<< NRF_RX_DR )) )  {
		
		//TODO:
		//Data is ready, read the data
		transmitter.getPayload(payload);
		//Should the RX FIFO be flushed here?
		
		//clear interrupt
		transmitter.clearInterrupt(NRF_RX_DR);
	}
	
	//Maximum Retransmits
	//MAX_RT is not disabled	and		MAX_RT interrupt flag is set
	if (!(config & (1<<NRF_MASK_MAX_RT)) && (transmitter.status &  (1<< NRF_MAX_RT)) )  {
		
		//TODO:
		
		//Max retries has been reached
		
		//clear interrupt
		transmitter.clearInterrupt(NRF_MAX_RT);
	}
	
	//////////////////////////////////////////////////////////////////////////
	//TODO: test - take this out
	//payload[0] = 0x42;
	//////////////////////////////////////////////////////////////////////////
	
	asm volatile ("sei"::);		//activate interrupts
}



