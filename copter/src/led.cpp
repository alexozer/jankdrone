#include <Arduino.h>
#include "shm.h"
#include "config.h"
#include "led.h"
#include "logger.h"

constexpr int MAX_VALUE = 255,
		  ANIM_INTERVAL = 750;

Led::PieceFunc::PieceFunc(std::initializer_list<Coord> pts):
	m_pts{pts} {}

int Led::PieceFunc::operator()(int time) {
	int modTime = time % (m_pts[m_pts.size()-1].first - m_pts[0].first);
	for (auto it = m_pts.cbegin() + 1; it != m_pts.cend(); ++it) {
		if (time > it->first) continue;

		auto prev = it - 1;
		int interval = it->first - prev->first;
		int intervalTime = modTime - prev->first;
		int delta = it->second - prev->second;
		return prev->second + intervalTime * delta / interval;
	}
	return 0;
}

Led::Led(): 
	m_front{Shm::var("leds.front")},
	m_back{Shm::var("leds.back")},
	m_left{Shm::var("leds.left")},
	m_right{Shm::var("leds.right")},
	m_unkilled{
		{ANIM_INTERVAL * 0, 0},
		{ANIM_INTERVAL * 0.25, MAX_VALUE},
		{ANIM_INTERVAL * 0.5, MAX_VALUE},
		{ANIM_INTERVAL * 0.75, 0},
		{ANIM_INTERVAL * 1, 0}}
{
	for (auto pin : {FRONT_LED_PIN, LEFT_LED_PIN, RIGHT_LED_PIN, BACK_LED_PIN}) {
		pinMode(pin, OUTPUT);
	}
}

void Led::operator()() {
	analogWrite(FRONT_LED_PIN, m_front->getInt());
	analogWrite(LEFT_LED_PIN, m_left->getInt());
	analogWrite(RIGHT_LED_PIN, m_right->getInt());
	analogWrite(BACK_LED_PIN, m_back->getInt());
}

void Led::showStatus() {
	int now = millis();
	static auto lowPower = Shm::var("power.low"),
				softKill = Shm::var("switches.softKill");
	if (lowPower->getBool()) {
		// Blink
		int blinkInterval = ANIM_INTERVAL / 4;
		int out = (now % blinkInterval) < blinkInterval / 2 ? MAX_VALUE : 0;
		for (auto v : {m_front, m_back, m_left, m_right}) {
			v->set(out);
		}

	} else if (!softKill->getBool()) {
		// Fade
		int out1 = m_unkilled(now);
		int out2 = m_unkilled(now + ANIM_INTERVAL / 2);
		m_front->set(out1);
		m_back->set(out1);
		m_left->set(out2);
		m_right->set(out2);
	} else {
		// Off
		for (auto v : {m_front, m_back, m_left, m_right}) {
			v->set(0);
		}
	}
}
