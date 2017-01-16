#pragma once

#include "task.h"

using namespace mission;

class Sequential : public Task {
	public:
		Sequential(std::vector<task> tasks, bool finite = true);
		bool onRun() override;

	private:
		std::vector<task> m_tasks;
		bool m_finite;
		decltype(m_tasks.begin()) m_currentTask;
};

class Concurrent : public Task {
	public:
		Concurrent(std::vector<task> tasks, bool finite = true);
		bool onRun() override;

	private:
		std::vector<task> m_tasks;
		bool m_finite, m_lastSuccess;
};

class MasterConcurrent : public Task {
	public:
		MasterConcurrent(task primaryTask, task secondaryTask);
		bool onRun() override;

	private:
		task m_primaryTask, m_secondaryTask;
};
