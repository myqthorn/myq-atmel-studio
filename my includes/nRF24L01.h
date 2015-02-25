/*
@file NRF24L01.h
@author Mike Litster and Jacob Litster
@version 1.0

<Interface with NRF24L01+ Single Chip 2.4GHz Transceiver>

Register values are shown in hexadecimal format and should be ORed together.
For example, to set the Air Data Rate to 2Mhz and the RF output power to -6dB, you would write
the following to the RF_SETUP(0x06) register:
(NRF_RF_DR_2MHZ | NRF_RF_PWR_6DB)

Bit locations are in decimal format and should be left shifted and ORed.
For example, to mask the RX_DR interrupt and mask the TX_DS interrupt, you would write
the following to the CONFIG(0x00) register:
	((1<<NRF_MASK_RX_DR) | (1<<NRF_MASK_TX_DS))

to compare the bit value you would left shift and AND the value.
To check if the MASK_RX_DR bit has been set in the CONFIG register, 
after reading the CONFIG register into a variable configRegValue:
	if(configRegValue & (1<<NRF_MASK_RX_DR))
*/

#include <avr/io.h>		//for uint8_t definition

//prototype for INT0 interrupt, assigned as friend to nRF class below
extern "C" void INT0_vect (void) __attribute__ ((signal)) ;

//Command set for the NRF SPI
#define NRF_R_REGISTER			0x00
#define NRF_W_REGISTER			0x20
#define NRF_R_RX_PAYLOAD		0x61
#define NRF_W_TX_PAYLOAD		0xA0
#define NRF_FLUSH_TX			0xE1
#define NRF_FLUSH_RX			0xE2
#define NRF_REUSE_TX_PL		0xE3
//#define NRF_ACTIVATE			0x50		//depreciated - not listed in the NRF+ datasheet
#define NRF_R_RX_PL_WID		0x60
#define NRF_W_ACK_PAYLOAD		0xA8
#define NRF_W_TX_PAYLOAD_NACK	0xB0
#define NRF_NOP				0xFF

//-----------------------------------------------------------------
//Register map table
//-----------------------------------------------------------------
#define NRF_CONFIG		0x00	
// CONFIG bits
#define NRF_MASK_RX_DR		6
#define NRF_MASK_TX_DS		5
#define NRF_MASK_MAX_RT		4
#define NRF_EN_CRC			3
#define NRF_CRCO			2
#define NRF_PWR_UP			1
#define NRF_PRIM_RX			0

#define NRF_CRC_OFF			0b00
#define	NRF_CRC_1BYTE		0b10
#define NRF_CRC_2BYTE		0b11
#define NRF_MODE_RX			0b0
#define NRF_MODE_TX			0b1

#define NRF_EN_AA		0x01
// EN_AA bits
#define NRF_ENAA_P5			5
#define NRF_ENAA_P4			4
#define NRF_ENAA_P3			3
#define NRF_ENAA_P2			2
#define NRF_ENAA_P1			1
#define NRF_ENAA_P0			0

#define NRF_EN_RXADDR	0x02
// EN_RXADDR bits
#define NRF_ERX_P5			5
#define NRF_ERX_P4			4
#define NRF_ERX_P3			3
#define NRF_ERX_P2			2
#define NRF_ERX_P1			1
#define NRF_ERX_P0			0

#define NRF_SETUP_AW	0x03
// SETUP_AW values
#define NRF_AW_3BYTES		0x01
#define NRF_AW_4BYTES		0x02
#define NRF_AW_5BYTES		0x03

#define NRF_SETUP_RETR	0x04
// SETUP_RETR bits
//	bits 7:4 are the auto retransmit delay.  0000 = 250uS, 0001 = 500uS .... 1111 = 4000uS
//			Delay defined from end of transmission to start of next transmission
//			(xxxx<<NRF_ARD) where xxxx is the 4-bit value
#define NRF_ARD			4
//  bits 3:0 are Auto Retransmit Count
//			0000 = disabled, 0001 = Up to 1 Re-Transmit on fail of AA .... 1111 Up to 15 Re-Transmit on fail of AA
//			(xxxx<<NRF_ARC) where xxxx is the 4-bit value
#define NRF_ARC			0

#define NRF_RF_CH		0x05
//bits 6:0 set the frequency channel NRF operates on

#define NRF_RF_SETUP	0x06
//RF_SETUP values
#define NRF_CONT_WAVE		0x80
#define NRF_RF_DR_1MHZ		0x00
#define NRF_RF_DR_2MHZ		0x08
#define NRF_RF_DR_250KHZ	0x20
#define NRF_RF_PWR_18DB		0x00
#define NRF_RF_PWR_12DB		0x02
#define NRF_RF_PWR_6DB		0x04
#define NRF_RF_PWR_0DB		0x06


#define NRF_STATUS		0x07
//STATUS bits
#define NRF_RX_DR			6
#define NRF_TX_DS			5
#define NRF_MAX_RT			4
// bits 3:1 are the data pipe number for the payload available for reading from RX_FIFO (READ ONLY)
#define NRF_TX_FIFO_FULL	0	// (READ ONLY)

#define NRF_OBSERVE_TX	0x08
// bits 7:4 Count lost packets. The counter is overflow protected to 15, and discontinues at max until
//			reset. The counter is reset by writing to RF_CH. (READ ONLY)
// bits 3:0 Count retransmitted packets. The counter is reset when transmission of a new packet starts.
//			(READ ONLY)

#define NRF_RPD			0x09
// Received Power Detected bit
#define NRF_RPD_BIT			0

#define NRF_RX_ADDR_P0	0x0A
//Receive address data pipe 0. 5 Bytes maximum length. (LSByte is written first. Write the number
//			of bytes defined by SETUP_AW)
#define NRF_RX_ADDR_P1	0x0B
//Receive address data pipe 1. 5 Bytes maximum length. (LSByte is written first. Write the number
//			of bytes defined by SETUP_AW)
#define NRF_RX_ADDR_P2	0x0C
//Receive address data pipe 2. Only LSB. MSBytes is equal to RX_ADDR_P1[39:8]
#define NRF_RX_ADDR_P3	0x0D
//Receive address data pipe 3. Only LSB. MSBytes is equal to RX_ADDR_P1[39:8]
#define NRF_RX_ADDR_P4	0x0E
//Receive address data pipe 4. Only LSB. MSBytes is equal to RX_ADDR_P1[39:8]
#define NRF_RX_ADDR_P5	0x0F
//Receive address data pipe 5. Only LSB. MSBytes is equal to RX_ADDR_P1[39:8]

#define NRF_TX_ADDR		0x10
//Transmit address. Used for a PTX device only. (LSByte is written first)
//			Set RX_ADDR_P0 equal to this address to handle automatic acknowledge if this is a PTX
//			device with Enhanced ShockBurst™ enabled.

#define NRF_RX_PW_P0	0x11
//bits 5:0	Number of bytes in RX payload in data pipe 0 (1 to 32 bytes).
#define NRF_RX_PW_P1	0x12
//bits 5:0	Number of bytes in RX payload in data pipe 1 (1 to 32 bytes).
#define NRF_RX_PW_P2	0x13
//bits 5:0	Number of bytes in RX payload in data pipe 2 (1 to 32 bytes).
#define NRF_RX_PW_P3	0x14
//bits 5:0	Number of bytes in RX payload in data pipe 3 (1 to 32 bytes).
#define NRF_RX_PW_P4	0x15
//bits 5:0	Number of bytes in RX payload in data pipe 4 (1 to 32 bytes).
#define NRF_RX_PW_P5	0x16
//bits 5:0	Number of bytes in RX payload in data pipe 5 (1 to 32 bytes).

#define NRF_FIFO_STATUS 0x17
//FIFO_STATUS bits
#define NRF_TX_REUSE		6
#define NRF_TX_FULL			5
#define NRF_TX_EMPTY		4
#define NRF_RX_FULL			1
#define NRF_RX_EMPTY		0

#define NRF_DYNPD		0x1C
//DYNPD bits
#define NRF_DPL_P5			5
#define NRF_DPL_P4			4
#define NRF_DPL_P3			3
#define NRF_DPL_P2			2
#define NRF_DPL_P1			1
#define NRF_DPL_P0			0

#define NRF_FEATURE		0x1D
//FEATURE bits
#define NRF_EN_DPL			2
#define NRF_EN_ACK_PAY		1
#define NRF_EN_DYN_ACK		0


//--------------------------------------
//these can be redefined in Hardware.h
#ifndef NRF_CE_PIN
	#define NRF_CE_PIN		PB0
	#define NRF_CE_PORT		PORTB
	#define NRF_CE_DDR		DDRB
#endif //CE

#ifndef NRF_CSN_PIN
	#define NRF_CSN_PIN		PB1
	#define NRF_CSN_PORT	PORTB
	#define NRF_CSN_DDR		DDRB
#endif //CE
//----------------------------------------




class NRF {
	
	public:		
		NRF();			
		NRF(uint8_t ch, uint8_t pl);					//
		void	init();									//
		
		void	flushTX();								//
		void	flushRX();	
		void	maskIRQ(uint8_t irq);					//
		void	powerUp();								//
		void	powerDown();							//
		uint8_t isPoweredUp();							//
		void	setCRC(uint8_t crc);					//
		void	enableAutoAcknowledge(uint8_t datapipes);//
		uint8_t getENAA();								//
		void	enableRXDatapipes(uint8_t datapipes);	//
		uint8_t getENRXADDR();							//
		void	setAddressWidth(uint8_t addressWidth);	//
		void	setupRetransmit(uint8_t val);			//
		void	setChannel(uint8_t ch);					//
		uint8_t getChannel();							//
		void	setupRF(uint8_t val);					//
		uint8_t	getRFsetup();							//
		void	updateStatus();							//
		uint8_t dataReady();								//test later, when transmitting
		void	clearInterrupt(uint8_t interrupt);			//test later, when transmitting
		uint8_t getDatapipeOfRXPayload();					//test later, when transmitting
		uint8_t isTXFIFOFull();							//
		uint8_t getLostPacketCount();						//test later, when transmitting
		uint8_t getRetransmitPacketCount();					//test later, when transmitting
		uint8_t receivedPowerDetected();				//
		void	setRXAddress(uint8_t * address);		//
		void	setRXAddress(uint8_t dataPipe, uint8_t * address);	//
		void	setTXAddress(uint8_t * address);		//
		void	setPayloadLength(uint8_t length);		//
		void	setPayloadLength(uint8_t dataPipe, uint8_t length);//
		uint8_t isTXFull();								//
		uint8_t isTXEmpty();							//
		uint8_t isRXFull();								//
		uint8_t isRXEmpty();							//
		void	enableDynamicPayloadLength(uint8_t datapipes);//
		void	enableDPL();							//
		void	enableACKPayload();						//
		void	enableDynamicACK();						//
		void	clearFeature();							//
		
		void	getPayload(uint8_t * datain);
		void	sendPayload(uint8_t * dataout, uint8_t length);
		
		void	setMode(uint8_t);						//
		uint8_t getMode();								//
		
		uint8_t getConfig(void);						//
		
		friend void INT0_vect();
				
		/// Updated every time a command is sent to the NRF
		uint8_t status;	
			
	//private:
		
		uint8_t payloadLength;
		uint8_t	read(uint8_t reg);						//
		void	read(uint8_t reg, uint8_t * data, uint8_t length);//
		void	write(uint8_t reg);						//
		void	write(uint8_t reg, uint8_t data);		//
		void	write(uint8_t reg, uint8_t * data, uint8_t length);//
		void	setConfig(uint8_t val);					//
		
			
};	//end class NRF
	
	
/*
//////////////////////////////////////////////////////////////////////////
//Enhanced ShockBurst Transmitting Payload
///taken from p72 of NRF+ datasheet
//////////////////////////////////////////////////////////////////////////
	
1. Set the configuration bit PRIM_RX low.
	
2. When the application MCU has data to transmit, 
clock the address for the receiving node(TX_ADDR) and 
payload	data(TX_PLD) into NRF+ through the SPI.
The width of TX-payload is counted from the 
number of bytes written into the TX FIFO from the MCU.
TX_PLD must be written continuously while holding CSN low.
TX_ADDR doesn't have to be rewritten if it is unchanged from the last transmission.
If the PTX device shall receive acknowledge, configure data pipe 0 to receive the ACK packet.
TX_ADDR and RX_ADDR_P0 on the PTX device and RX_ADDR_Px on the PRX device must all be the same.
	
3. Start transmission with a high (pulse) on CE.  Minimum pulse width is 10uS.
	
4. NRF+ ShockBurst:
	Radio is powered up.
	16MHz internal clock is started.
	RF packet is completed (see the packet description).
	Data is transmitted.
		
5. If auto Acknowledgment is activated (ENAA_P0 = 1), the radio goes into RX mode immediately, 
unless the NO_ACK bit is set in the received packet.  If a valid packet is received in the valid
acknowledgment time window, the transmission is considered a success.  The TX_DS bit in the 
STATUS register is set high and the payload is removed from TX FIFO.  
If a valid ACK packet is not received in the specified time window, the payload is retransmitted
(if auto retransmit is enabled).  
If the auto retransmit counter (ARC_CNT) exceeds the programmed maximum limit (ARC),
the MAX_RT bit in the STATUS register is set high.  The payload in TX FIFO is NOT removed.
The IRQ pin is active(low) when MAX_RT or TX_DS is high.
To turn off the IRQ pin, reset the interrupt source by writing to the STATUS register.
If no ACK packet is received for a packet after the maximum number of retransmits,
no further packets can be transmitted before the MAX_RT interrupt is cleared.
The packet loss counter (PLOS_CNT) is incremented at each MAX_RT interrupt.
ARC_CNT counts the number of retransmits required to get a single packet through.
PLOS_CNT counts the number of packets that did not get through after the maximum number of retransmits.
	
6. NRF+ goes into standby-I mode if CE is low.  Otherwise, the next payload in TX FIFO
is transmitted.  If TX FIFO is empty and CE is still high, NRF+ enters standby-II mode.
	
7. If NRF+ is in standby-II mode, it goes to standby-I mode immediately if CE is set low.
	
//////////////////////////////////////////////////////////////////////////
//my description of transmission
//////////////////////////////////////////////////////////////////////////
	
setup
	set TX_ADDR = RX_ADDR_P0 = RX_ADDR_Px(on PRX device)	
	enable Auto ACK	
	make sure TX_DS IRQ is not masked if using interrupts
		
sendPayload:
	clears CE		//not sure this is necessary
	sets PRIM_RX low (transmitter mode)
	sets CE to start transmission		
		
NRF+ ShockBurst: (automagically taken care of)
	Radio is powered up.
	16MHz internal clock is started.
	RF packet is completed (see the packet description).
	Data is transmitted.
	When ACK is received:
		TX_DS bit is set indicating that data has been sent
		IRQ is active if not masked.
		Payload is removed from TX FIFO		
	interrupts:
		TX_DS:
			data transmitted successfully
		MAX_RT:
			Maximum retries reached.  
			Flag set on STATUS register, transmission stops until cleared.
		
interrupt handler
	TX_DS IRQ:
		clear CE to put back into standby-I mode
		clear TX_DS interrupt by writing to STATUS register	
	MAX_RT IRQ:
		*flush TX FIFO if data isn't critical
		clear MAX_RT interrupt by writing to STATUS register
		
//////////////////////////////////////////////////////////////////////////


*/