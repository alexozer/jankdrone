#include <Arduino.h>
#include "thread.h"

Thread::Thread(std::function<void()> func, size_t intervalMicros, int* tickTime):
	m_func{func}, m_interval{intervalMicros}, m_tickTime{tickTime}, m_hasRun{false} {}

void Thread::operator()() {
	auto t = micros();
	if (!m_hasRun || t - m_lastTime >= m_interval) {
		m_func();
		if (m_tickTime) *m_tickTime = micros() - t;
		m_hasRun = true;
		m_lastTime = t;
	}
}

FuncSet::FuncSet(std::initializer_list<std::function<void()>> funcs):
	m_funcs{funcs} {}

void FuncSet::operator()() {
	for (auto& f : m_funcs) {
		f();
	}
}
