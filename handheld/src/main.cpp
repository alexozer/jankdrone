#include <Arduino.h>
#include <RFM69.h>
#include <pb_encode.h>
#include "shm.pb.h"
#include "shm.h"

// Unfortunately including SPI.h seems necessary for RFM69 lib on arduino nano
#include <SPI.h>
#include "radio/radio_stream.h"

constexpr int SERIAL_BAUD = 115200;

constexpr int LEFT_BUTTON_PIN = 7,
		  RIGHT_BUTTON_PIN = 6,
		  SOFT_KILL_PIN = RIGHT_BUTTON_PIN,
		  UN_SOFT_KILL_PIN = LEFT_BUTTON_PIN,

		  LEFT_X_PIN = A4, LEFT_Y_PIN = A3,
		  RIGHT_X_PIN = A0, RIGHT_Y_PIN = A1,
		  ANALOG_PINS[] = {LEFT_X_PIN, LEFT_Y_PIN, RIGHT_X_PIN, RIGHT_Y_PIN},

		  LED_PIN = 4;

constexpr bool INVERT_LEFT_X = false,
		  INVERT_LEFT_Y = true,
		  INVERT_RIGHT_X = false,
		  INVERT_RIGHT_Y = true;

constexpr int INPUT_SEND_PERIOD = 100;

constexpr int MAX_INPUT = 1023;
constexpr int MAX_TILT = 5;

constexpr int RADIO_NETWORK_ID = 100,
		  RADIO_NODE_ID = 1,
		  RADIO_RECEIVER_ID = 2,

		  // Set based on your RF69 module
		  RADIO_FREQUENCY = RF69_915MHZ; 
constexpr bool HAVE_RFM69HCW = false;

constexpr int MOSI_PIN = 11,
		  MISO_PIN = 12,
		  SCK_PIN = 13,

		  RADIO_CS_PIN = 10,
		  RADIO_IRQ_PIN = 2,
		  RADIO_IRQN = 0,
		  RADIO_RST_PIN = 9,
		  RADIO_POWER = 31; // [0, 31]

RadioStream radioStream(
		RADIO_CS_PIN,
		RADIO_IRQ_PIN,
		RADIO_IRQN,
		RADIO_RST_PIN,
		RADIO_FREQUENCY,
		RADIO_NODE_ID,
		RADIO_RECEIVER_ID,
		RADIO_NETWORK_ID,
		RADIO_POWER,
		HAVE_RFM69HCW
);

uint8_t sendBuf[255]; // Max size that length byte can describe
size_t lastInputSend = millis();
bool lastSoftKill = false;

float inputLerp(int pin, bool invert) {
	float l = (float)(analogRead(pin) - MAX_INPUT) / (MAX_INPUT / 2);
	return invert ? -l : l;
}

void writeRadioVar(const ShmMsg& v) {
	size_t encodedSize;
	pb_get_encoded_size(&encodedSize, ShmMsg_fields, &v);
	if (encodedSize > sizeof(sendBuf)) {
		Serial.println("[fatal]\tEncoded variable too large to send");
		exit(1);
	}

	radioStream.write((uint8_t)encodedSize);
	auto ostream = pb_ostream_from_buffer(sendBuf, sizeof(sendBuf));
	pb_encode(&ostream, ShmMsg_fields, &v);
	radioStream.write(sendBuf, encodedSize);
}

ShmMsg desireVar(int shmTag, float v) {
	ShmMsg msg;
	msg.tag = shmTag;
	msg.which_value = ShmMsg_floatValue_tag;
	msg.value.floatValue = v;
	return msg;
}

void inputsToRadio() {
	size_t t = millis();
	if (t - lastInputSend < INPUT_SEND_PERIOD) return;
	lastInputSend = t;

	bool softKill = lastSoftKill;
	if (!digitalRead(SOFT_KILL_PIN)) {
		softKill = true;
	} else if (!digitalRead(UN_SOFT_KILL_PIN)) {
		softKill = false;
	}
	if (softKill != lastSoftKill) {
		ShmMsg softKillMsg;
		softKillMsg.tag = SHM_SWITCHES_SOFTKILL_TAG;
		softKillMsg.which_value = ShmMsg_boolValue_tag;
		softKillMsg.value.boolValue = softKill;
		writeRadioVar(softKillMsg);
		lastSoftKill = softKill;
	}

	float force = inputLerp(LEFT_Y_PIN, INVERT_LEFT_Y);
	float yaw = inputLerp(LEFT_X_PIN, INVERT_LEFT_X) * 180;
	float pitch = inputLerp(RIGHT_X_PIN, INVERT_RIGHT_X) * MAX_TILT;
	float roll = inputLerp(RIGHT_Y_PIN, INVERT_RIGHT_Y) * MAX_TILT;

	writeRadioVar(desireVar(SHM_DESIRES_FORCE_TAG, force));
	writeRadioVar(desireVar(SHM_DESIRES_YAW_TAG, yaw));
	writeRadioVar(desireVar(SHM_DESIRES_PITCH_TAG, pitch));
	writeRadioVar(desireVar(SHM_DESIRES_ROLL_TAG, roll));

	radioStream.flush();
}

void mapStream(Stream* s1, Stream* s2) {
	while (s1->available()) {
		s2->write(s1->read());
	}
	s2->flush();
}

void setup() {
	Serial.begin(SERIAL_BAUD);
	pinMode(SOFT_KILL_PIN, INPUT_PULLUP);
	pinMode(UN_SOFT_KILL_PIN, INPUT_PULLUP);
	pinMode(LED_PIN, OUTPUT);
}

void loop() {
	inputsToRadio();
	mapStream(&radioStream, &Serial);
	mapStream(&Serial, &radioStream);
}
