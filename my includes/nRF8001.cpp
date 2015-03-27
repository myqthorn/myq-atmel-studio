

#include "nRF8001.h"

#include "myqspi.h"

#include <avr/pgmspace.h> // for PROGMEM 
//#include "hal_aci_tl.h"
#define HAL_ACI_MAX_LENGTH 31

#ifdef __GNUC__
#  if __GNUC__ >= 4
#    define _aci_packed_ __attribute__((__packed__))
#  else
#    error "older g++ versions don't handle packed attribute in typedefs"
#  endif
#else
#  define _aci_packed_
#endif


//services.h is a configuration file made with Nordic Semiconductor's nRFgo program
#include "services.h"


//#ifdef PROGMEM
	//#undef PROGMEM
	//#define PROGMEM __attribute__(( section(".progmem.data") ))
//#endif

#define TRUE	1
#define FALSE	0

//Currently working on :---------------------------------------------------------------------------------------------------------
//Event Handling 25 Dec
//Data Command Functions 26 Dec
//TODO: remove all instances of NRF_INIT_MODE - they should be commented out
//TODO: set SPI speed high again
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//this came from ble_heart_rate_template.ino-------------------------------------------------
/*
Store the nRF8001 setup information generated on the flash of the AVR.
This reduces the RAM requirements for the nRF8001.
*/

#ifdef PROGRAMSPACE
typedef struct {
	uint8_t status_byte;
	uint8_t buffer[HAL_ACI_MAX_LENGTH+1];
	} _aci_packed_ PROGMEM hal_aci_data_t;
static hal_aci_data_t const setup_msgs[NB_SETUP_MESSAGES] PROGMEM = SETUP_MESSAGES_CONTENT;
#else
typedef struct {
	uint8_t status_byte;
	uint8_t buffer[HAL_ACI_MAX_LENGTH+1];
	} _aci_packed_ hal_aci_data_t;
static hal_aci_data_t const setup_msgs[NB_SETUP_MESSAGES] = SETUP_MESSAGES_CONTENT;
#endif
//--------------------------------------------------------------------------------------------

#define SPI_SETTING ((1<<SPE) | (0<<SPIE)| (1<<DORD)| (1<<MSTR)| (1<<SPR1)|(1<<SPR0)| (0<<CPOL)| (0<<CPHA))            

	//((1<<SPE)|             // SPI Enable
	//(0<<SPIE)|             // SPI Interrupt Enable
	//(1<<DORD)|             // Data Order (0:MSB first / 1:LSB first)
	//(1<<MSTR)|             // Master/Slave select
	//(0<<SPR1)|(1<<SPR0)|   // SPI Clock Rate
	//(0<<CPOL)|             // Clock Polarity (0:SCK low / 1:SCK hi when idle)
	//(0<<CPHA))             // Clock Phase (0:leading / 1:trailing edge sampling)

SPI spi(SPI_SETTING);


//uint8_t initCodes[12][32];
static uint8_t initCount = 0;

//NRF::NRF(hal_aci_data_t *message){
NRF::NRF(){
	//msg = message;
	
	init();
	RxCount = 0x00;
	temperature = 0x00;
	//mode = 0x00;
	//set to INIT_MODE and RX_READY
	//we are waiting for a NRF_EVT_DEVICE_STARTED in NRF_MODE_SETUP
	//status |= (1<<NRF_RX_READY | 1<<NRF_INIT_MODE | 1<<NRF_WAIT_FOR_RESPONSE);
	status |= (1<<NRF_RX_READY | 1<<NRF_WAIT_FOR_RESPONSE);
}
void NRF::process(void){	
	uint8_t i;
	if (isReadyToReceiveData()){
		clearRxData();
	}
		
	
	//setup message from services.h file loaded onto nRF8001
	if( (mode == NRF_MODE_SETUP)  && !hasDataToSend() && !waitingForResponse() ) {
		//load current line of setup msg into TX buffer
		for(int i = 0; i <= setup_msgs[initCount].buffer[0]; i++){
#ifdef PROGRAMSPACE			
			TxData[i] = pgm_read_byte(&setup_msgs[initCount].buffer[i]);
#else
			TxData[i] = setup_msgs[initCount].buffer[i];
#endif
		}
		
		//set transmit ready and wait for response flags in status register
		status |= ((1<<NRF_TX_READY));
		
		initCount++;
		////after the last setup message
		if (initCount > NB_SETUP_MESSAGES) 
		{
			initCount = 0;
			
			clearTxData();
		}
	}//setup message
	
	
	//handle messages received from nRF8001
	//else //testing**************************************************************************************
	if (!isReadyToReceiveData()){
		RxCount++;
				
		switch (RxData[1]){
			
			//system events:
			case NRF_EVT_DEVICE_STARTED:	//0x81
				//indicates reset recovery or changing operating mode
				//RxData[2]	:	Operating Mode
				//					0x01	Test
				//					0x02	Setup
				//					0x03	Standby
				//RxData[3]	:	HWError 0x01 = error, 0x00 = no error
				//RxData[4]	:	DataCreditAvailable - Number of DataCommand buffers available
				
				if (RxData[2]>0)
					mode = RxData[2];
				
				status &= ~(1<<NRF_SLEEPING);
				
				//when NRF_CMD_WAKEUP has been called, the NRF_WAIT_FOR_RESPONSE bit in the status register should NOT be cleared
				//when finishing a NRF_CMD_SETUP, the NRF_WAIT_FOR_RESPONSE bit in the status register SHOULD be cleared
				if (mode == NRF_MODE_SETUP){
				//if (isInitializing()){
					//clear the NRF_WAIT_FOR_RESPONSE bit in the status register
					status &= ~(1<<NRF_WAIT_FOR_RESPONSE);
					
				}
				
				
				if (mode == NRF_MODE_STANDBY){
					//clear the NRF_INIT_MODE bit in the status register when SETUP finishes
					//status &= ~(1<<NRF_INIT_MODE);
					dataCredit = RxData[4];
					status &= ~(1<<NRF_WAIT_FOR_RESPONSE);
				}
				
				
				
				//set the NRF_RX_READY bit in the status register
				status |= (1<<NRF_RX_READY);
				
				//TODO: error handling
				break;
			case NRF_EVT_ECHO:				//0x82
				//returns an identical copy of the PDU sent using the echo command in Test Mode
				//RxData[0]	:	length
				//RxData[1]	:	NRF_EVT_ECHO (0x82)
				//RxData[2..31]	:	Identical message to one sent with NRF_CMD_ECHO
				data[0] = RxData[0];
				data[1] = RxData[1];
				data[2] = RxData[2];
				data[3] = RxData[3];
				
				//clear the NRF_WAIT_FOR_RESPONSE bit in the status register
				status &= ~(1<<NRF_WAIT_FOR_RESPONSE);
				break;
			case NRF_EVT_HARDWARE_ERROR:	//0x83
				//provides debug information
				//in case of firmware failure, this event follows NRF_EVT_DEVICE_STARTED
				
				//clear the NRF_WAIT_FOR_RESPONSE bit in the status register
				status &= ~(1<<NRF_WAIT_FOR_RESPONSE);
				break;
			case NRF_EVT_COMMAND_RESPONSE:	//0x84
				//confirms reception or execution of an ACI command
				processCommandResponse();				
				break;
			case NRF_EVT_CONNECTED:			//0x85
				//indicates that a connection has been established with a peer device
				//RxData[2]		:	AddressType - peer address type
				//RxData[3-8]	:	6-byte address of peer
				//RxData[9-10]	:	ConnectionInterval (LSB/MSB)
				//RxData[11-12]	:	SlaveLatency (LSB/MSB)
				//RxData[13-14]	:	SupervisionTimeout (LSB/MSB) (x10ms)
				//RxData[15]	:	MasterClockAccuracy (peer device clock accuracy)
				status |= (1<<NRF_CONNECTED);
				
							//do NOT clear the NRF_WAIT_FOR_RESPONSE bit of the status register here
							//there will be either a NRF_EVT_BOND_STATUS or a NRF_EVT_PIPE_STATUS following NRF_EVT_CONNECTED
				//actually it seems to need to be cleared here
				status &= ~(1<<NRF_WAIT_FOR_RESPONSE);
				
				break;
			case NRF_EVT_DISCONNECTED:		//0x86
				//notification of a lost connection or a failure to bond
				//RxData[2]		:	AciStatus: reason for disconnection (local host origin)
				//RxData[3]		:	BtLeStatus: reason for disconnection, BT error code
				//						(origin not related to local Host)
				
				//AciStatus		
				//	0x03		:	Check BtLeStatus
				//	0x93		:	Timeout while advertising, unable to establish connection
				//	0x94		:	Remote device failed to complete a Security Manager procedure
				//	0x8D		:	Bond required to proceed with connection
				
				//BtLeStatus
				//	0x00		:	n/a
				//	0x01..FF	:	BT Core Spec v4.0, Vol2, Pt D, Error Code Description for a complete list of error codes
				
				
				//TODO: can we be still connected if we get here through a failure to bond
				status &= ~(1<<NRF_CONNECTED | 1<<NRF_BONDED);
				
				//clear the NRF_WAIT_FOR_RESPONSE bit in the status register
				status &= ~(1<<NRF_WAIT_FOR_RESPONSE);
				break;
			case NRF_EVT_BOND_STATUS:		//0x87
				//successful bonding procedure.
				//If bonding fails, a NRF_EVT_DISCONNECTED will occur instead of NRF_EVT_BOND_STATUS
				//RxData
				//	2		:	BondStatusCode
				//					0x00 : success
				//					all else	: failed
				//	3		:	BondStatusSource
				//					0x01		: generated locally (nRF8001)
				//					0x02		: generated by the remote peer.
				//	4		:	BondStatus-SecMode1 - LE Security Mode 1
				//	5		:	LE Security Mode 2
				//	6		:	Keys Exchanged (slave)
				//	7		:	Keys Exchanged (master)
				if (RxData[2] == 0x00){ // successful bond
					status |= (1<<NRF_BONDED);
				}
				
				
				//clear the NRF_WAIT_FOR_RESPONSE bit in the status register
				status &= ~(1<<NRF_WAIT_FOR_RESPONSE);
				break;
			case NRF_EVT_PIPE_STATUS:
				//this event is sent when there is a change in service pipe availability
				//RxData[2..9]		: 64-bit PipesOpen bitmap
				//RxData[10..17]	: 64-bit PipesClosed bitmap
				//see pg 148 of datasheet for details
				for (i = 0 ; i<8; i++){
					pipesOpen[i] = RxData[i+2];
					pipesClosed[i] = RxData[i+10];
				}
				//clear the NRF_WAIT_FOR_RESPONSE bit in the status register
				status &= ~(1<<NRF_WAIT_FOR_RESPONSE);
				break;
			case NRF_EVT_TIMING:
				//sent when connects to a peer device 
				//or the connection parameters are updated by a device in the central role
				//RxData[2-3]	: ConnectionInterval (MSB first)
				//RxData[4-5]	: SlaveLatency (LSB/MSB)
				//RxData[6-7]	: SupervisionTimeout (x10ms)
				
				//clear the NRF_WAIT_FOR_RESPONSE bit in the status register
				status &= ~(1<<NRF_WAIT_FOR_RESPONSE);
				break;
			case NRF_EVT_DISPLAY_KEY:
				//RxData[2..7] A fixed 6-byte ASCII string representing the passkey
				//padded with zeros if shorter than 6 digits
				
				//clear the NRF_WAIT_FOR_RESPONSE bit in the status register
				status &= ~(1<<NRF_WAIT_FOR_RESPONSE);
				break;
			case NRF_EVT_KEY_REQUEST:
				//RxData[2] : key requested
				//must send the SetKey command within 30 seconds after receiving this command
				
				//clear the NRF_WAIT_FOR_RESPONSE bit in the status register
				status &= ~(1<<NRF_WAIT_FOR_RESPONSE);
				break;
			//Data Events:
			case NRF_EVT_DATA_CREDIT:		//0x8A
				//RxData[2]	:	number of data credits returned to the application controller
				dataCredit += RxData[2];	
				
				//clear the NRF_WAIT_FOR_RESPONSE bit in the status register
				status &= ~(1<<NRF_WAIT_FOR_RESPONSE);			
				break;
			case NRF_EVT_DATA_ACK:
			
				//do NOT clear the NRF_WAIT_FOR_RESPONSE bit in the status register
				//there will be a NRF_EVT_DATA_CREDIT following the NRF_EVT_DATA_ACK				
				break;
			case NRF_EVT_DATA_RECEIVED:
				//RxData[2]		:	service pipe number
				//RxData[3..23]	:	Data
				
				// Fill data with RxData values
				
				for (i = 0; i< RxData[0] - 1;i++){
					data[i] = RxData[0] + 2;
				}
				
				//set data flag in status
				status |= (1<<NRF_DATA_TO_PROCESS);
								
				//clear the NRF_WAIT_FOR_RESPONSE bit in the status register
				status &= ~(1<<NRF_WAIT_FOR_RESPONSE);
				break;
			case NRF_EVT_PIPE_ERROR:
				
				//clear the NRF_WAIT_FOR_RESPONSE bit in the status register
				status &= ~(1<<NRF_WAIT_FOR_RESPONSE);
				break;
			default:			
				RxCount--;
				//RxData[0] = 0x00;					
				break;
		}//end switch		
		
		//set the NRF_RX_READY bit in the status register
		status |= (1<<NRF_RX_READY);
		
	}	//if (!isReadyToReceiveData())
	
	//once received messages are handled, we are able transmit/receive again
	//if (isReadyToReceiveData()){
	else{
		//Receive Buffer doesn't contain useful data
		
		//if nRF8001 is requesting transfer, receive it
		//if we have data to send, send it
		if ((RDYN == 0) || (hasDataToSend())) {			
			transferACI();
			
		}
	}//else RxData
}//process

void NRF::processCommandResponse(){
	//when a commandResponse event occurs, RxData will contain:
	// byte			contents
	//	0			length
	//	1		0x84 (CommandResponseEvent)
	//	2		CommandOpCode (see switch statement below)
	//	3		Status of command execution
	//	4..31	Command specific data
	
	
	switch (RxData[2]){	//CommandOpCode
		//System commands
		case NRF_CMD_TEST:
			//RxData[3]	:	Status
			//					ACI_STATUS_ERROR_DEVICE_STATE_INVALID when used in incorrect mode
			//						can only be used in NRF_MODE_TEST, NRF_MODE_SETUP, NRF_MODE_STANDBY
		
			//clear the NRF_WAIT_FOR_RESPONSE bit in the status register
			status &= ~(1<<NRF_WAIT_FOR_RESPONSE);
			break;
		case NRF_CMD_ECHO:	//0x02
			//RxData[3]	:	Status
			//					ACI_STATUS_ERROR_DEVICE_STATE_INVALID when used in incorrect mode
			//						can only be used in NRF_MODE_TEST
		
			//clear the NRF_WAIT_FOR_RESPONSE bit in the status register
			status &= ~(1<<NRF_WAIT_FOR_RESPONSE);
			break;
		case NRF_CMD_DTM_COMMAND:
			//RxData[3] 	:	Status
			//					ACI_STATUS_ERROR_DEVICE_STATE_INVALID when used in incorrect mode
			//						can only be used in NRF_MODE_STANDBY
			//RxData[4-5]	:	DTM Event (2 bytes MSB/LSB)
			//Refer to Bluetooth Core specification v4.0, Volume 6, Part F, Direct Test Mode,
			//Sect.3.4, ‘Events’, for description of DTM event format and content
		
			//clear the NRF_WAIT_FOR_RESPONSE bit in the status register
			status &= ~(1<<NRF_WAIT_FOR_RESPONSE);
			break;
		case NRF_CMD_SLEEP:
			//RxData[3]	:	Status
			//					ACI_STATUS_ERROR_DEVICE_STATE_INVALID when used in incorrect mode
			//						can only be used in NRF_MODE_STANDBY
		
			//clear the NRF_WAIT_FOR_RESPONSE bit in the status register
			status &= ~(1<<NRF_WAIT_FOR_RESPONSE);
			break;
		case NRF_CMD_WAKEUP:
			//when woken up, the nRF8001 first returns a NRF_EVT_DEVICE_STARTED,
			//then a NRF_EVT_COMMAND_RESPONSE with a NRF_CMD_WAKEUP, which brings us here
			//RxData[3]		: status - better be success
			//RxData[4]		: none
			
			status &= ~(1<<NRF_SLEEPING);
			
			//clear the NRF_WAIT_FOR_RESPONSE bit in the status register
			status &= ~(1<<NRF_WAIT_FOR_RESPONSE);
			break;
		case NRF_CMD_SETUP:		//pg 102
			//after a NRF_CMD_SETUP is called, after completion of the setup sequence,
			//it returns a NRF_EVT_COMMAND_RESPONSE, followed by a NRF_EVT_DEVICE_STARTED
			//RxData[3]	: Status code
			//		0x01	ACI_STATUS_TRANSACTION_CONTINUE
			//		0x02	ACI_STATUS_TRANSACTION_COMPLETE
			//				ACI_STATUS_ERROR_DEVICE_STATE_INVALID if device is not in the correct
			//					operating mode for a NRF_CMD_SETUP command
			//					can only be called in standby or setup mode
		
			
				
			//on the last run of CMD_SETUP
			//do NOT clear the NRF_WAIT_FOR_RESPONSE bit in the status register
			//the NRF_CMD_SETUP will be followed by NRF_EVT_DEVICE_STARTED			
			if (RxData[3] == ACI_STATUS_TRANSACTION_CONTINUE){
				status &= ~(1<<NRF_WAIT_FOR_RESPONSE);
			}
			
			//set the NRF_RX_READY bit in the status register
			status |= (1<<NRF_RX_READY);
			break;
		case NRF_CMD_READ_DYNAMIC_DATA:
		
			//clear the NRF_WAIT_FOR_RESPONSE bit in the status register
			status &= ~(1<<NRF_WAIT_FOR_RESPONSE);
			break;
		case NRF_CMD_WRITE_DYNAMIC_DATA:
		
			//clear the NRF_WAIT_FOR_RESPONSE bit in the status register
			status &= ~(1<<NRF_WAIT_FOR_RESPONSE);
			break;
		case NRF_CMD_GET_DEVICE_VERSION:	//0x09
			//returns nRF8001 version information
			//RxData[3]		:	Status
			//RxData[4-5]	:	Config ID - (LSB/MSB)
			//RxData[6]		:	ACI Protocol version
			//
		
			//TODO: finish this
			version = RxData[6];
		
			//clear the NRF_WAIT_FOR_RESPONSE bit in the status register
			status &= ~(1<<NRF_WAIT_FOR_RESPONSE);
			break;
		case NRF_CMD_GET_DEVICE_ADDRESS:
		
			//clear the NRF_WAIT_FOR_RESPONSE bit in the status register
			status &= ~(1<<NRF_WAIT_FOR_RESPONSE);
			break;
		case NRF_CMD_GET_BATTERY_LEVEL:
		
			//clear the NRF_WAIT_FOR_RESPONSE bit in the status register
			status &= ~(1<<NRF_WAIT_FOR_RESPONSE);
			break;
		case NRF_CMD_GET_TEMPERATURE:
			// RxData[4] : temperature LSB
			// RxData[5] : temperature MSB
		
			//data[0] = NRF_CMD_GET_TEMPERATURE;
			//data[1] = RxData[4];
			//data[2] = RxData[3];
			if (RxData[3] == ACI_STATUS_SUCCESS)
			temperature = (RxData[5]<<4 | RxData[4]);
			clearRxData();
		
			//clear the NRF_WAIT_FOR_RESPONSE bit in the status register
			status &= ~(1<<NRF_WAIT_FOR_RESPONSE);
		break;
			case NRF_CMD_RADIO_RESET:
			//will initialization have to be redone upon reset?
		
			//clear the NRF_WAIT_FOR_RESPONSE bit in the status register
			status &= ~(1<<NRF_WAIT_FOR_RESPONSE);
			break;
		case NRF_CMD_CONNECT:
		
			//do NOT clear the NRF_WAIT_FOR_RESPONSE bit in the status register
			//the NRF_CMD_CONNECT will be followed by NRF_EVT_PIPE_STATUS upon success and
			//by NRF_EVT_DISCONNECTED upon failure
			//status &= ~(1<<NRF_WAIT_FOR_RESPONSE);
			break;
		case NRF_CMD_BOND:
		
			//do NOT clear the NRF_WAIT_FOR_RESPONSE bit in the status register
			//the NRF_CMD_BOND will be followed by NRF_EVT_CONNECTED
			//status &= ~(1<<NRF_WAIT_FOR_RESPONSE);
			break;
		case NRF_CMD_DISCONNECT:
		
			//do NOT clear the NRF_WAIT_FOR_RESPONSE bit in the status register
			//the NRF_CMD_DISCONNECT will be followed by NRF_EVT_DISCONNECTED
			//status &= ~(1<<NRF_WAIT_FOR_RESPONSE);
			break;
		case NRF_CMD_SET_TX_POWER:
		
			//clear the NRF_WAIT_FOR_RESPONSE bit in the status register
			status &= ~(1<<NRF_WAIT_FOR_RESPONSE);
			break;
		case NRF_CMD_CHANGE_TIMING_REQUEST:
		
			//do NOT clear the NRF_WAIT_FOR_RESPONSE bit in the status register
			//the NRF_CMD_CHANGE_TIMING_REQUEST will be followed by NRF_EVT_TIMING
			break;
		case NRF_CMD_OPEN_REMOTE_PIPE:
		
			//do NOT clear the NRF_WAIT_FOR_RESPONSE bit in the status register
			//the NRF_CMD_OPEN_REMOTE_PIPE will be followed by NRF_EVT_PIPE_STATUS(success)
			//or by NRF_EVT_PIPE_ERROR(failure)
			break;
		case NRF_CMD_SET_APP_LATENCY:
		
			//clear the NRF_WAIT_FOR_RESPONSE bit in the status register
			status &= ~(1<<NRF_WAIT_FOR_RESPONSE);
			break;
		case NRF_CMD_SET_KEY:
		
			//clear the NRF_WAIT_FOR_RESPONSE bit in the status register
			status &= ~(1<<NRF_WAIT_FOR_RESPONSE);
			break;
		case NRF_CMD_OPEN_ADV_PIPE:
		
			//clear the NRF_WAIT_FOR_RESPONSE bit in the status register
			status &= ~(1<<NRF_WAIT_FOR_RESPONSE);
			break;
		case NRF_CMD_BROADCAST:
		
			//clear the NRF_WAIT_FOR_RESPONSE bit in the status register
			status &= ~(1<<NRF_WAIT_FOR_RESPONSE);
			break;
		case NRF_CMD_BOND_SEC_REQUEST:
		
			//clear the NRF_WAIT_FOR_RESPONSE bit in the status register
			status &= ~(1<<NRF_WAIT_FOR_RESPONSE);
			break;
		case NRF_CMD_DIRECT_CONNECT:
		
			//clear the NRF_WAIT_FOR_RESPONSE bit in the status register
			status &= ~(1<<NRF_WAIT_FOR_RESPONSE);
			break;
		case NRF_CMD_CLOSE_REMOTE_PIPE:
		
			//do NOT clear the NRF_WAIT_FOR_RESPONSE bit in the status register
			//the NRF_CMD_CLOSE_REMOTE_PIPE will be followed by NRF_EVT_PIPE_STATUS(success)
			//or by NRF_EVT_PIPE_ERROR(failure)
			break;
			
		//Data Commands
		case NRF_CMD_SET_LOCAL_DATA:
		
			
			break;
		case NRF_CMD_SEND_DATA:
		
			
			break;
		case NRF_CMD_SEND_DATA_ACK:
		
			
			break;
		case NRF_CMD_REQUEST_DATA:
		
			
			break;
		case NRF_CMD_SEND_DATA_NACK:
		
			
			break;
		default:
			break;
		
	}//end switch RxData[2]
	
}//processCommandResponse()

///@description Prepares send buffer TxData and sets NRF_TX_READY flag in status buffer. Data will be sent during process() function.
///@brief Send data from uC to nRF8001
///@param command
///@param data
///@param length Length of data
///@returns	error 0x00: success, 0x01: NRF initializing, 0x02: uC has not sent current data to NRF, 0x03: nRF has not responded from last command yet
uint8_t NRF::PrepareTxData(uint8_t command, uint8_t *data, uint8_t length){
	//TODO: write this function
	int i;
	if (isInitializing())
		return 0x01;
	if (hasDataToSend())
		return 0x02;
	if (waitingForResponse() && isSystemCommand(command))
		return 0x03;
	
	lastCommand = command;
	
	clearTxData();
	
	//populate TxData with length and data
	TxData[0] = length + 1;
	TxData[1] = command;
	for(i = 0; i < length; i++)
		TxData[i+2] = data[i];	
		
	//data to send flag
	status |= (1<<NRF_TX_READY);
	return 0x00;	//success
}//PrepareTxData

uint8_t NRF::PrepareTxData(uint8_t command, uint8_t content, uint8_t *data, uint8_t length){
	
	int i;
	if (isInitializing())
		return 0x01;
	if (hasDataToSend())
		return 0x02;
	if (waitingForResponse() && isSystemCommand(command))
		return 0x03;
	
	lastCommand = command;
	
	clearTxData();
	//populate TxData with length and data
	TxData[0] = length + 2;
	TxData[1] = command;
	TxData[2] = content;
	for(i = 0; i < length; i++)
		TxData[i+3] = data[i];
	
	//data to send flag
	status |= (1<<NRF_TX_READY);
	return 0x00;	//success
}//PrepareTxData

uint8_t NRF::PrepareTxData(uint8_t command){
	if (isInitializing())
		return 0x01;
	if (hasDataToSend())
		return 0x02;
	if (waitingForResponse() && isSystemCommand(command))
		return 0x03;
	
	lastCommand = command;
	
	clearTxData();
	TxData[0] = 0x01;
	TxData[1] = command;
	status |= (1<<NRF_TX_READY);
	return 0x00;
}

#pragma region System Command Functions
uint8_t NRF::test(uint8_t testFeature){	
	return (PrepareTxData(NRF_CMD_TEST, &testFeature, 1));
}//test

uint8_t NRF::echo(uint8_t *data, uint8_t length){
	return (PrepareTxData(NRF_CMD_ECHO, data, length));
}

uint8_t NRF::DTMcommand(uint16_t command){
	uint8_t localData[2];
	localData[0] = (command&0xFF00)>>8; //MSB
	localData[1] = command&0x00FF;		//LSB
	return (PrepareTxData(NRF_CMD_DTM_COMMAND, localData, 2));
}//DTMcommand

uint8_t NRF::sleep(){
	status |= (1<<NRF_SLEEPING);
	return (PrepareTxData(NRF_CMD_SLEEP));
}//sleep

///@returns	error 0x00: success, 0x01: NRF initializing, 0x02: uC has not sent current data to NRF, 0x03: nRF has not responded from last command yet
uint8_t NRF::wakeup(void){	
	return (PrepareTxData(NRF_CMD_WAKEUP));
}

uint8_t NRF::readDynamicData(){
	return (PrepareTxData(NRF_CMD_READ_DYNAMIC_DATA));
}//readDynamicData

uint8_t NRF::writeDynamicData(uint8_t sequenceNumber, uint8_t *setupData, uint8_t length){
	return (PrepareTxData(NRF_CMD_WRITE_DYNAMIC_DATA, sequenceNumber, setupData, length + 1));
}//writeDynamicData

///@returns	error 0x00: success, 0x01: NRF initializing, 0x02: uC has not sent current data to NRF, 0x03: nRF has not responded from last command yet
uint8_t NRF::getDeviceVersion(void){
	return (PrepareTxData(NRF_CMD_GET_DEVICE_VERSION));
}//getDeviceVersion

uint8_t NRF::getDeviceAddress(){
	return (PrepareTxData(NRF_CMD_GET_DEVICE_ADDRESS));
}//getDeviceAddress

uint8_t NRF::getBatteryLevel(){
	return (PrepareTxData(NRF_CMD_GET_BATTERY_LEVEL));
}//getBatteryLevel

///@returns	error 0x00: success, 0x01: NRF initializing, 0x02: uC has not sent current data to NRF, 0x03: nRF has not responded from last command yet
uint8_t NRF::getTemperature(void){	
	return (PrepareTxData(NRF_CMD_GET_TEMPERATURE));
}//getTemperature

uint8_t NRF::radioReset(){
	return (PrepareTxData(NRF_CMD_RADIO_RESET));
}//

uint8_t NRF::connect(uint16_t timeout, uint16_t advInterval){	
	//uint8_t localData[4];	
	//localData[0] = (timeout&0x00FF);	//LSB
	//localData[1] = (timeout&0xFF00)>>8; //MSB
	//localData[2] = (advInterval&0x00FF);	//LSB
	//localData[3] = (advInterval&0xFF00)>>8; //MSB
	//return (PrepareTxData(NRF_CMD_CONNECT, localData, 4));
	
	if (isInitializing())
	return 0x01;
	if (hasDataToSend())
	return 0x02;
	if (waitingForResponse())
	return 0x03;
	
	lastCommand = TxData[1];
	
	clearTxData();
	TxData[0] = 0x05;	//length
	TxData[1] = NRF_CMD_CONNECT;
	TxData[2] = (timeout&0x00FF);	//LSB
	TxData[3] = (timeout&0xFF00)>>8; //MSB
	TxData[4] = (advInterval&0x00FF);	//LSB
	TxData[5] = (advInterval&0xFF00)>>8; //MSB
	status |= (1<<NRF_TX_READY);
	return 0x00;
}//

uint8_t NRF::bond(uint16_t timeout, uint16_t advInterval){	
	uint8_t localData[4];	
	localData[0] = (timeout&0x00FF);	//LSB
	localData[1] = (timeout&0xFF00)>>8; //MSB
	localData[2] = (advInterval&0x00FF);	//LSB
	localData[3] = (advInterval&0xFF00)>>8; //MSB
	return (PrepareTxData(NRF_CMD_BOND, localData, 4));
}//

uint8_t NRF::disconnect(uint8_t reason){
	return (PrepareTxData(NRF_CMD_DISCONNECT, &reason, 1));
}//

uint8_t NRF::setTxPower(uint8_t TxPowerLevel){
	return (PrepareTxData(NRF_CMD_SET_TX_POWER, &TxPowerLevel, 1));
}//

uint8_t NRF::changeTimingRequest(){
return (PrepareTxData(NRF_CMD_CHANGE_TIMING_REQUEST));
}//

uint8_t NRF::changeTimingRequest(uint16_t intervalMin, uint16_t intervalMax, uint16_t slaveLatency, uint16_t timeout){
	uint8_t localData[8];	
	localData[0] = (intervalMin&0x00FF);		//LSB
	localData[1] = (intervalMin&0xFF00)>>8; 	//MSB
	localData[2] = (intervalMax&0x00FF);		//LSB
	localData[3] = (intervalMax&0xFF00)>>8; 	//MSB
	localData[4] = (slaveLatency&0x00FF);		//LSB
	localData[5] = (slaveLatency&0xFF00)>>8;	//MSB
	localData[6] = (timeout&0x00FF);			//LSB
	localData[7] = (timeout&0xFF00)>>8; 		//MSB
	return (PrepareTxData(NRF_CMD_CHANGE_TIMING_REQUEST, localData, 8));
}//

uint8_t NRF::openRemotePipe(uint8_t servicePipeNumber){
	return (PrepareTxData(NRF_CMD_OPEN_REMOTE_PIPE, &servicePipeNumber, 1));
}//

uint8_t NRF::setApplLatency(uint8_t enable, uint16_t latency){
	uint8_t localData[3];	
	localData[0] = enable;
	localData[1] = (latency & 0x00FF);		//LSB
	localData[2] = (latency & 0xFF00)>>8; 	//MSB
	return (PrepareTxData(NRF_CMD_SET_APP_LATENCY, localData, 3));
}//

uint8_t NRF::setKey(uint8_t *key){	
	return (PrepareTxData(NRF_CMD_SET_KEY, 0x01, key, 6));
}//

uint8_t NRF::rejectKey(){
	uint8_t localData = 0x00;		//reject key request
	return (PrepareTxData(NRF_CMD_SET_KEY, &localData, 1));
}//

uint8_t NRF::openAdvPipe(uint8_t *pipeBitmap){
return (PrepareTxData(NRF_CMD_OPEN_ADV_PIPE, pipeBitmap, 8));
}//

uint8_t NRF::broadcast(uint16_t timeout, uint16_t advInterval){
	uint8_t localData[4];	
	localData[0] = (timeout&0x00FF);		//LSB
	localData[1] = (timeout&0xFF00)>>8;		//MSB
	localData[2] = (advInterval&0x00FF);	//LSB
	localData[3] = (advInterval&0xFF00)>>8; //MSB
	return (PrepareTxData(NRF_CMD_BROADCAST, localData, 4));
}//

uint8_t NRF::bondSecurityRequest(){
return (PrepareTxData(NRF_CMD_BOND_SEC_REQUEST));
}//

uint8_t NRF::directedConnect(){
return (PrepareTxData(NRF_CMD_DIRECT_CONNECT));
}//

uint8_t NRF::closeRemotePipe(uint8_t servicePipeNumber){
return (PrepareTxData(NRF_CMD_CLOSE_REMOTE_PIPE, &servicePipeNumber, 1));
}//
#pragma endregion System Command Functions

#pragma region Data Command Functions
uint8_t NRF::setLocalData(uint8_t servicePipeNumber, uint8_t *data, uint8_t length){	
	return (PrepareTxData(NRF_CMD_SET_LOCAL_DATA, servicePipeNumber, data, length));
}//

uint8_t NRF::sendData(uint8_t servicePipeNumber, uint8_t *data, uint8_t length){
	uint8_t retVal = PrepareTxData(NRF_CMD_SET_LOCAL_DATA, servicePipeNumber, data, length);
	if (retVal == 0x00)
		dataCredit-=1;
	return (retVal);
}//

uint8_t NRF::sendDataAck(uint8_t servicePipeNumber){
	uint8_t retVal = PrepareTxData(NRF_CMD_SEND_DATA_ACK, &servicePipeNumber, 1);
	if (retVal == 0x00)
		dataCredit-=1;
	return (retVal);
}//

uint8_t NRF::requestData(uint8_t servicePipeNumber){
	uint8_t retVal = PrepareTxData(NRF_CMD_REQUEST_DATA, &servicePipeNumber, 1);
	if (retVal == 0x00)
		dataCredit-=1;
	return (retVal);
}//

uint8_t NRF::sendDataNack(uint8_t servicePipeNumber, uint8_t errorCode){
	uint8_t retVal = PrepareTxData(NRF_CMD_REQUEST_DATA, servicePipeNumber, &errorCode, 1);
	if (retVal == 0x00)
		dataCredit-=1;
	return (retVal);
}//

		

#pragma endregion Data Command Functions

#pragma region Status Functions
uint8_t NRF::isInitializing(void){
	//return ((status & (1<<NRF_INIT_MODE))?TRUE:FALSE);
	return (mode == NRF_MODE_SETUP);
}
uint8_t NRF::hasDataToSend(void){
	return ((status & (1<<NRF_TX_READY))?TRUE:FALSE);
}
uint8_t NRF::isReadyToReceiveData(){
	return ((status & (1<<NRF_RX_READY))?TRUE:FALSE);
}
uint8_t NRF::waitingForResponse(void){
	return ((status & (1<<NRF_WAIT_FOR_RESPONSE))?TRUE:FALSE);
	}
uint8_t NRF::isConnected(void){
	return ((status & (1<<NRF_CONNECTED))?TRUE:FALSE);
}	
uint8_t NRF::isBonded(void){
	return ((status & (1<<NRF_BONDED))?TRUE:FALSE);	
}
uint8_t NRF::hasDataToProcess(void){
	return ((status & (1<<NRF_DATA_TO_PROCESS))?TRUE:FALSE);
}
void	NRF::dataHasBeenProcessed(void){
	status &= ~(1<<NRF_DATA_TO_PROCESS);
}
uint8_t NRF::isSleeping(void){
	return ((status & (1<<NRF_SLEEPING))?TRUE:FALSE);
}
#pragma endregion Status Functions

uint8_t	NRF::isSystemCommand(uint8_t command){
	return ((	command == NRF_CMD_SET_LOCAL_DATA	||
		command == NRF_CMD_SEND_DATA		||
		command == NRF_CMD_SEND_DATA_ACK	||
		command == NRF_CMD_REQUEST_DATA		||
		command == NRF_CMD_SEND_DATA_NACK	 )	? FALSE: TRUE);
}
uint8_t NRF::isServiceDiscoveryFinished(void){
	//if bit0 in pipesOpen[0] is set, Service Discovery is complete.
	return((pipesOpen[0] & 0x01)? TRUE: FALSE);
	
}


void NRF::init(){
	//ACT and RDYN set as inputs	
	ACT_DDR &= ~(1<<ACT_PIN);	
	RDYN_DDR &= ~(1<<RDYN_PIN);
	
	//REQN set as output
	REQN_DDR |= (1<<REQN_PIN);
	
	
	//////DON'T KNOW WHY THIS IS CAUSING A PROBLEM********************************************************
	//set pullup on RDYN
	//RDYN_PORT |= (1<<RDYN_PIN);
	
	//REQN is active low, so initialize it HIGH
	SetREQN();
}
/*
The following procedure is performed when the application controller sends a command to nRF8001:
1. The application controller requests the right to send data by setting the REQN pin to ground.
2. nRF8001 sets the RDYN pin to ground when it is ready to receive data.
3. The application controller starts sending data on the MOSI pin:
• Byte 1 (length byte) from the application controller defines the length of the message.
• Byte 2 (ACI byte1) is the first byte of the ACI data.
• Byte N is the last byte of the ACI data.
• The application controller sets the REQN pin high to terminate the data transaction.
Note: The maximum length of a command packet is 32 bytes, including the length byte. MOSI shall
be held low if the application controller receives an event and has no message to send to the nRF8001.
*/
/*



The application controller receives the ACI event by performing the following procedure:
1. nRF8001 sets the RDYN pin to ground.
2. The application controller sets the REQN pin to ground and starts clocking on the SCK pin.
• Byte 1 (debug byte) from nRF8001 is an internal debug byte and the application
controller discards it.
• Byte 2 (length byte) from nRF8001 defines the length of the message.
• Byte 3 (ACI byte1) is the first byte of the ACI data.
• Byte N is the last byte of the ACI data.
3. The application controller sets the REQN pin high to close the event.
Note: The maximum length of an event packet is 31 bytes, including the length byte.


nRF8001 is capable of receiving an ACI command simultaneously as it sends an ACI event to the
application controller.
The application controller shall always read the length byte from nRF8001 and check if the length is
greater than 0. If the length is greater than 0 the data on the MISO line shall be read
*/
void NRF::transferACI(void){
	uint8_t length; 
	
	//transmit and receive
	if (hasDataToSend() && (TxData[0] > 0)){			
		length = TxData[0];
	
		//Request communication with nRF8001
		ClearREQN();
	
		//Wait for nRF8001 to clear RDYN pin indicating it is ready to receive
		while (RDYN == 1);
	
		//transmit the Tx LENGTH BYTE and receive the Rx DEBUG BYTE
		//the LENGTH BYTE is not counted as part of the length
		RxData[0] = spi.transfer1byte(TxData[0]);
	
	
		//transmit the first byte of data while receiving the length byte
		//we are discarding the Rx DEBUG BYTE
		RxData[0] = spi.transfer1byte(TxData[1]);
	
		////set length to the greater of TxData less the byte we already sent or 
		////RxData length
		length = (RxData[0]>(TxData[0] -1))?RxData[0]:(TxData[0]-1);
		if (length>HAL_ACI_MAX_LENGTH) 
			length = HAL_ACI_MAX_LENGTH;
			
		//if (length>0)
		spi.transfer(&TxData[2],&RxData[1],length);
		//for (uint8_t i = 0; i< length; i++){
			//RxData[i+1] = spi.transfer1byte(TxData[i+2]); 
		//}
	
		//Set REQN high to terminate the transfer
		SetREQN();
		
		//when data is received, clear the ready to receive flag until
		//the received data is handled by the process function
		if (RxData[0] != 0x00){
			status &= ~(1<<NRF_RX_READY);
		}
		
		
		lastCommand = TxData[1];
		
		//clear the NRF_TX_READY flag in the status register
		status &= ~(1<<NRF_TX_READY);
		
		if (isSystemCommand(TxData[1]))
			status |= (1<<NRF_WAIT_FOR_RESPONSE);
				
	}//if hasDataToSend
	
	//receive only
	else if(RDYN == 0)
	{
		clearTxData();
		//Request communication with nRF8001
		ClearREQN();
		
		//transmit the DUMMY BYTE and receive the Rx DEBUG BYTE
		//the LENGTH BYTE is not counted as part of the length
		RxData[0] = spi.transfer1byte(0x00);
		
		//Transmit a DUMMY BYTE and receive the LENGTH BYTE
		RxData[0] = spi.transfer1byte(0x00);
		
		length = RxData[0];
		if (length>HAL_ACI_MAX_LENGTH)
			length = HAL_ACI_MAX_LENGTH;
		if (length>0)
		{
			
			spi.transfer(&TxData[2],&RxData[1],length);
		}//if length>0
		
		//Set REQN high to terminate the transfer
		SetREQN();
		
		//when data is received, clear the ready to receive flag until
		//the received data is handled by the process function
		if (RxData[0] != 0x00){
			status &= ~(1<<NRF_RX_READY);
		}
		
	}//else if(RDYN == 0)	
	
}//transferACI()
void NRF::clearTxData(){
	for(int i = 0; i<32; i++) 
		TxData[i] = 0x00;		
}
void NRF::clearRxData(){
	for(int i = 0; i<32; i++)
		RxData[i] = 0x00;
}
