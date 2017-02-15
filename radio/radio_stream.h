#pragma once

#include <Arduino.h>
#include <RFM69.h>

class RadioStream : public Stream {
	public:
		RadioStream(uint8_t csPin, uint8_t irqPin, bool haveRFM69HCW);

		// Arguments not needed to construct RFM69 object passed here
		void begin(int freq, int nodeId, int receiverId, int networkId, uint8_t rstPin, int power);

		int available() override;
		int read() override;
		int peek() override;

		size_t write(uint8_t b) override;
		size_t write(const uint8_t* buffer, size_t size) override;
		void flush() override;

		RFM69& rfm69();

	private:
		RFM69 m_radio;
		int m_receiverId;
		size_t m_recvBegin, m_recvEnd;

		uint8_t m_sendBuf[RF69_MAX_DATA_LEN];
		size_t m_sendEnd;
};
