/*
   MPL3115A2 Barometric Pressure Sensor Library
By: Nathan Seidle
SparkFun Electronics
Date: September 22nd, 2013
License: This code is public domain but you buy me a beer if you use this and we meet someday (Beerware license).

This library allows an Arduino to read from the MPL3115A2 low-cost high-precision pressure sensor.

If you have feature suggestions or need support please use the github support page: https://github.com/sparkfun/MPL3115A2_Breakout

Hardware Setup: The MPL3115A2 lives on the I2C bus. Attach the SDA pin to A4, SCL to A5. Use inline 10k resistors
if you have a 5V board. If you are using the SparkFun breakout board you *do not* need 4.7k pull-up resistors 
on the bus (they are built-in).

Link to the breakout board product:

Software:
.begin() Gets sensor on the I2C bus.
.readAltitude() Returns float with meters above sealevel. Ex: 1638.94
.readAltitudeFt() Returns float with feet above sealevel. Ex: 5376.68
.readPressure() Returns float with barometric pressure in Pa. Ex: 83351.25
.readTemp() Returns float with current temperature in Celsius. Ex: 23.37
.readTempF() Returns float with current temperature in Fahrenheit. Ex: 73.96
.setModeBarometer() Puts the sensor into Pascal measurement mode.
.setModeAltimeter() Puts the sensor into altimetery mode.
.setModeStandy() Puts the sensor into Standby mode. Required when changing CTRL1 register.
.setModeActive() Start taking measurements!
.setOversampleRate(byte) Sets the # of samples from 1 to 128. See datasheet.
.enableEventFlags() Sets the fundamental event flags. Required during setup.

*/

#if defined(ARDUINO) && ARDUINO >= 100
#include "Arduino.h"
#else
#include "WProgram.h"
#endif

// Use enhanced i2c library for teensy
#if (defined(__MK20DX128__) || defined(__MK20DX256__) || defined(__MKL26Z64__) || \
                           defined(__MK64FX512__) || defined(__MK66FX1M0__)) // 3.0/3.1-3.2/LC/3.5/3.6
#include <i2c_t3.h>
#else
#include <Wire.h>
#endif

#include "SparkFunMPL3115A2.h"

#define MPL3115A2_ADDRESS 0x60 // Unshifted 7-bit I2C address for sensor
#define STATUS     0x00
#define OUT_P_MSB  0x01
#define OUT_P_CSB  0x02
#define OUT_P_LSB  0x03
#define OUT_T_MSB  0x04
#define OUT_T_LSB  0x05
#define DR_STATUS  0x06
#define OUT_P_DELTA_MSB  0x07
#define OUT_P_DELTA_CSB  0x08
#define OUT_P_DELTA_LSB  0x09
#define OUT_T_DELTA_MSB  0x0A
#define OUT_T_DELTA_LSB  0x0B
#define WHO_AM_I   0x0C
#define F_STATUS   0x0D
#define F_DATA     0x0E
#define F_SETUP    0x0F
#define TIME_DLY   0x10
#define SYSMOD     0x11
#define INT_SOURCE 0x12
#define PT_DATA_CFG 0x13
#define BAR_IN_MSB 0x14
#define BAR_IN_LSB 0x15
#define P_TGT_MSB  0x16
#define P_TGT_LSB  0x17
#define T_TGT      0x18
#define P_WND_MSB  0x19
#define P_WND_LSB  0x1A
#define T_WND      0x1B
#define P_MIN_MSB  0x1C
#define P_MIN_CSB  0x1D
#define P_MIN_LSB  0x1E
#define T_MIN_MSB  0x1F
#define T_MIN_LSB  0x20
#define P_MAX_MSB  0x21
#define P_MAX_CSB  0x22
#define P_MAX_LSB  0x23
#define T_MAX_MSB  0x24
#define T_MAX_LSB  0x25
#define CTRL_REG1  0x26
#define CTRL_REG2  0x27
#define CTRL_REG3  0x28
#define CTRL_REG4  0x29
#define CTRL_REG5  0x2A
#define OFF_P      0x2B
#define OFF_T      0x2C
#define OFF_H      0x2D

MPL3115A2::MPL3115A2():
	m_pressureMsb{0}, m_pressureCsb{0}, m_pressureLsb{0},
	m_tempMsb{0}, m_tempLsb{0},
	m_barometerMode{false} {}

void MPL3115A2::begin(bool beginWire) {
	if (beginWire) {
		Wire.begin();
	}
	setModeAltimeter();
	toggleOneShot();
}

bool MPL3115A2::available() {
	return IIC_Read(STATUS) & (1<<1);
}

void MPL3115A2::read() {
	//Wait for PDR bit, indicates we have new pressure data
	constexpr size_t waitPeriod = 512;
	size_t startReadTime = millis();
	while (!available()) {
		if (startReadTime - millis() >= waitPeriod) {
			return;
		}
	}

	// Read pressure registers
	Wire.beginTransmission(MPL3115A2_ADDRESS);
	Wire.write(OUT_P_MSB);  // Address of data to get
	Wire.endTransmission(false); // Send data to I2C dev with option for a repeated start. THIS IS NECESSARY and not supported before Arduino V1.0.1!
	if (Wire.requestFrom(MPL3115A2_ADDRESS, 3) != 3) { // Request three bytes
		return;
	}

	m_pressureMsb = Wire.read();
	m_pressureCsb = Wire.read();
	m_pressureLsb = Wire.read();

	// Read temperature registers
	Wire.beginTransmission(MPL3115A2_ADDRESS);
	Wire.write(OUT_T_MSB);  // Address of data to get
	Wire.endTransmission(false); // Send data to I2C dev with option for a repeated start. THIS IS NECESSARY and not supported before Arduino V1.0.1!
	if (Wire.requestFrom(MPL3115A2_ADDRESS, 2) != 2) { // Request two bytes
		return;
	}

	m_tempMsb = Wire.read();
	m_tempLsb = Wire.read();

	toggleOneShot();
}

//Returns the number of meters above sea level
//Returns -1 if no new data is available
float MPL3115A2::altitude() {
	if (m_barometerMode) return -999;

	// The least significant bytes l_altitude and l_temp are 4-bit,
	// fractional values, so you must cast the calulation in (float),
	// shift the value over 4 spots to the right and divide by 16 (since 
	// there are 16 values in 4-bits). 
	float tempCsb = (m_pressureLsb>>4)/16.0;

	return (float)( (m_pressureMsb << 8) | m_pressureCsb) + tempCsb;
}

//Returns the number of feet above sea level
float MPL3115A2::altitudeFt() {
	return altitude() * 3.28084;
}

//Reads the current pressure in Pa
//Unit must be set in barometric pressure mode
//Returns -1 if no new data is available
float MPL3115A2::pressure() {
	if (!m_barometerMode) return -999;

	// Pressure comes back as a left shifted 20 bit number
	long pressure_whole = (long)m_pressureMsb<<16 | 
		(long)m_pressureCsb<<8 | 
		(long)m_pressureLsb;
	pressure_whole >>= 6; //Pressure is an 18 bit number with 2 bits of decimal. Get rid of decimal portion.

	uint8_t lsb = m_pressureLsb;
	lsb &= B00110000; //Bits 5/4 represent the fractional component
	lsb >>= 4; //Get it right aligned
	float pressure_decimal = (float)lsb/4.0; //Turn it into fraction

	float pressure = (float)pressure_whole + pressure_decimal;

	return pressure;
}

float MPL3115A2::temp() {
	//Negative temperature fix by D.D.G.
	uint16_t foo = 0;
	bool negSign = false;
	uint8_t msb = m_tempMsb, lsb = m_tempLsb;

	//Check for 2s compliment
	if (msb > 0x7F) {
		foo = ~((msb << 8) + lsb) + 1;  //2’s complement
		msb = foo >> 8;
		lsb = foo & 0x00F0; 
		negSign = true;
	}

	// The least significant bytes l_altitude and l_temp are 4-bit,
	// fractional values, so you must cast the calulation in (float),
	// shift the value over 4 spots to the right and divide by 16 (since 
	// there are 16 values in 4-bits). 
	float templsb = (lsb>>4)/16.0; //temp, fraction of a degree

	float temperature = (float)(msb + templsb);

	if (negSign) temperature = -temperature;

	return temperature;
}

//Give me temperature in fahrenheit!
float MPL3115A2::tempF() {
	return (temp() * 9.0)/ 5.0 + 32.0; // Convert celsius to fahrenheit
}

//Sets the mode to Barometer
//CTRL_REG1, ALT bit
void MPL3115A2::setModeBarometer() {
	uint8_t tempSetting = IIC_Read(CTRL_REG1); //Read current settings
	tempSetting &= ~(1<<7); //Clear ALT bit
	IIC_Write(CTRL_REG1, tempSetting);
	m_barometerMode = true;
}

//Sets the mode to Altimeter
//CTRL_REG1, ALT bit
void MPL3115A2::setModeAltimeter() {
	uint8_t tempSetting = IIC_Read(CTRL_REG1); //Read current settings
	tempSetting |= (1<<7); //Set ALT bit
	IIC_Write(CTRL_REG1, tempSetting);
	m_barometerMode = false;
}

//Puts the sensor in standby mode
//This is needed so that we can modify the major control registers
void MPL3115A2::setModeStandby() {
	uint8_t tempSetting = IIC_Read(CTRL_REG1); //Read current settings
	tempSetting &= ~(1<<0); //Clear SBYB bit for Standby mode
	IIC_Write(CTRL_REG1, tempSetting);
}

//Puts the sensor in active mode
//This is needed so that we can modify the major control registers
void MPL3115A2::setModeActive() {
	uint8_t tempSetting = IIC_Read(CTRL_REG1); //Read current settings
	tempSetting |= (1<<0); //Set SBYB bit for Active mode
	IIC_Write(CTRL_REG1, tempSetting);
}

//Call with a rate from 0 to 7. See page 33 for table of ratios.
//Sets the over sample rate. Datasheet calls for 128 but you can set it 
//from 1 to 128 samples. The higher the oversample rate the greater
//the time between data samples.
void MPL3115A2::setOversampleRate(uint8_t sampleRate) {
	if(sampleRate > 7) sampleRate = 7; //OS cannot be larger than 0b.0111
	sampleRate <<= 3; //Align it for the CTRL_REG1 register

	uint8_t tempSetting = IIC_Read(CTRL_REG1); //Read current settings
	tempSetting &= B11000111; //Clear out old OS bits
	tempSetting |= sampleRate; //Mask in new OS bits
	IIC_Write(CTRL_REG1, tempSetting);
}

//Enables the pressure and temp measurement event flags so that we can
//test against them. This is recommended in datasheet during setup.
void MPL3115A2::enableEventFlags() {
	IIC_Write(PT_DATA_CFG, 0x07); // Enable all three pressure and temp event flags 
}

//Clears then sets the OST bit which causes the sensor to immediately take another reading
//Needed to sample faster than 1Hz
void MPL3115A2::toggleOneShot(void) {
	uint8_t tempSetting = IIC_Read(CTRL_REG1); //Read current settings
	tempSetting &= ~(1<<1); //Clear OST bit
	IIC_Write(CTRL_REG1, tempSetting);

	tempSetting = IIC_Read(CTRL_REG1); //Read current settings to be safe
	tempSetting |= (1<<1); //Set OST bit
	IIC_Write(CTRL_REG1, tempSetting);
}


// These are the two I2C functions in this sketch.
uint8_t MPL3115A2::IIC_Read(uint8_t regAddr) {
	// This function reads one byte over IIC
	Wire.beginTransmission(MPL3115A2_ADDRESS);
	Wire.write(regAddr);  // Address of CTRL_REG1
	Wire.endTransmission(false); // Send data to I2C dev with option for a repeated start. THIS IS NECESSARY and not supported before Arduino V1.0.1!
	Wire.requestFrom(MPL3115A2_ADDRESS, 1); // Request the data...
	return Wire.read();
}

void MPL3115A2::IIC_Write(uint8_t regAddr, uint8_t value) {
	// This function writes one byto over IIC
	Wire.beginTransmission(MPL3115A2_ADDRESS);
	Wire.write(regAddr);
	Wire.write(value);
	Wire.endTransmission(true);
}

