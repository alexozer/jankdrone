#pragma once

#include <vector>
#include "task.h"

using namespace mission;

class Sequential : public BaseTask {
	public:
		Sequential(std::vector<Task> tasks, bool finite = true);
		bool onRun() override;

	private:
		std::vector<Task> m_tasks;
		bool m_finite;
		decltype(m_tasks.begin()) m_currentTask;
};

class Concurrent : public BaseTask {
	public:
		Concurrent(std::vector<Task> tasks, bool finite = true);
		bool onRun() override;

	private:
		std::vector<Task> m_tasks;
		bool m_finite;
};

class MasterConcurrent : public BaseTask {
	public:
		MasterConcurrent(Task primaryTask, Task secondaryTask);
		bool onRun() override;

	private:
		Task m_primaryTask, m_secondaryTask;
};
