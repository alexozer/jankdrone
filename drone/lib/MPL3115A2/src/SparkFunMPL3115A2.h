/* 
MPL3115A2 Barometric Pressure Sensor Library
Original By: Nathan Seidle
This version by: Alex Ozer
SparkFun Electronics
Date: September 24th, 2013
License: This code is public domain but you buy me a beer if you use this and we meet someday (Beerware license).

Get pressure, altitude and temperature from the MPL3115A2 sensor.

*/

#pragma once

#include <cstdint>

class MPL3115A2 {
	public:
		MPL3115A2();

		void begin(bool beginWire = true); // Gets sensor on the I2C bus.
		bool available(); // Returns if sensor data is available for immediate read
		void read(); // Waits until sensor data available and then reads

		float altitude(); // Returns float with meters above sealevel. Ex: 1638.94
		float altitudeFt(); // Returns float with feet above sealevel. Ex: 5376.68
		float pressure(); // Returns float with barometric pressure in Pa. Ex: 83351.25
		float temp(); // Returns float with current temperature in Celsius. Ex: 23.37
		float tempF(); // Returns float with current temperature in Fahrenheit. Ex: 73.96

		void setModeBarometer(); // Puts the sensor into Pascal measurement mode.
		void setModeAltimeter(); // Puts the sensor into altimetery mode.
		void setModeStandby(); // Puts the sensor into Standby mode. Required when changing CTRL1 register.
		void setModeActive(); // Start taking measurements!
		void setOversampleRate(uint8_t); // Sets the # of samples from 1 to 128. See datasheet.
		void enableEventFlags(); // Sets the fundamental event flags. Required during setup.

	private:
		// Bytes from most recent data read
		uint8_t m_pressureMsb, m_pressureCsb, m_pressureLsb;
		uint8_t m_tempMsb, m_tempLsb;
		bool m_barometerMode;

		void toggleOneShot();
		uint8_t IIC_Read(uint8_t regAddr);
		void IIC_Write(uint8_t regAddr, uint8_t value);
};
