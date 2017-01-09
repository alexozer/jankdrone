/*********************************************************************
This is a library for our nRF8001 Bluetooth Low Energy Breakout

  Pick one up today in the adafruit shop!
  ------> http://www.adafruit.com/products/1697

These displays use SPI to communicate, 4 or 5 pins are required to
interface

Adafruit invests time and resources providing this open source code,
please support Adafruit and open-source hardware by purchasing
products from Adafruit!

Written by Kevin Townsend/KTOWN  for Adafruit Industries.
MIT license, check LICENSE for more information
All text above, and the splash screen below must be included in any redistribution
*********************************************************************/
#include <SPI.h>
#include <avr/pgmspace.h>
#if defined(__AVR__)
  #include <util/delay.h>
#endif
#include <stdlib.h>
#include <ble_system.h>
#include <lib_aci.h>
#include <aci_setup.h>
#include "uart/services.h"

#include "Adafruit_BLE_UART.h"

/* Get the service pipe data created in nRFGo Studio */
#ifdef SERVICES_PIPE_TYPE_MAPPING_CONTENT
    static services_pipe_type_mapping_t
        services_pipe_type_mapping[NUMBER_OF_PIPES] = SERVICES_PIPE_TYPE_MAPPING_CONTENT;
#else
    #define NUMBER_OF_PIPES 0
    static services_pipe_type_mapping_t * services_pipe_type_mapping = NULL;
#endif

/* Length of the buffer used to store flash strings temporarily when printing. */
#define PRINT_BUFFER_SIZE 20

/* Store the setup for the nRF8001 in the flash of the AVR to save on RAM */
static const hal_aci_data_t setup_msgs[NB_SETUP_MESSAGES] PROGMEM = SETUP_MESSAGES_CONTENT;

static struct aci_state_t aci_state;            /* ACI state data */
static hal_aci_evt_t  aci_data;                 /* Command buffer */
static bool timing_change_done = false;

// This is the Uart RX buffer, which we manage internally when data is available!
#define ADAFRUIT_BLE_UART_RXBUFFER_SIZE 64
uint8_t adafruit_ble_rx_buffer[ADAFRUIT_BLE_UART_RXBUFFER_SIZE];
volatile uint16_t adafruit_ble_rx_head;
volatile uint16_t adafruit_ble_rx_tail;


int8_t HAL_IO_RADIO_RESET, HAL_IO_RADIO_REQN, HAL_IO_RADIO_RDY, HAL_IO_RADIO_IRQ;

/**************************************************************************/
/*!
    Constructor for the UART service
*/
/**************************************************************************/
// default RX callback!

void Adafruit_BLE_UART::defaultRX(uint8_t *buffer, uint8_t len)
{
  for(int i=0; i<len; i++)
  {
    uint16_t new_head = (uint16_t)(adafruit_ble_rx_head + 1) % ADAFRUIT_BLE_UART_RXBUFFER_SIZE;

    // if we should be storing the received character into the location
    // just before the tail (meaning that the head would advance to the
    // current location of the tail), we're about to overflow the buffer
    // and so we don't write the character or advance the head.
    if (new_head != adafruit_ble_rx_tail) {
      adafruit_ble_rx_buffer[adafruit_ble_rx_head] = buffer[i];

      // debug echo print
      // Serial.print((char)buffer[i]);

      adafruit_ble_rx_head = new_head;
    }
  }

  /*
  Serial.print("Buffer: ");
  for(int i=0; i<adafruit_ble_rx_head; i++)
    {
      Serial.print(" 0x"); Serial.print((char)adafruit_ble_rx_buffer[i], HEX);
    }
  Serial.println();
  */
}


/* Stream stuff */

int Adafruit_BLE_UART::available(void)
{
  return (uint16_t)(ADAFRUIT_BLE_UART_RXBUFFER_SIZE + adafruit_ble_rx_head - adafruit_ble_rx_tail)
    % ADAFRUIT_BLE_UART_RXBUFFER_SIZE;
}

int Adafruit_BLE_UART::read(void)
{
  // if the head isn't ahead of the tail, we don't have any characters
  if (adafruit_ble_rx_head == adafruit_ble_rx_tail) {
    return -1;
  } else {
    unsigned char c = adafruit_ble_rx_buffer[adafruit_ble_rx_tail];
    adafruit_ble_rx_tail ++;
    adafruit_ble_rx_tail %= ADAFRUIT_BLE_UART_RXBUFFER_SIZE;
    return c;
  }
}

int Adafruit_BLE_UART::peek(void)
{
  if (adafruit_ble_rx_head == adafruit_ble_rx_tail) {
    return -1;
  } else {
    return adafruit_ble_rx_buffer[adafruit_ble_rx_tail];
  }
}

void Adafruit_BLE_UART::flush(void)
{
  // MEME: KTOWN what do we do here?
}



//// more callbacks

void Adafruit_BLE_UART::defaultACICallback(aci_evt_opcode_t event)
{
  currentStatus = event;
}

aci_evt_opcode_t Adafruit_BLE_UART::getState(void) {
  return currentStatus;
}



/**************************************************************************/
/*!
    Constructor for the UART service
*/
/**************************************************************************/
Adafruit_BLE_UART::Adafruit_BLE_UART(int8_t req, int8_t rdy, int8_t rst)
{
  debugMode = true;

  HAL_IO_RADIO_REQN = req;
  HAL_IO_RADIO_RDY = rdy;
  HAL_IO_RADIO_RESET = rst;

  rx_event = NULL;
  aci_event = NULL;

  memset(device_name, 0x00, 8);

  adafruit_ble_rx_head = adafruit_ble_rx_tail = 0;

  currentStatus = ACI_EVT_DISCONNECTED;
}

void Adafruit_BLE_UART::setACIcallback(aci_callback aciEvent) {
  aci_event = aciEvent;
}

void Adafruit_BLE_UART::setRXcallback(rx_callback rxEvent) {
  rx_event = rxEvent;
}

/**************************************************************************/
/*!
    Transmits data out via the TX characteristic (when available)
*/
/**************************************************************************/
size_t Adafruit_BLE_UART::println(const char * thestr)
{
  uint8_t len     = strlen(thestr),
          written = len ? write((uint8_t *)thestr, len) : 0;
  if(written == len) written += write((uint8_t *)"\r\n", 2);

  return written;
}

size_t Adafruit_BLE_UART::print(const char * thestr)
{
  return write((uint8_t *)thestr, strlen(thestr));
}

size_t Adafruit_BLE_UART::print(String thestr)
{
  return write((uint8_t *)thestr.c_str(), thestr.length());
}

size_t Adafruit_BLE_UART::print(int theint)
{
  char message[4*sizeof(int)+1] = {0};
  itoa(theint, message, 10);
  return write((uint8_t *)message, strlen(message));
}

size_t Adafruit_BLE_UART::print(const __FlashStringHelper *ifsh)
{
  // Copy bytes from flash string into RAM, then send them a buffer at a time.
  char buffer[PRINT_BUFFER_SIZE] = {0};
  const char PROGMEM *p = (const char PROGMEM *)ifsh;
  size_t written = 0;
  int i = 0;
  unsigned char c = pgm_read_byte(p++);
  // Read data from flash until a null terminator is found.
  while (c != 0) {
    // Copy data to RAM and increase buffer index.
    buffer[i] = c;
    i++;
    if (i >= PRINT_BUFFER_SIZE) {
      // Send buffer when it's full and reset buffer index.
      written += write((uint8_t *)buffer, PRINT_BUFFER_SIZE);
      i = 0;
    }
    // Grab a new byte from flash.
    c = pgm_read_byte(p++);
  }
  if (i > 0) {
    // Send any remaining data in the buffer.
    written += write((uint8_t *)buffer, i);
  }
  return written;
}

size_t Adafruit_BLE_UART::write(const uint8_t * buffer, size_t len)
{
  uint8_t bytesThisPass, sent = 0;

  /* Blocking delay waiting for available credit(s) */
  while (0 == aci_state.data_credit_available)
  {
    pollACI();
    delay(10);
  }

#ifdef BLE_RW_DEBUG
  Serial.print(F("\tWriting out to BTLE:"));
  for (uint8_t i=0; i<len; i++) {
    Serial.print(F(" 0x")); Serial.print(buffer[i], HEX);
  }
  Serial.println();
#endif

  while(len) { // Parcelize into chunks
    bytesThisPass = len;
    if(bytesThisPass > ACI_PIPE_TX_DATA_MAX_LEN)
       bytesThisPass = ACI_PIPE_TX_DATA_MAX_LEN;

    if(!lib_aci_is_pipe_available(&aci_state, PIPE_UART_OVER_BTLE_UART_TX_TX))
    {
      pollACI();
      continue;
    }

    lib_aci_send_data(PIPE_UART_OVER_BTLE_UART_TX_TX, (uint8_t*)&buffer[sent],
      bytesThisPass);
    aci_state.data_credit_available--;

    delay(35); // required delay between sends

    if(!(len -= bytesThisPass)) break;
    sent += bytesThisPass;
  }

  return sent;
}

size_t Adafruit_BLE_UART::write(uint8_t buffer)
{
  /* Blocking delay waiting for available credit(s) */
  while (0 == aci_state.data_credit_available)
  {
    pollACI();
    delay(10);
  }

#ifdef BLE_RW_DEBUG
  Serial.print(F("\tWriting one byte 0x")); Serial.println(buffer, HEX);
#endif
  if (lib_aci_is_pipe_available(&aci_state, PIPE_UART_OVER_BTLE_UART_TX_TX))
  {
    lib_aci_send_data(PIPE_UART_OVER_BTLE_UART_TX_TX, &buffer, 1);
    aci_state.data_credit_available--;

    delay(35); // required delay between sends
    return 1;
  }

  pollACI();

  return 0;
}

/**************************************************************************/
/*!
    Update the device name (7 characters or less!)
*/
/**************************************************************************/
void Adafruit_BLE_UART::setDeviceName(const char * deviceName)
{
  if (strlen(deviceName) > 7)
  {
    /* String too long! */
    return;
  }
  else
  {
    memcpy(device_name, deviceName, strlen(deviceName)+1);
  }
}

/**************************************************************************/
/*!
    Handles low level ACI events, and passes them up to an application
    level callback when appropriate
*/
/**************************************************************************/
void Adafruit_BLE_UART::pollACI()
{
  // We enter the if statement only when there is a ACI event available to be processed
  if (lib_aci_event_get(&aci_state, &aci_data))
  {
    aci_evt_t * aci_evt;

    aci_evt = &aci_data.evt;
    switch(aci_evt->evt_opcode)
    {
        /* As soon as you reset the nRF8001 you will get an ACI Device Started Event */
        case ACI_EVT_DEVICE_STARTED:
        {
          aci_state.data_credit_total = aci_evt->params.device_started.credit_available;
          switch(aci_evt->params.device_started.device_mode)
          {
            case ACI_DEVICE_SETUP:
            /* Device is in setup mode! */
            if (ACI_STATUS_TRANSACTION_COMPLETE != do_aci_setup(&aci_state))
            {
              if (debugMode) {
                Serial.println(F("Error in ACI Setup"));
              }
            }
            break;

            case ACI_DEVICE_STANDBY:
              /* Start advertising ... first value is advertising time in seconds, the */
              /* second value is the advertising interval in 0.625ms units */
              if (device_name[0] != 0x00)
              {
                /* Update the device name */
                lib_aci_set_local_data(&aci_state, PIPE_GAP_DEVICE_NAME_SET , (uint8_t *)&device_name, strlen(device_name));
              }
              lib_aci_connect(adv_timeout, adv_interval);
              defaultACICallback(ACI_EVT_DEVICE_STARTED);
	      if (aci_event)
		aci_event(ACI_EVT_DEVICE_STARTED);
          }
        }
        break;

      case ACI_EVT_CMD_RSP:
        /* If an ACI command response event comes with an error -> stop */
        if (ACI_STATUS_SUCCESS != aci_evt->params.cmd_rsp.cmd_status)
        {
          // ACI ReadDynamicData and ACI WriteDynamicData will have status codes of
          // TRANSACTION_CONTINUE and TRANSACTION_COMPLETE
          // all other ACI commands will have status code of ACI_STATUS_SUCCESS for a successful command
          if (debugMode) {
            Serial.print(F("ACI Command "));
            Serial.println(aci_evt->params.cmd_rsp.cmd_opcode, HEX);
            Serial.println(F("Evt Cmd respone: Error. Arduino is in an while(1); loop"));
          }
          while (1);
        }
        if (ACI_CMD_GET_DEVICE_VERSION == aci_evt->params.cmd_rsp.cmd_opcode)
        {
          // Store the version and configuration information of the nRF8001 in the Hardware Revision String Characteristic
          lib_aci_set_local_data(&aci_state, PIPE_DEVICE_INFORMATION_HARDWARE_REVISION_STRING_SET,
            (uint8_t *)&(aci_evt->params.cmd_rsp.params.get_device_version), sizeof(aci_evt_cmd_rsp_params_get_device_version_t));
        }
        break;

      case ACI_EVT_CONNECTED:
        aci_state.data_credit_available = aci_state.data_credit_total;
        /* Get the device version of the nRF8001 and store it in the Hardware Revision String */
        lib_aci_device_version();

	defaultACICallback(ACI_EVT_CONNECTED);
	if (aci_event)
	  aci_event(ACI_EVT_CONNECTED);

      case ACI_EVT_PIPE_STATUS:
        if (lib_aci_is_pipe_available(&aci_state, PIPE_UART_OVER_BTLE_UART_TX_TX) && (false == timing_change_done))
        {
          lib_aci_change_timing_GAP_PPCP(); // change the timing on the link as specified in the nRFgo studio -> nRF8001 conf. -> GAP.
                                            // Used to increase or decrease bandwidth
          timing_change_done = true;
        }
        break;

      case ACI_EVT_TIMING:
        /* Link connection interval changed */
        break;

      case ACI_EVT_DISCONNECTED:
        /* Restart advertising ... first value is advertising time in seconds, the */
        /* second value is the advertising interval in 0.625ms units */

	defaultACICallback(ACI_EVT_DISCONNECTED);
	if (aci_event)
	  aci_event(ACI_EVT_DISCONNECTED);

	lib_aci_connect(adv_timeout, adv_interval);

	defaultACICallback(ACI_EVT_DEVICE_STARTED);
	if (aci_event)
	  aci_event(ACI_EVT_DEVICE_STARTED);
	break;

      case ACI_EVT_DATA_RECEIVED:
	defaultRX(aci_evt->params.data_received.rx_data.aci_data, aci_evt->len - 2);
        if (rx_event)
	  rx_event(aci_evt->params.data_received.rx_data.aci_data, aci_evt->len - 2);
        break;

      case ACI_EVT_DATA_CREDIT:
        aci_state.data_credit_available = aci_state.data_credit_available + aci_evt->params.data_credit.credit;
        break;

      case ACI_EVT_PIPE_ERROR:
        /* See the appendix in the nRF8001 Product Specication for details on the error codes */
        if (debugMode) {
          Serial.print(F("ACI Evt Pipe Error: Pipe #:"));
          Serial.print(aci_evt->params.pipe_error.pipe_number, DEC);
          Serial.print(F("  Pipe Error Code: 0x"));
          Serial.println(aci_evt->params.pipe_error.error_code, HEX);
        }

        /* Increment the credit available as the data packet was not sent */
        aci_state.data_credit_available++;
        break;
    }
  }
  else
  {
    // Serial.println(F("No ACI Events available"));
    // No event in the ACI Event queue and if there is no event in the ACI command queue the arduino can go to sleep
    // Arduino can go to sleep now
    // Wakeup from sleep from the RDYN line
  }
}

/**************************************************************************/
/*!
    Configures the nRF8001 and starts advertising the UART Service

    @param[in]  advTimeout
                The advertising timeout in seconds (0 = infinite advertising)
    @param[in]  advInterval
                The delay between advertising packets in 0.625ms units
*/
/**************************************************************************/
bool Adafruit_BLE_UART::begin(uint16_t advTimeout, uint16_t advInterval)
{
  /* Store the advertising timeout and interval */
  adv_timeout = advTimeout;   /* ToDo: Check range! */
  adv_interval = advInterval; /* ToDo: Check range! */

  /* Setup the service data from nRFGo Studio (services.h) */
  if (NULL != services_pipe_type_mapping)
  {
    aci_state.aci_setup_info.services_pipe_type_mapping = &services_pipe_type_mapping[0];
  }
  else
  {
    aci_state.aci_setup_info.services_pipe_type_mapping = NULL;
  }
  aci_state.aci_setup_info.number_of_pipes    = NUMBER_OF_PIPES;
  aci_state.aci_setup_info.setup_msgs         = (hal_aci_data_t*)setup_msgs;
  aci_state.aci_setup_info.num_setup_msgs     = NB_SETUP_MESSAGES;

  /* Pass the service data into the appropriate struct in the ACI */
  lib_aci_init(&aci_state);

  /* ToDo: Check for chip ID to make sure we're connected! */

  return true;
}
