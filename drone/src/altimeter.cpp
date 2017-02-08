#include "shm.h"
#include "altimeter.h"

Altimeter::Altimeter(): m_groundAltitude{0} {
	m_sensor.begin(false);
	m_sensor.setModeAltimeter();
	m_sensor.setOversampleRate(7); // Set Oversample to the recommended 128
	m_sensor.enableEventFlags(); // Enable all three pressure and temp event flags 
	shm().switches.calibrateAltimeter = true;
}

void Altimeter::operator()() {
	if (shm().switches.calibrateAltimeter) {
		m_sensor.read();
		m_groundAltitude = m_sensor.altitude();
		shm().switches.calibrateAltimeter = false;

	} else if (m_sensor.available()) {
		m_sensor.read();
		shm().placement.z = m_sensor.altitude() - m_groundAltitude;
		shm().temperature.altimeter = m_sensor.temp();
	}
}
