#ifndef NRF8001_H
#define NRF8001_H

#include "HardwareProfile.h"
//#include "hal_aci_tl.h"

/* HardwareProfile.h needs to define the following:
for example:
#define ACT 		(ACT_PORT & (1<<ACT_PIN))
#define ACT_PIN		PB0
#define ACT_PORT	PORTB
#define ACT_DDR		DDRB

#define REQN		(REQN_PORT & (1<<REQN_PIN))
#define REQN_PIN	1
#define REQN_PORT	PORTB
#define REQN_DDR	DDRB
#define SetREQN()	REQN_PORT |= (1<<REQN_PIN)
#define ClearREQN()	REQN_PORT &= ~(1<<REQN_PIN)

#define RDYN		(RDYN_PORT & (1<<RDYN_PIN))
#define RDYN_PIN	2
#define	RDYN_PORT	PORTB
#define RDYN_DDR	DDRB

*/



class NRF {
	public:
		uint8_t data[21]; // byte0 is the pipe it came from
		uint8_t RxCount;
		uint16_t temperature;
		uint16_t version;
		
		
		//NRF(hal_aci_data_t *);
		NRF();	
		
		
		//System Command Functions
		uint8_t test(uint8_t testFeature);
		uint8_t echo(uint8_t *data, uint8_t length);
		uint8_t DTMcommand(uint16_t command);
		uint8_t sleep();
		uint8_t wakeup();
		uint8_t readDynamicData();
		uint8_t writeDynamicData(uint8_t sequenceNumber, uint8_t *setupData, uint8_t length);
		uint8_t getDeviceVersion();
		uint8_t getDeviceAddress();
		uint8_t getBatteryLevel();
		uint8_t getTemperature();
		uint8_t radioReset();
		uint8_t connect(uint16_t timeout, uint16_t advInterval);
		uint8_t bond(uint16_t timeout, uint16_t advInterval);
		uint8_t disconnect(uint8_t reason);
		uint8_t setTxPower(uint8_t TxPowerLevel);
		uint8_t changeTimingRequest();
		uint8_t changeTimingRequest(uint16_t intervalMin, uint16_t intervalMax, uint16_t slaveLatency, uint16_t timeout);
		uint8_t openRemotePipe(uint8_t servicePipeNumber);
		uint8_t setApplLatency(uint8_t enable, uint16_t latency);
		uint8_t setKey(uint8_t *key);
		uint8_t rejectKey();
		uint8_t openAdvPipe(uint8_t *pipeBitmap);
		uint8_t broadcast(uint16_t timeout, uint16_t advInterval);
		uint8_t bondSecurityRequest();
		uint8_t directedConnect();
		uint8_t closeRemotePipe(uint8_t servicePipeNumber);
		
		//Data Command Functions
		uint8_t setLocalData(uint8_t servicePipeNumber, uint8_t *data, uint8_t length);
		uint8_t sendData(uint8_t servicePipeNumber, uint8_t *data, uint8_t length);
		uint8_t sendDataAck(uint8_t servicePipeNumber);
		uint8_t requestData(uint8_t servicePipeNumber);
		uint8_t sendDataNack(uint8_t servicePipeNumber, uint8_t errorCode);
		
		
		
		//Status Functions
		uint8_t isInitializing();
		uint8_t hasDataToSend();
		uint8_t isReadyToReceiveData();
		uint8_t waitingForResponse();
		uint8_t isConnected();
		uint8_t isBonded();
		uint8_t isSleeping();
		uint8_t hasDataToProcess();
		void	dataHasBeenProcessed();
		
		uint8_t	isSystemCommand(uint8_t command);
		uint8_t isServiceDiscoveryFinished();
		
		void process(void);		
		
	//private:
		uint8_t	status;
		//ACI packet data
		//byte0 is length, byte1 is OPCODE, following bytes are data
		uint8_t RxData[32], TxData[32], pipesOpen[8], pipesClosed[8], key[6];
		
		uint8_t mode; //
		uint8_t dataCredit;
		
		uint8_t lastCommand;
		
			
		uint8_t PrepareTxData(uint8_t command, uint8_t *data, uint8_t length);
		uint8_t PrepareTxData(uint8_t command, uint8_t content, uint8_t *data, uint8_t length);
		uint8_t PrepareTxData(uint8_t command);
		void init();		
		void transferACI();
		void clearTxData();
		void clearRxData();
		void processCommandResponse();	
	};
	
	//System commands	
	#define NRF_CMD_TEST					0x01
	#define NRF_CMD_ECHO					0x02
	#define NRF_CMD_DTM_COMMAND				0x03
	#define NRF_CMD_SLEEP					0x04
	#define NRF_CMD_WAKEUP					0x05
	#define NRF_CMD_SETUP					0x06
	#define NRF_CMD_READ_DYNAMIC_DATA		0x07
	#define NRF_CMD_WRITE_DYNAMIC_DATA		0x08
	#define NRF_CMD_GET_DEVICE_VERSION		0x09
	#define NRF_CMD_GET_DEVICE_ADDRESS		0x0A
	#define NRF_CMD_GET_BATTERY_LEVEL		0x0B
	#define NRF_CMD_GET_TEMPERATURE			0x0C
	#define NRF_CMD_RADIO_RESET				0x0E
	#define NRF_CMD_CONNECT					0x0F
	#define NRF_CMD_BOND					0x10
	#define NRF_CMD_DISCONNECT				0x11
	#define NRF_CMD_SET_TX_POWER			0x12
	#define NRF_CMD_CHANGE_TIMING_REQUEST	0x13
	#define NRF_CMD_OPEN_REMOTE_PIPE		0x14
	#define NRF_CMD_SET_APP_LATENCY			0x19
	#define NRF_CMD_SET_KEY					0x1A
	#define NRF_CMD_OPEN_ADV_PIPE			0x1B
	#define NRF_CMD_BROADCAST				0x1C
	#define NRF_CMD_BOND_SEC_REQUEST		0x1D
	#define NRF_CMD_DIRECT_CONNECT			0x1E
	#define NRF_CMD_CLOSE_REMOTE_PIPE		0x1F
	
	//System Events
	#define NRF_EVT_DEVICE_STARTED			0x81
	#define NRF_EVT_ECHO					0x82
	#define NRF_EVT_HARDWARE_ERROR			0x83
	#define NRF_EVT_COMMAND_RESPONSE		0x84
	#define NRF_EVT_CONNECTED				0x85
	#define NRF_EVT_DISCONNECTED			0x86
	#define NRF_EVT_BOND_STATUS				0x87
	#define NRF_EVT_PIPE_STATUS				0x88
	#define NRF_EVT_TIMING					0x89
	#define NRF_EVT_DISPLAY_KEY				0x8E
	#define NRF_EVT_KEY_REQUEST				0x8F
	
	//Data Commands
	#define NRF_CMD_SET_LOCAL_DATA			0x0D
	#define NRF_CMD_SEND_DATA				0x15
	#define NRF_CMD_SEND_DATA_ACK			0x16
	#define NRF_CMD_REQUEST_DATA			0x17
	#define NRF_CMD_SEND_DATA_NACK			0x18
	
	//Data Events	
	#define NRF_EVT_DATA_CREDIT				0x8A
	#define NRF_EVT_DATA_ACK				0x8B
	#define NRF_EVT_DATA_RECEIVED			0x8C
	#define NRF_EVT_PIPE_ERROR				0x8D
	
	//ACI Status Codes - Page 158 of Datasheet
	
	//status variable - specific to this library
	#define NRF_RX_READY					7
	#define NRF_TX_READY	 				6
	#define NRF_DATA_TO_PROCESS				5
	#define NRF_WAIT_FOR_RESPONSE			4
	#define NRF_CONNECTED					3
	#define NRF_BONDED						2
	#define NRF_SLEEPING					1
	//#define NRF_			0
	
	//modes
	#define NRF_MODE_TEST					0x01
	#define NRF_MODE_SETUP					0x02
	#define NRF_MODE_STANDBY				0x03
	
#endif //NRF8001_H