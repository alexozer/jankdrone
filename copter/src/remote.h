#pragma once

#include <Arduino.h>
#include <Adafruit_BLE_UART.h>
#include <pb_decode.h>
#include "shm_update.pb.h"

class Remote {
	public:
		Remote();
		void operator()();

	private:
		Adafruit_BLE_UART m_bluetooth;
		aci_evt_opcode_t m_lastState;
		uint8_t m_messageBuffer[512];

		void readBluetooth();
		void readStream(Stream* stream);
		static bool decodeVar(pb_istream_t* stream, const pb_field_t* field, void** arg);
};
