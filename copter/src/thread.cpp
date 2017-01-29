#include <Arduino.h>
#include "thread.h"

Thread::Thread(std::function<void()> func, unsigned long intervalMicros):
	m_func{func}, m_interval{intervalMicros}, m_hasRun{false} {}

void Thread::operator()() {
	auto t = micros();
	if (!m_hasRun || t - m_lastTime >= m_interval) {
		m_func();
		m_hasRun = true;
		m_lastTime = t;
	}
}

ThreadController::ThreadController(std::initializer_list<Thread> threads):
	m_threads{threads} {}

void ThreadController::operator()() {
	for (auto& t : m_threads) {
		t();
	}
}
