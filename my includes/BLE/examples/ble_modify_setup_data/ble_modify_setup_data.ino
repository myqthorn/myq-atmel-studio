/* Copyright (c) 2014, Nordic Semiconductor ASA
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */
 

/** @defgroup ble_uart_project_template ble_uart_project_template
@{
@ingroup projects
@brief Empty project that can be used as a template for new projects.

@details
This project is a firmware template for new projects. 
The project will run correctly in its current state.
It can send data on the UART TX characteristic
It can receive data on the UART RX characterisitc.
With this project you have a starting point for adding your own application functionality.

The following instructions describe the steps to be made on the Windows PC:

 -# Install the Master Control Panel on your computer. Connect the Master Emulator 
    (nRF2739) and make sure the hardware drivers are installed.

-# You can use the nRF UART app in the Apple iOS app store with this UART template app

 *
 * Click on the "Serial Monitor" button on the Arduino IDE to reset the Arduino and start the application.
 * The setup() function is called first and is called only once for each reset of the Arduino.
 * The loop() function as the name implies is called in a loop.
 *
 * The setup() and loop() function are called in this way.
 * main() 
 *  {
 *   setup(); 
 *   while(1)
 *   {
 *     loop();
 *   }
 * }
 *    
 */
#include <SPI.h>
#include <avr/pgmspace.h>
#include <lib_aci.h>
#include <aci_setup.h>

/**
Put the nRF8001 setup in the RAM of the nRF8001.

Modify the Setup message with 0x20 -> ATTDB    Offset -> 0x00
Change the Characteristics Properties to Read    0x0e -> 0x02 
Change the Device name value permissions to Read 0x14 -> 0x04
Recompute the CRC in the code and put it back into the header file
*/

#include "services_ota_with_pipe_ota_removed.h"
/**
Include the services_lock.h to put the setup in the OTP memory of the nRF8001
This would mean that the setup cannot be changed once put in
However this removes the need to do the setup of the nRF8001 on every reset
*/


#ifdef SERVICES_PIPE_TYPE_MAPPING_CONTENT
    static services_pipe_type_mapping_t
        services_pipe_type_mapping[NUMBER_OF_PIPES] = SERVICES_PIPE_TYPE_MAPPING_CONTENT;
#else
    #define NUMBER_OF_PIPES 0
    static services_pipe_type_mapping_t * services_pipe_type_mapping = NULL;
#endif

/* Store the setup for the nRF8001 in the flash of the AVR to save on RAM */
static hal_aci_data_t setup_msgs[NB_SETUP_MESSAGES] PROGMEM = SETUP_MESSAGES_CONTENT;

// aci_struct that will contain 
// total initial credits
// current credit
// current state of the aci (setup/standby/active/sleep)
// open remote pipe pending
// close remote pipe pending
// Current pipe available bitmap
// Current pipe closed bitmap
// Current connection interval, slave latency and link supervision timeout
// Current State of the the GATT client (Service Discovery)
// Status of the bond (R) Peer address
static struct aci_state_t aci_state;

/*
Temporary buffers for sending ACI commands
*/
static hal_aci_evt_t  aci_data;


/*
Timing change state variable
*/
static bool timing_change_done = false;

/*
Used to test the UART TX characteristic notification
*/
static uint8_t uart_buffer[20];
static uint8_t uart_buffer_len = 0;
static bool    bonded = false;

/*
Initialize the radio_ack. This is the ack received for every transmitted packet.
*/
//static bool radio_ack_pending = false;

/** crc function to re-calulate the CRC after making changes to the setup data.
*/
uint16_t crc_16_ccitt(uint16_t crc, uint8_t * data_in, uint16_t data_len) {
  
  uint16_t i;

  for(i = 0; i < data_len; i++)
  {
    crc  = (unsigned char)(crc >> 8) | (crc << 8);
    crc ^= pgm_read_byte(&data_in[i]);
    crc ^= (unsigned char)(crc & 0xff) >> 4;
    crc ^= (crc << 8) << 4;
    crc ^= ((crc & 0xff) << 4) << 1;
  }
  
  return crc;
}


/*
Description:

In this template we are using the BTLE as a UART and can send and receive packets.
The maximum size of a packet is 20 bytes.
When a command it received a response(s) are transmitted back.
Since the response is done using a Notification the peer must have opened it(subscribed to it) before any packet is transmitted.
The pipe for the UART_TX becomes available once the peer opens it.
See section 20.4.1 -> Opening a Transmit pipe 
In the master control panel, clicking Enable Services will open all the pipes on the nRF8001.

The ACI Evt Data Credit provides the radio level ack of a transmitted packet.
*/
void setup(void)
{ 
  uint16_t crc_seed = 0xFFFF;
  uint8_t msg_len;
  uint8_t crc_loop;
  
  Serial.begin(115200);
  Serial.println(F("Arduino setup"));
  
  
  //Run the CRC algorithm on the modified Setup to find the new CRC
  for (crc_loop=0; crc_loop <NB_SETUP_MESSAGES; crc_loop++)
  {
    if (NB_SETUP_MESSAGES-1 == crc_loop)
    {
      msg_len = pgm_read_byte(&setup_msgs[crc_loop].buffer[0]) - 1; //since the 2 bytes of CRC itself should not be used
                                                                    //to calculate the CRC
    }
    else
    {
      msg_len = pgm_read_byte(&setup_msgs[crc_loop].buffer[0]) + 1;
    }
    Serial.print(F("0x"));
    Serial.println(msg_len, HEX);
    crc_seed = crc_16_ccitt(crc_seed, &setup_msgs[crc_loop].buffer[0], msg_len);
  }
  Serial.print(F("0x"));
  Serial.println(crc_seed, HEX);
  
  
  /**
  Point ACI data structures to the the setup data that the nRFgo studio generated for the nRF8001
  */   
  if (NULL != services_pipe_type_mapping)
  {
    aci_state.aci_setup_info.services_pipe_type_mapping = &services_pipe_type_mapping[0];
  }
  else
  {
    aci_state.aci_setup_info.services_pipe_type_mapping = NULL;
  }
  aci_state.aci_setup_info.number_of_pipes    = NUMBER_OF_PIPES;
  aci_state.aci_setup_info.setup_msgs         = setup_msgs;
  aci_state.aci_setup_info.num_setup_msgs     = NB_SETUP_MESSAGES;
  
  /*
  Tell the ACI library, the MCU to nRF8001 pin connections.
  The Active pin is optional and can be marked UNUSED
  */	  	
  aci_state.aci_pins.board_name = BOARD_DEFAULT; //See board.h for details REDBEARLAB_SHIELD_V1_1
  aci_state.aci_pins.reqn_pin   = 9;
  aci_state.aci_pins.rdyn_pin   = 8;
  aci_state.aci_pins.mosi_pin   = MOSI;
  aci_state.aci_pins.miso_pin   = MISO;
  aci_state.aci_pins.sck_pin    = SCK;

  aci_state.aci_pins.spi_clock_divider     = SPI_CLOCK_DIV8;
	  
  aci_state.aci_pins.reset_pin             = 4;
  aci_state.aci_pins.active_pin            = UNUSED;
  aci_state.aci_pins.optional_chip_sel_pin = UNUSED;
	  
  aci_state.aci_pins.interface_is_interrupt	  = false;
  aci_state.aci_pins.interrupt_number	      = 1;

  //We reset the nRF8001 here by toggling the RESET line connected to the nRF8001
  //and initialize the data structures required to setup the nRF8001
  lib_aci_init(&aci_state);
}

void uart_tx()
{
  lib_aci_send_data(PIPE_UART_OVER_BTLE_UART_TX_TX, uart_buffer, uart_buffer_len);
  aci_state.data_credit_available--;
}

void aci_loop()
{
  // We enter the if statement only when there is a ACI event available to be processed
  if (lib_aci_event_get(&aci_state, &aci_data))
  {
    aci_evt_t * aci_evt;
    
    aci_evt = &aci_data.evt;    
    switch(aci_evt->evt_opcode)
    {
        /**
        As soon as you reset the nRF8001 you will get an ACI Device Started Event
        */
        case ACI_EVT_DEVICE_STARTED:
        {          
          aci_state.data_credit_total = aci_evt->params.device_started.credit_available;
          switch(aci_evt->params.device_started.device_mode)
          {
            case ACI_DEVICE_SETUP:
            /**
            When the device is in the setup mode
            */
            Serial.println(F("Evt Device Started: Setup"));
            if (ACI_STATUS_TRANSACTION_COMPLETE != do_aci_setup(&aci_state))
            {
              Serial.println(F("Error in ACI Setup"));
            }
            break;
            
            case ACI_DEVICE_STANDBY:
              Serial.println(F("Evt Device Started: Standby"));
              //Looking for an iPhone by sending radio advertisements
              //When an iPhone connects to us we will get an ACI_EVT_CONNECTED event from the nRF8001
              if (aci_evt->params.device_started.hw_error)
              {
                delay(20); //Magic number used to make sure the HW error event is handled correctly.
              }
              else
              {
              lib_aci_connect(180/* in seconds */, 0x0050 /* advertising interval 50ms*/);
              Serial.println(F("Advertising started"));
              }
              break;
          }
        }
        break; //ACI Device Started Event
        
      case ACI_EVT_CMD_RSP:
        //If an ACI command response event comes with an error -> stop
        if (ACI_STATUS_SUCCESS != aci_evt->params.cmd_rsp.cmd_status)
        {
          //ACI ReadDynamicData and ACI WriteDynamicData will have status codes of
          //TRANSACTION_CONTINUE and TRANSACTION_COMPLETE
          //all other ACI commands will have status code of ACI_STATUS_SCUCCESS for a successful command
          Serial.print(F("ACI Command "));
          Serial.println(aci_evt->params.cmd_rsp.cmd_opcode, HEX);
          Serial.println(F("Evt Cmd respone: Error. Arduino is in an while(1); loop"));
          while (1);
        }
        if (ACI_CMD_GET_DEVICE_VERSION == aci_evt->params.cmd_rsp.cmd_opcode)
        {
          //Store the version and configuration information of the nRF8001 in the Hardware Revision String Characteristic
          lib_aci_set_local_data(&aci_state, PIPE_DEVICE_INFORMATION_HARDWARE_REVISION_STRING_SET, 
            (uint8_t *)&(aci_evt->params.cmd_rsp.params.get_device_version), sizeof(aci_evt_cmd_rsp_params_get_device_version_t));
        }        
        break;
        
      case ACI_EVT_CONNECTED:
        Serial.println(F("Evt Connected"));
        aci_state.data_credit_available = aci_state.data_credit_total;
        
        /*
        Get the device version of the nRF8001 and store it in the Hardware Revision String
        */
        lib_aci_device_version();
        break;
        
      case ACI_EVT_PIPE_STATUS:
        Serial.println(F("Evt Pipe Status"));
        if (lib_aci_is_pipe_available(&aci_state, PIPE_UART_OVER_BTLE_UART_TX_TX) && (false == timing_change_done))
        {
          lib_aci_change_timing_GAP_PPCP(); // change the timing on the link as specified in the nRFgo studio -> nRF8001 conf. -> GAP. 
                                            // Used to increase or decrease bandwidth
          timing_change_done = true;
        }
        break;
	
	  case ACI_EVT_BOND_STATUS:
		if (ACI_BOND_STATUS_SUCCESS == aci_evt->params.bond_status.status_code)
		{
			bonded = true;
		}
		else
		{
			bonded = false;
		}
		break;
	
	  case ACI_EVT_DISPLAY_PASSKEY:
		Serial.print  (char(aci_evt->params.display_passkey.passkey[0]));
		Serial.print  (char(aci_evt->params.display_passkey.passkey[1]));
		Serial.print  (char(aci_evt->params.display_passkey.passkey[2]));
		Serial.print  (char(aci_evt->params.display_passkey.passkey[3]));
		Serial.print  (char(aci_evt->params.display_passkey.passkey[4]));
		Serial.println(char(aci_evt->params.display_passkey.passkey[5]));
		break;
        
      case ACI_EVT_TIMING:
        Serial.println(F("Evt link connection interval changed"));
        break;
        
      case ACI_EVT_DISCONNECTED:
        Serial.println(F("Evt Disconnected/Advertising timed out"));
	lib_aci_connect(180/* in seconds */, 0x0100 /* advertising interval 100ms*/);
        Serial.println(F("Advertising started"));        
        break;
        
      case ACI_EVT_DATA_RECEIVED:
        Serial.print(F("UART RX: 0x"));
        Serial.print(aci_evt->params.data_received.rx_data.pipe_number, HEX);
        {
          Serial.print(F(" Data(Hex) : "));
          for(int i=0; i<aci_evt->len - 2; i++)
          {
            Serial.print(aci_evt->params.data_received.rx_data.aci_data[i], HEX);
            uart_buffer[i] = aci_evt->params.data_received.rx_data.aci_data[i];
            Serial.print(F(" "));
          }
          uart_buffer_len = aci_evt->len - 2;
        }
        Serial.println(F(""));
        if (lib_aci_is_pipe_available(&aci_state, PIPE_UART_OVER_BTLE_UART_TX_TX))
        {
          uart_tx();
        }
        break;
   
      case ACI_EVT_DATA_CREDIT:
        aci_state.data_credit_available = aci_state.data_credit_available + aci_evt->params.data_credit.credit;
        break;
      
      case ACI_EVT_PIPE_ERROR:
        //See the appendix in the nRF8001 Product Specication for details on the error codes
        Serial.print(F("ACI Evt Pipe Error: Pipe #:"));
        Serial.print(aci_evt->params.pipe_error.pipe_number, DEC);
        Serial.print(F("  Pipe Error Code: 0x"));
        Serial.println(aci_evt->params.pipe_error.error_code, HEX);
                
        //Increment the credit available as the data packet was not sent.
        //The pipe error also represents the Attribute protocol Error Response sent from the peer and that should not be counted 
        //for the credit.
        if (ACI_STATUS_ERROR_PEER_ATT_ERROR != aci_evt->params.pipe_error.error_code)
        {
          aci_state.data_credit_available++;
        }
        break;
      case ACI_EVT_HW_ERROR:
        Serial.println(F("HW error: "));
        Serial.println(aci_evt->params.hw_error.line_num, DEC);
      
        for(uint8_t counter = 0; counter <= (aci_evt->len - 3); counter++)
        {
          Serial.write(aci_evt->params.hw_error.file_name[counter]); //uint8_t file_name[20];
        }
        Serial.println();
        lib_aci_connect(30/* in seconds */, 0x0100 /* advertising interval 100ms*/);
        Serial.println(F("Advertising started"));
        break;   
           
    }
  }
  else
  {
    //Serial.println(F("No ACI Events available"));
    // No event in the ACI Event queue and if there is no event in the ACI command queue the arduino can go to sleep
    // Arduino can go to sleep now
    // Wakeup from sleep from the RDYN line
  }
}

void loop()
{
  aci_loop();
}

