#include <Arduino.h>
#include <string.h>
#include "radio_stream.h"

RadioStream::RadioStream(uint8_t csPin, uint8_t irqPin, bool haveRFM69HCW):
	m_radio{csPin, irqPin, haveRFM69HCW, (uint8_t)digitalPinToInterrupt(irqPin)},
	m_recvBegin{0},
	m_recvEnd{0},
	m_sendEnd{0} {}

void RadioStream::begin(int freq, int nodeId, int receiverId, int networkId, uint8_t rstPin, int power) {
	m_receiverId = receiverId;

	// Hard reset
	pinMode(rstPin, OUTPUT);
	digitalWrite(rstPin, HIGH);
	delay(100);
	digitalWrite(rstPin, LOW);
	delay(100);

	m_radio.initialize(freq, nodeId, networkId);
	m_radio.setPowerLevel(power);
}

int RadioStream::available() {
	if (m_recvEnd > 0) return m_recvEnd - m_recvBegin;

	if (m_radio.receiveDone()) {
		m_recvEnd = m_radio.DATALEN;
	}
	return m_recvEnd;
}

int RadioStream::read() {
	if (!available()) return -1;

	int b = m_radio.DATA[m_recvBegin++];
	if (m_recvBegin == m_recvEnd) {
		m_recvBegin = 0;
		m_recvEnd = 0;

		if (m_radio.ACKRequested()) {
			m_radio.sendACK();
		}
		m_radio.receiveDone(); // Put radio into rx mode
	}
	return b;
}

int RadioStream::peek() {
	if (!available()) return -1;

	return m_radio.DATA[m_recvBegin];
}

size_t RadioStream::write(uint8_t b) {
	return write(&b, 1);
}

size_t RadioStream::write(const uint8_t* buf, size_t len) {
	if (len == 0) return 0;

	size_t writeLen = min(sizeof(m_sendBuf) - m_sendEnd, len);
	memcpy(&m_sendBuf[m_sendEnd], buf, writeLen);
	m_sendEnd += writeLen;

	if (m_sendEnd == sizeof(m_sendBuf)) {
		flush();
		write(&buf[writeLen], len - writeLen);
	}

	return len;
}

void RadioStream::flush() {
	if (m_sendEnd == 0) return;

	// Don't send with retry to limit avoid blocking
	m_radio.send(m_receiverId, m_sendBuf, m_sendEnd);
	m_sendEnd = 0;
	m_radio.receiveDone();
}

RFM69& RadioStream::rfm69() {
	return m_radio;
}
