#pragma once

#include <functional>
#include <vector>

class Thread {
	public:
		Thread(std::function<void()> func, unsigned long interval);
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
