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

BaseTask::BaseTask(): BaseTask{nullptr} {}

BaseTask::BaseTask(Task task):
	m_task{task},
	m_hasRun{false},
	m_finished{false},
	m_hasEverFinished{false},
	m_success{true} {}

BaseTask::~BaseTask() {}

bool BaseTask::operator()() {
	if (m_task) {
		return (*m_task)();
	}

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

bool BaseTask::finished() const {
	return m_finished;
}

bool BaseTask::hasEverFinished() const {
	return m_hasEverFinished;
}

bool BaseTask::success() const {
	return m_success;
}

bool BaseTask::onFirstRun() { return false; }
bool BaseTask::onRun() { return true; }
void BaseTask::onFinish() {}

void BaseTask::succeed() {
	m_success = true;
}

void BaseTask::fail() {
	m_success = false;
}

void BaseTask::success(bool state) {
	m_success = state;
}
