#include <Arduino.h>
#include <cmath>
#include <SPI.h>
#include "log.h"
#include "shm.h"
#include "config.h"
#include "remote.h"

Remote::Remote():
	m_radioStream{
		RADIO_CS_PIN,
		RADIO_IRQ_PIN,
		HAVE_RFM69HCW
	},
	m_gotMsg{false},
	m_lastMsgTime{0}
{
	m_radioStream.begin(
		RADIO_FREQUENCY,
		RADIO_NODE_ID,
		RADIO_RECEIVER_ID,
		RADIO_NETWORK_ID,
		RADIO_RST_PIN,
		RADIO_POWER
	);
}

void Remote::operator()() {
	m_gotMsg = false;

	int rssi = m_radioStream.rfm69().RSSI;
	if (rssi != 0) shm().remote.rssi = rssi;
	readStream(&m_radioStream);
	readStream(&Serial);

	unsigned long t = millis();
	if (m_gotMsg) m_lastMsgTime = t;
	shm().remote.connected = t - m_lastMsgTime < REMOTE_TIMEOUT;
}

void Remote::readStream(Stream* stream) {
	while (stream->available()) {
		uint8_t size = stream->read();
		size_t bytesRead = 0;
		while (bytesRead < size && stream->available()) {
			m_messageBuffer[bytesRead++] = stream->read();
		}
		if (bytesRead < size) {
			// Assume the whole message is available at once
			Log::error("Remote message shorter than expected length, discarding");
			continue;
		}

		ShmMsg msg = ShmMsg_init_zero;
		auto pbUpdateStream = pb_istream_from_buffer(m_messageBuffer, bytesRead);
		if (!pb_decode_noinit(&pbUpdateStream, ShmMsg_fields, &msg)) {
			Log::error("Failed to decode remote message: %s",
					PB_GET_ERROR(&pbUpdateStream));
		}

		auto shmVar = shm().varIfExists(msg.tag);
		if (!shmVar) {
			Log::error("Remote var tag not found: %d", msg.tag);
			continue;
		}

		Shm::Var::Type msgVarType;
		switch (msg.which_value) {
			case ShmMsg_intValue_tag:
				msgVarType = Shm::Var::Type::INT;
				break;
			case ShmMsg_floatValue_tag:
				msgVarType = Shm::Var::Type::FLOAT;
				break;
			case ShmMsg_boolValue_tag:
				msgVarType = Shm::Var::Type::BOOL;
				break;
			default:
				sendVar(stream, shmVar);
				m_gotMsg = true;
				continue;
		}

		auto shmVarType = shmVar->type();
		if (msgVarType != shmVarType) {
			Log::error("Remote var type mismatch: expected %s, got %s",
					Shm::Var::typeString(shmVarType).c_str(),
					Shm::Var::typeString(msgVarType).c_str());
			continue;
		}

		switch (shmVarType) {
			case Shm::Var::Type::INT:
				shmVar->set((int)msg.value.intValue);
				break;
			case Shm::Var::Type::FLOAT:
				shmVar->set(msg.value.floatValue);
				break;
			case Shm::Var::Type::BOOL:
				shmVar->set(msg.value.boolValue);
				break;
			default:
				Log::error("Unsupported remote var type");
				continue;
		}

		m_gotMsg = true;
	}

	stream->flush();
}

void Remote::sendVar(Stream* stream, Shm::Var* var) {
	ShmMsg msg;
	msg.tag = var->tag();
	switch (var->type()) {
		case Shm::Var::Type::INT:
			msg.which_value = ShmMsg_intValue_tag;
			msg.value.intValue = var->get<int>();
			break;
		case Shm::Var::Type::FLOAT:
			msg.which_value = ShmMsg_floatValue_tag;
			msg.value.floatValue = var->get<float>();
			break;
		case Shm::Var::Type::BOOL:
			msg.which_value = ShmMsg_boolValue_tag;
			msg.value.boolValue = var->get<bool>();
			break;
		default:
			Log::error("Unsupported remote var type");
			return;
	}

	size_t encodedSize;
	pb_get_encoded_size(&encodedSize, ShmMsg_fields, &msg);
	constexpr int bufSize = sizeof(m_messageBuffer) / sizeof(m_messageBuffer[0]);
	if (encodedSize > (bufSize - 1)) {
		Log::error("Encoded remote message too large to send");
		return;
	}

	m_messageBuffer[0] = (uint8_t)encodedSize;
	auto ostream = pb_ostream_from_buffer(&m_messageBuffer[1], bufSize - 1);
	pb_encode(&ostream, ShmMsg_fields, &msg);

	stream->write(m_messageBuffer, encodedSize+1);
}
