#pragma once

#include "task.h"

namespace mission {

class Timer : public Task {
	public:
		Timer(size_t period);
		bool onFirstRun() override;
		bool onRun() override;

	private:
		size_t m_period;
		size_t m_startTime;
};

class Timeout : public Task {
	public:
		Timeout(task task, size_t timeout);
		bool onRun() override;
		bool timedOut() const;

	private:
		task m_task, m_timer;
		bool m_timedOut;
};

} // namespace mission
