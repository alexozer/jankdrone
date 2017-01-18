#pragma once

#include "task.h"

namespace mission {

class Timer : public BaseTask {
	public:
		Timer(size_t period);
		bool onFirstRun() override;
		bool onRun() override;

	private:
		size_t m_period;
		size_t m_startTime;
};

class Timeout : public BaseTask {
	public:
		Timeout(Task task, size_t timeout);
		bool onRun() override;
		bool timedOut() const;

	private:
		Task m_task, m_timer;
		bool m_timedOut;
};

} // namespace mission
