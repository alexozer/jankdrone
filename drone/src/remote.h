#pragma once

#include <Arduino.h>
#include <limits>
#include <RFM69.h>
#include <pb_encode.h>
#include <pb_decode.h>
#include "shm.pb.h"
#include "radio/radio_stream.h"

class Remote {
	public:
		Remote();
		void operator()();

	private:
		uint8_t m_messageBuffer[std::numeric_limits<uint8_t>::max()];
		RadioStream m_radioStream;

		void readStream(Stream* stream);
		void sendVar(Stream* stream, Shm::Var* var);
		static bool sendFromStream(pb_ostream_t* stream, const uint8_t* buf, size_t count);
};
