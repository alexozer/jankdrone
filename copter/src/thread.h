#pragma once

#include <functional>
#include <vector>

class Thread {
	public:
		static constexpr int SECOND = 1e6;

		Thread(std::function<void()> func, unsigned long intervalMicros);
		void operator()();

	private:
		std::function<void()> m_func;
		unsigned long m_interval;
		unsigned long m_lastTime;
		bool m_hasRun;
};

class ThreadController {
	public:
		ThreadController(std::initializer_list<Thread> threads);
		void operator()();
	
	private:
		std::vector<Thread> m_threads;
};
