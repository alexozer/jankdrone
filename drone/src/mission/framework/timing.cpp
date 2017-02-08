#include <Arduino.h>
#include "timing.h"

using namespace mission;

Timer::Timer(unsigned long period): m_period{period} {}

bool Timer::onFirstRun() {
	m_startTime = millis();
	return false;
}

bool Timer::onRun() {
	return millis() - m_startTime >= m_period;
}

Timeout::Timeout(Task task, unsigned long timeout):
	m_task{task},
	m_timer{make<Timer>(timeout)},
	m_timedOut{false} {}

bool Timeout::onRun() {
	(*m_task)();
	(*m_timer)();
	if (m_task->finished()) {
		success(m_task->success());
		return true;
	} else if (m_timer->finished()) {
		fail();
		m_timedOut = true;
		return true;
	}

	return false;
}

bool Timeout::timedOut() const {
	return m_timedOut;
}
