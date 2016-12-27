#pragma once

#include <functional>
#include <vector>

class Thread {
	public:
		Thread(std::function<void()> func, unsigned long interval, bool useMillis = true);
		void operator()();

	private:
		std::function<void()> m_func;
		unsigned long m_interval;
		unsigned long m_lastTime;
		bool m_hasRun;
		bool m_useMillis;
};

class ThreadController {
	public:
		ThreadController(std::initializer_list<Thread> threads);
		void operator()();
	
	private:
		std::vector<Thread> m_threads;
};
