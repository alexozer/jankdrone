//      This library provides the high-level functions needed to use the I2C
//	serial interface supported by the hardware of several AVR processors.
#include <avr/io.h>
#include <avr/interrupt.h>
#include "types.h"
#include "defs.h"

// TWSR values (not bits)
// (taken from avr-libc twi.h - thank you Marek Michalkiewicz)
// Master
#define TW_START					0x08
#define TW_REP_START				0x10
// Master Transmitter
#define TW_MT_SLA_ACK				0x18
#define TW_MT_SLA_NACK				0x20
#define TW_MT_DATA_ACK				0x28
#define TW_MT_DATA_NACK				0x30
#define TW_MT_ARB_LOST				0x38
// Master Receiver
#define TW_MR_ARB_LOST				0x38
#define TW_MR_SLA_ACK				0x40
#define TW_MR_SLA_NACK				0x48
#define TW_MR_DATA_ACK				0x50
#define TW_MR_DATA_NACK				0x58
// Slave Transmitter
#define TW_ST_SLA_ACK				0xA8
#define TW_ST_ARB_LOST_SLA_ACK		0xB0
#define TW_ST_DATA_ACK				0xB8
#define TW_ST_DATA_NACK				0xC0
#define TW_ST_LAST_DATA				0xC8
// Slave Receiver
#define TW_SR_SLA_ACK				0x60
#define TW_SR_ARB_LOST_SLA_ACK		0x68
#define TW_SR_GCALL_ACK				0x70
#define TW_SR_ARB_LOST_GCALL_ACK	0x78
#define TW_SR_DATA_ACK				0x80
#define TW_SR_DATA_NACK				0x88
#define TW_SR_GCALL_DATA_ACK		0x90
#define TW_SR_GCALL_DATA_NACK		0x98
#define TW_SR_STOP					0xA0
// Misc
#define TW_NO_INFO					0xF8
#define TW_BUS_ERROR				0x00

// defines and constants
#define TWCR_CMD_MASK		0x0F
#define TWSR_STATUS_MASK	0xF8

// return values
#define I2C_OK				0x00
#define I2C_ERROR_NODEV		0x01

#define sbi(var, mask)   ((var) |= (uint8_t)(1 << mask))
#define cbi(var, mask)   ((var) &= (uint8_t)~(1 << mask))

#define WRITE_sda() DDRC = DDRC | 0b00010000 //SDA must be output when writing
#define READ_sda()  DDRC = DDRC & 0b11101111 //SDA must be input when reading - don't forget the resistor on SDA!!

// functions

//! Initialize I2C (TWI) interface
void i2cInit(void);

//! Set the I2C transaction bitrate (in KHz)
void i2cSetBitrate(unsigned short bitrateKHz);

// Low-level I2C transaction commands 
//! Send an I2C start condition in Master mode
void i2cSendStart(void);
//! Send an I2C stop condition in Master mode
void i2cSendStop(void);
//! Wait for current I2C operation to complete
void i2cWaitForComplete(void);
//! Send an (address|R/W) combination or a data byte over I2C
void i2cSendByte(unsigned char data);
//! Receive a data byte over I2C  
// ackFlag = TRUE if recevied data should be ACK'ed
// ackFlag = FALSE if recevied data should be NACK'ed
void i2cReceiveByte(unsigned char ackFlag);
//! Pick up the data that was received with i2cReceiveByte()
unsigned char i2cGetReceivedByte(void);
//! Get current I2c bus status from TWSR
unsigned char i2cGetStatus(void);
void delay_ms(uint16_t x);

// high-level I2C transaction commands

//! send I2C data to a device on the bus (non-interrupt based)
unsigned char i2cMasterSendNI(unsigned char deviceAddr, unsigned char length, unsigned char* data);
//! receive I2C data from a device on the bus (non-interrupt based)
unsigned char i2cMasterReceiveNI(unsigned char deviceAddr, unsigned char length, unsigned char *data);

/*********************
 ****I2C Functions****
 *********************/

void i2cInit(void)
{
	// set i2c bit rate to 40KHz
	i2cSetBitrate(100);
	// enable TWI (two-wire interface)
	sbi(TWCR, TWEN);	// Enable TWI
}

void i2cSetBitrate(unsigned short bitrateKHz)
{
	unsigned char bitrate_div;
	// set i2c bitrate
	// SCL freq = F_CPU/(16+2*TWBR))
	cbi(TWSR, TWPS0);
	cbi(TWSR, TWPS1);
	
	//calculate bitrate division	
	bitrate_div = ((F_CPU/4000l)/bitrateKHz);
	if(bitrate_div >= 16)
		bitrate_div = (bitrate_div-16)/2;
	outb(TWBR, bitrate_div);
}

void i2cSendStart(void)
{
	WRITE_sda();
	// send start condition
	TWCR = (1<<TWINT)|(1<<TWSTA)|(1<<TWEN);
}

void i2cSendStop(void)
{
	// transmit stop condition
        TWCR = (1<<TWINT)|(1<<TWEN)|(1<<TWSTO);
}

void i2cWaitForComplete(void)
{
	int i = 0;		//time out variable
	
	// wait for i2c interface to complete operation
    while ((!(TWCR & (1<<TWINT))) && (i < 90))
		i++;
}

void i2cSendByte(unsigned char data)
{
	delay_ms(1);
	//printf("sending 0x%x\n", data);
	WRITE_sda();
	// save data to the TWDR
	TWDR = data;
	// begin send
	TWCR = (1<<TWINT)|(1<<TWEN);
}

void i2cReceiveByte(unsigned char ackFlag)
{
	// begin receive over i2c
	if( ackFlag )
	{
		// ackFlag = TRUE: ACK the recevied data
		outb(TWCR, (inb(TWCR)&TWCR_CMD_MASK)|BV(TWINT)|BV(TWEA));
	}
	else
	{
		// ackFlag = FALSE: NACK the recevied data
		outb(TWCR, (inb(TWCR)&TWCR_CMD_MASK)|BV(TWINT));
	}
}

unsigned char i2cGetReceivedByte(void)
{
	// retieve received data byte from i2c TWDR
	return( inb(TWDR) );
}

unsigned char i2cGetStatus(void)
{
	// retieve current i2c status from i2c TWSR
	return( inb(TWSR) );
}

void delay_ms(uint16_t x)
{
  uint8_t y, z;
  for ( ; x > 0 ; x--){
    for ( y = 0 ; y < 90 ; y++){
      for ( z = 0 ; z < 6 ; z++){
        asm volatile ("nop");
      }
    }
  }
}
