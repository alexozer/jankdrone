#include "task.h"

namespace mission {

template <>
int V<int>::readShm() {
	return m_shm->getInt();
}

template <>
float V<float>::readShm() {
	return m_shm->getFloat();
}

template <>
bool V<bool>::readShm() {
	return m_shm->getBool();
}

} // namespace mission

using namespace mission;

Task::Task(): 
	m_hasRun{false},
	m_finished{false},
	m_hasEverFinished{false},
	m_success{true} {}

Task::~Task() {}

bool Task::operator()() {
	m_finished = false;
	m_finished = m_hasRun ? onRun() : onFirstRun();

	if (m_finished) {
		m_hasRun = true;
		onFinish();
		m_hasEverFinished = true;

	} else if (!m_hasRun) {
		m_hasRun = true;
		return (*this)();
	}

	return m_finished;
}

bool Task::finished() const {
	return m_finished;
}

bool Task::hasEverFinished() const {
	return m_hasEverFinished;
}

bool Task::success() const {
	return m_success;
}

bool Task::onFirstRun() { return false; }
bool Task::onRun() { return true; }
void Task::onFinish() {}

void Task::succeed() {
	m_success = true;
}

void Task::fail() {
	m_success = false;
}

void Task::success(bool state) {
	m_success = state;
}
