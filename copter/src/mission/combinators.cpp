#include "combinators.h"

using namespace mission;

Sequential::Sequential(std::vector<task> tasks, bool finite):
	m_tasks{tasks}, m_finite{finite}, m_currentTask{m_tasks.begin()} {}

bool Sequential::onRun() {
	if (m_finite) {
		if (hasEverFinished()) return true;
		m_currentTask = m_tasks.begin();
	}

	for (; m_currentTask != m_tasks.end(); ++m_currentTask) {
		if (!(**m_currentTask)()) return false;

		if (!(*m_currentTask)->success()) {
			fail();
			return true;
		}
	}

	return true;
}

Concurrent::Concurrent(std::vector<task> tasks, bool finite):
	m_tasks{tasks}, m_finite{finite}, m_lastSuccess{true} {}

bool Concurrent::onRun() {
	if (m_finite && hasEverFinished()) return true;

	bool allFinished = true;
	bool allSuccess = true;
	for (auto& t : m_tasks) {
		if (m_finite && t->hasEverFinished()) continue;
		allFinished = allFinished && (*t)();
		allSuccess = allSuccess && t->success();
	}
	if (allFinished) {
		success(allSuccess);
		return true;
	}
	return false;
}

MasterConcurrent::MasterConcurrent(task primaryTask, task secondaryTask):
	m_primaryTask{primaryTask}, m_secondaryTask{secondaryTask} {}

bool MasterConcurrent::onRun() {
	(*m_primaryTask)();
	(*m_secondaryTask)();
	if (m_primaryTask->finished()) {
		success(m_primaryTask->success());
		return true;
	}
	return false;
}
