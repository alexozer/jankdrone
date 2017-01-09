#pragma once

#include <Arduino.h>
#include <limits>
#include <Adafruit_BLE_UART.h>
#include <pb_encode.h>
#include <pb_decode.h>
#include "shm.pb.h"

class Remote {
	public:
		Remote();
		void operator()();

	private:
		bool m_firstRun;
		Adafruit_BLE_UART m_bluetooth;
		aci_evt_opcode_t m_lastState;
		uint8_t m_messageBuffer[std::numeric_limits<uint8_t>::max()];

		void readBluetooth();
		void readStream(Stream* stream);
		
		void sendVar(Stream* stream, Shm::Var* var);
		static bool sendFromStream(pb_ostream_t* stream, const uint8_t* buf, size_t count);
};
