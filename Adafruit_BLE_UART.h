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

#if ARDUINO >= 100
 #include "Arduino.h"
#else
 #include "WProgram.h"
#endif

#ifndef _ADAFRUIT_BLE_UART_H_
#define _ADAFRUIT_BLE_UART_H_

#include "utility/aci_evts.h"

/* Uncomment the next line to display debug messages when reading/writing packets */
//#define BLE_RW_DEBUG

extern "C" 
{
  /* Callback prototypes */
  typedef void (*aci_callback)(aci_evt_opcode_t event);
  typedef void (*rx_callback) (uint8_t *buffer, uint8_t len);
}

class Adafruit_BLE_UART : public Stream
{
 public:
  Adafruit_BLE_UART (int8_t req, int8_t rdy, int8_t rst);
  
  bool begin   ( uint16_t advTimeout = 0, uint16_t advInterval = 80 );
  void pollACI ( void );
  size_t write ( const uint8_t * buffer, size_t len ) override;
  size_t write ( uint8_t buffer) override;

  size_t println(const char * thestr);
  size_t print(const char * thestr);
  size_t print(String thestr);
  size_t print(int theint);
  size_t print(const __FlashStringHelper *ifsh);

  void setACIcallback(aci_callback aciEvent = NULL);
  void setRXcallback(rx_callback rxEvent = NULL);
  void setDeviceName(const char * deviceName);

  // Stream compatibility
  int available(void) override;
  int read(void) override;
  int peek(void) override;
  void flush(void) override;

  aci_evt_opcode_t getState(void);

 private:  
  void defaultACICallback(aci_evt_opcode_t event);
  void defaultRX(uint8_t *buffer, uint8_t len);

  // callbacks you can set with setCallback function for user extension
  aci_callback aci_event;
  rx_callback  rx_event; 

  bool         debugMode;
  uint16_t     adv_timeout;
  uint16_t     adv_interval;
  char         device_name[8];

  aci_evt_opcode_t currentStatus;
  
  // pins usd
  int8_t _REQ, _RDY, _RST;
};

#endif
